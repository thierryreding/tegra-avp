#include <common.h>
#include <errno.h>
#include <irq.h>
#include <string.h>
#include <usb.h>

#include <avp/bct.h>
#include <avp/bit.h>
#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/sdram.h>
#include <avp/timer.h>
#include <avp/uart.h>
#include <avp/usb.h>

#include <nv3p.h>

#define PMC_STRAPPING_OPT_A 0x64

#define USBCMD 0x130
#define  USBCMD_ITC(x) ((x) << 16)
#define  USBCMD_ITC_MASK (0xff << 16)
#define  USBCMD_RST (1 << 1)
#define  USBCMD_RUN (1 << 0)

#define USBSTS 0x134
#define  USBSTS_SLI (1 << 8)
#define  USBSTS_SRI (1 << 7)
#define  USBSTS_URI (1 << 6)
#define  USBSTS_AAI (1 << 5)
#define  USBSTS_SEI (1 << 4)
#define  USBSTS_FRI (1 << 3)
#define  USBSTS_PCI (1 << 2)
#define  USBSTS_UEI (1 << 1)
#define  USBSTS_UI  (1 << 0)

#define USBINTR 0x138
#define  USBINTR_SLE (1 << 8)
#define  USBINTR_SRE (1 << 7)
#define  USBINTR_URE (1 << 6)
#define  USBINTR_AAE (1 << 5)
#define  USBINTR_SEE (1 << 4)
#define  USBINTR_FRE (1 << 3)
#define  USBINTR_PCE (1 << 2)
#define  USBINTR_UEE (1 << 1)
#define  USBINTR_UE  (1 << 0)

#define PERIODICLISTBASE 0x144
#define  PERIODICLISTBASE_USBADR(x) (((x) & 0x7f) << 25)

#define ASYNCLISTADDR 0x148

#define HOSTPC1_DEVLC 0x1b4
#define  HOSTPC1_DEVLC_ASUS (1 << 17)

#define OTGSC 0x1f4

#define USBMODE 0x1f8
#define  USBMODE_CM_IDLE (0 << 0)
#define  USBMODE_CM_DEVICE (2 << 0)
#define  USBMODE_CM_HOST (3 << 0)
#define  USBMODE_CM_MASK (3 << 0)

#define ENDPTSETUPSTAT 0x208
#define ENDPTPRIME 0x20c
#define ENDPTFLUSH 0x210
#define ENDPTSTATUS 0x214
#define ENDPTCOMPLETE 0x218

#define ENDPTCTRL(x) (0x21c + (x) * 4)
#define  ENDPTCTRL_TXE (1 << 23)
#define  ENDPTCTRL_TXT_CTRL (0x0 << 18)
#define  ENDPTCTRL_TXT_ISO  (0x1 << 18)
#define  ENDPTCTRL_TXT_BULK (0x2 << 18)
#define  ENDPTCTRL_TXT_INTR (0x3 << 18)
#define  ENDPTCTRL_TXT_MASK (0x3 << 18)
#define  ENDPTCTRL_TXS (1 << 16)
#define  ENDPTCTRL_RXE (1 << 7)
#define  ENDPTCTRL_RXT_CTRL (0x0 << 2)
#define  ENDPTCTRL_RXT_ISO  (0x1 << 2)
#define  ENDPTCTRL_RXT_BULK (0x2 << 2)
#define  ENDPTCTRL_RXT_INTR (0x3 << 2)
#define  ENDPTCTRL_RXT_MASK (0x3 << 2)
#define  ENDPTCTRL_RXS (1 << 0)

#define USB_SUSP_CTRL 0x400
#define  USB_SUSP_CTRL_UTMIP_RESET (1 << 11)
#define  USB_SUSP_CTRL_USB_PHY_CLK_VALID (1 << 7)

#define USB_PHY_VBUS_SENSORS 0x404

struct usb_dqh {
	uint32_t caps;
	uint32_t current;
	uint32_t next;
	uint32_t info;
	uint32_t buffers[5];
	uint32_t reserved0;
	uint32_t setup[2];
	uint32_t reserved1;
	uint32_t reserved2;
	uint32_t reserved3;
	uint32_t reserved4;
};

#define DQH_CAPS_MAX_PKT_LEN(x) ((x) << 16)
#define DQH_CAPS_ZLT (1 << 29)
#define DQH_CAPS_IOC (1 << 15)
#define DQH_NEXT_TERMINATE (1 << 0)

struct usb_dtd {
	uint32_t next;
	uint32_t info;
	uint32_t buffers[5];
	uint32_t reserved;
};

#define DTD_NEXT_TERMINATE (1 << 0)
#define DTD_INFO_TOTAL_BYTES(x) (((x) & 0xffff) << 16)
#define DTD_INFO_IOC (1 << 15)
#define DTD_INFO_ACTIVE (1 << 7)

/* one dTD (device transfer descriptor) per endpoint */
static uint8_t bounce[4096] __attribute__((aligned(4096)));
static struct usb_dtd dtd[4] __attribute__((aligned(32)));

enum {
	CTRL,
	BULK,
	ISO,
};

enum {
	OUT,
	IN,
};

struct usb_ep {
	struct usb *usb;

	unsigned int index;
	unsigned int direction;
	unsigned int type;
	unsigned int max_pkt_len;

	struct usb_dqh *dqh;
	struct usb_dtd *dtd;
};

struct usb {
	unsigned long base;

	struct usb_ep ep_ctrl_out;
	struct usb_ep ep_ctrl_in;
	struct usb_ep ep_bulk_out;
	struct usb_ep ep_bulk_in;
};

static inline void usb_writel(struct usb *usb, uint32_t value,
			      unsigned long offset)
{
	writel(value, usb->base + offset);
}

static inline uint32_t usb_readl(struct usb *usb, unsigned long offset)
{
	return readl(usb->base + offset);
}

struct usb usbd = {
	.base = TEGRA_USBD_BASE,

	.ep_ctrl_out = {
		.usb = &usbd,
		.index = 0,
		.direction = OUT,
		.type = CTRL,
		.dtd = &dtd[0],
	},
	.ep_ctrl_in = {
		.usb = &usbd,
		.index = 0,
		.direction = IN,
		.type = CTRL,
		.dtd = &dtd[1],
	},
	.ep_bulk_out = {
		.usb = &usbd,
		.index = 1,
		.direction = OUT,
		.type = BULK,
		.dtd = &dtd[2],
	},
	.ep_bulk_in = {
		.usb = &usbd,
		.index = 1,
		.direction = IN,
		.type = BULK,
		.dtd = &dtd[3],
	},
};

static uint8_t recv[4096] __attribute__((aligned(32)));
static uint8_t send[4096] __attribute__((aligned(32)));
static void (*bootloader)(void) = NULL;
static struct bct _bct __attribute__((aligned(32)));
static struct bct *bct = &_bct;
static uint32_t bct_csum = 0;

#define USB_LANGUAGE_ID 0x00
#define USB_MANUFACTURER_ID 0x01
#define USB_PRODUCT_ID 0x02
#define USB_SERIAL_ID 0x03

static const uint8_t language_ids[] __attribute__((aligned(32))) = {
	4,
	USB_DESCRIPTOR_TYPE_STRING,
	0x09, 0x04
};

static const uint8_t manufacturer[] __attribute__((aligned(32))) = {
	14,
	USB_DESCRIPTOR_TYPE_STRING,
	'N', 0,
	'V', 0,
	'I', 0,
	'D', 0,
	'I', 0,
	'A', 0,
};

static const uint8_t product[] __attribute__((aligned(32))) = {
	8,
	USB_DESCRIPTOR_TYPE_STRING,
	'A', 0,
	'P', 0,
	'X', 0,
};

static const uint8_t serial[] __attribute__((aligned(32))) = {
	18,
	USB_DESCRIPTOR_TYPE_STRING,
	'0', 0,
	'0', 0,
	'0', 0,
	'0', 0,
	'0', 0,
	'0', 0,
	'0', 0,
	'0', 0,
};

static struct usb_device_descriptor device_descriptor __attribute__((aligned(32))) = {
	.bLength = sizeof(device_descriptor),
	.bDescriptorType = USB_DESCRIPTOR_TYPE_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0x00,
	.bDeviceSubClass = 0x00,
	.bDeviceProtocol = 0x00,
	.bMaxPacketSize0 = 0x40,
	.idVendor = 0x0955,
	.idProduct = 0x7140,
	.bcdDevice = 0x0101,
	.iManufacturer = USB_MANUFACTURER_ID,
	.iProduct = USB_PRODUCT_ID,
	.iSerialNumber = USB_SERIAL_ID,
	.bNumConfigurations = 0x01,
};

struct usb_configuration {
	struct usb_configuration_descriptor configuration;
	struct usb_interface_descriptor interface;
	struct usb_endpoint_descriptor ep_bulk_in;
	struct usb_endpoint_descriptor ep_bulk_out;
} __attribute__((packed));

static struct usb_configuration configuration __attribute__((aligned(32))) = {
	.configuration = {
		.bLength = sizeof(struct usb_configuration_descriptor),
		.bDescriptorType = USB_DESCRIPTOR_TYPE_CONFIGURATION,
		.wTotalLength = sizeof(configuration),
		.bNumInterfaces = 0x01,
		.bConfigurationValue = 0x01,
		.iConfiguration = 0x00,
		.bmAttributes = 0xc0,
		.bMaxPower = 0x10,
	},
	.interface = {
		.bLength = sizeof(struct usb_interface_descriptor),
		.bDescriptorType = USB_DESCRIPTOR_TYPE_INTERFACE,
		.bInterfaceNumber = 0x00,
		.bAlternateSetting = 0x00,
		.bNumEndpoints = 0x02,
		.bInterfaceClass = 0xff,
		.bInterfaceSubClass = 0xff,
		.bInterfaceProtocol = 0xff,
		.iInterface = 0x00,
	},
	.ep_bulk_in = {
		.bLength = sizeof(struct usb_endpoint_descriptor),
		.bDescriptorType = USB_DESCRIPTOR_TYPE_ENDPOINT,
		.bEndpointAddress = 0x81,
		.bmAttributes = 0x02,
		.wMaxPacketSize = 0x0200,
		.bInterval = 0x00,
	},
	.ep_bulk_out = {
		.bLength = sizeof(struct usb_endpoint_descriptor),
		.bDescriptorType = USB_DESCRIPTOR_TYPE_ENDPOINT,
		.bEndpointAddress = 0x01,
		.bmAttributes = 0x02,
		.wMaxPacketSize = 0x0200,
		.bInterval = 0x00,
	},
};

static uint32_t usb_ep_mask(struct usb_ep *ep)
{
	uint32_t value = (ep->direction == IN) ? 0x00010000 : 0x00000001;

	return value << ep->index;
}

static void usb_ep_setup(struct usb_ep *ep)
{
	uint32_t caps, value;
	unsigned int dqh;

	dqh = 0x1000 + (ep->index * 2 + ep->direction) * 0x40;
	ep->dqh = (struct usb_dqh *)(ep->usb->base + dqh);

	switch (ep->type) {
	case CTRL:
		if (ep->direction == OUT)
			caps = DQH_CAPS_IOC;

		ep->max_pkt_len = 64;
		break;

	case BULK:
		caps = DQH_CAPS_ZLT;

		/* TODO: parameterize based on speed */
		ep->max_pkt_len = 512;
		break;

	default:
		uart_printf(debug, "unsupported endpoint type: %u\n", ep->type);
		return;
	}

	memset(ep->dqh, 0, sizeof(*ep->dqh));
	ep->dqh->caps = DQH_CAPS_MAX_PKT_LEN(ep->max_pkt_len) | caps;

	if (ep->type == CTRL) {
		ep->dqh->next = DQH_NEXT_TERMINATE;
	} else {
		ep->dqh->next = DQH_NEXT_TERMINATE;
	}

	value = usb_readl(ep->usb, ENDPTCTRL(ep->index));

	if (ep->direction == OUT) {
		value &= ~ENDPTCTRL_RXT_MASK;
		value &= ~ENDPTCTRL_RXS;

		if (ep->type == CTRL)
			value |= ENDPTCTRL_RXT_CTRL;
		else
			value |= ENDPTCTRL_RXT_BULK;

		value |= ENDPTCTRL_RXE;
	} else {
		value &= ~ENDPTCTRL_TXT_MASK;
		value &= ~ENDPTCTRL_TXS;

		if (ep->type == CTRL)
			value |= ENDPTCTRL_TXT_CTRL;
		else
			value |= ENDPTCTRL_TXT_BULK;

		value |= ENDPTCTRL_TXE;
	}

	usb_writel(ep->usb, value, ENDPTCTRL(ep->index));
}

static void usb_ep_flush(struct usb_ep *ep)
{
	uint32_t mask = usb_ep_mask(ep);
	uint32_t value;

	usb_writel(ep->usb, mask, ENDPTFLUSH);

	while (true) {
		value = usb_readl(ep->usb, ENDPTFLUSH);
		if ((value & mask) == 0)
			break;
	}

	while (true) {
		value = usb_readl(ep->usb, ENDPTSTATUS);
		if ((value & mask) == 0)
			break;
	}

	while (true) {
		value = usb_readl(ep->usb, ENDPTPRIME);
		if ((value & mask) == 0)
			break;
	}

	memset(ep->dqh, 0, sizeof(*ep->dqh));
	memset(ep->dtd, 0, sizeof(*ep->dtd));

	usb_writel(ep->usb, mask, ENDPTCOMPLETE);
}

static void usb_ep_prime(struct usb_ep *ep)
{
	uint32_t mask = usb_ep_mask(ep), value;

	usb_writel(ep->usb, mask, ENDPTPRIME);

	while (true) {
		value = usb_readl(ep->usb, ENDPTPRIME);
		if ((value & mask) == 0)
			break;
	}
}

static void usb_ep_send(struct usb_ep *ep, const void *buffer, size_t size)
{
	usb_ep_flush(ep);

	ep->dqh->caps = DQH_CAPS_ZLT | DQH_CAPS_MAX_PKT_LEN(ep->max_pkt_len) |
			DQH_CAPS_IOC;
	ep->dqh->next = (uint32_t)ep->dtd & ~0x1f;

	ep->dtd->next = DTD_NEXT_TERMINATE;
	ep->dtd->info = DTD_INFO_TOTAL_BYTES(size) | DTD_INFO_IOC |
			DTD_INFO_ACTIVE;
	ep->dtd->buffers[0] = (uint32_t)buffer;

	usb_ep_prime(ep);
}

static void usb_ep_recv(struct usb_ep *ep, void *buffer, size_t size)
{
	usb_ep_flush(ep);

	memset(ep->dqh, 0, sizeof(*ep->dqh));
	ep->dqh->caps = DQH_CAPS_ZLT | DQH_CAPS_MAX_PKT_LEN(ep->max_pkt_len);
	ep->dqh->next = (uint32_t)ep->dtd & ~0x1f;

	memset(ep->dtd, 0, sizeof(*ep->dtd));
	ep->dtd->next = DTD_NEXT_TERMINATE;
	ep->dtd->info = DTD_INFO_TOTAL_BYTES(size) | DTD_INFO_IOC |
			DTD_INFO_ACTIVE;
	ep->dtd->buffers[0] = (uint32_t)buffer & ~0x1f;

	if (0) {
		uart_printf(debug, "  dqh:\n");
		uart_printf(debug, "    caps: %08x\n", ep->dqh->caps);
		uart_printf(debug, "    next: %08x\n", ep->dqh->next);
		uart_printf(debug, "  dtd:\n");
		uart_printf(debug, "    next: %08x\n", ep->dtd->next);
		uart_printf(debug, "    info: %08x\n", ep->dtd->info);
		uart_printf(debug, "    data: %08x\n", ep->dtd->buffers[0]);
	}

	usb_ep_prime(ep);
}

static void usb_ep_wait(struct usb_ep *ep)
{
	uint32_t value, mask = usb_ep_mask(ep);
	unsigned int retries = 1000;
	uint32_t prime, status;

	while (retries) {
		status = usb_readl(ep->usb, ENDPTSTATUS);
		prime = usb_readl(ep->usb, ENDPTPRIME);

		if (((prime & mask) == 0) && ((status & mask) == 0))
			break;

		udelay(1000);
		retries--;
	}

	if (retries == 0)
		uart_printf(debug, "time-out: prime: %08x status: %08x\n",
			    prime, status);

	if (ep->dqh->info & ((1 << 6) | (1 << 5) | (1 << 3))) {
		uart_printf(debug, "  error detected: %08x\n", ep->dqh->info);
		uart_printf(debug, "  dtd:\n");
		uart_printf(debug, "    next: %08x\n", ep->dtd->next);
		uart_printf(debug, "    info: %08x\n", ep->dtd->info);
		uart_printf(debug, "    buffers:\n");
		uart_printf(debug, "      0: %08x\n", ep->dtd->buffers[0]);
		uart_printf(debug, "      1: %08x\n", ep->dtd->buffers[1]);
		uart_printf(debug, "      2: %08x\n", ep->dtd->buffers[2]);
		uart_printf(debug, "      3: %08x\n", ep->dtd->buffers[3]);
		uart_printf(debug, "      4: %08x\n", ep->dtd->buffers[4]);
	}

	retries = 1000;

	while (retries) {
		value = usb_readl(ep->usb, ENDPTCOMPLETE);
		if ((value & mask) != 0)
			break;

		udelay(1000);
		retries--;
	}

	if (retries == 0)
		uart_printf(debug, "timed out: complete: %08x\n", value);

	if (ep->dqh->info & ((1 << 6) | (1 << 5) | (1 << 3)))
		uart_printf(debug, "  error detected\n");

	usb_writel(ep->usb, mask, ENDPTCOMPLETE);
}

static void usb_ep_ack(struct usb_ep *ep)
{
	usb_ep_send(ep, NULL, 0);
	usb_ep_wait(ep);
}

static void prepare_device_descriptor(struct usb_device_descriptor *desc)
{
	uint8_t chip, major, minor, family, sku;
	uint32_t value;

	value = readl(TEGRA_APB_MISC_BASE + 0x804);
	minor = (value >> 16) & 0xff;
	chip = (value >> 8) & 0xff;
	major = (value >> 4) & 0xf;
	family = value & 0xf;

	value = readl(TEGRA_FUSE_BASE + 0x100 + 0x10);
	sku = value & 0xf;

	desc->idProduct = (family << 12) | (sku << 8) | chip;
	desc->bcdDevice = major << 8 | minor;
}

static struct usb_setup *usb_ep_get_setup(struct usb_ep *ep)
{
	return (struct usb_setup *)ep->dqh->setup;
}

static void usb_process_setup_get_descriptor(struct usb *usb)
{
	struct usb_setup *setup = usb_ep_get_setup(&usb->ep_ctrl_out);
	uint8_t index = setup->wValue & 0xff;
	uint8_t type = setup->wValue >> 8;
	const void *data = NULL;
	size_t size = 0;

	switch (type) {
	case USB_DESCRIPTOR_TYPE_DEVICE:
		prepare_device_descriptor(&device_descriptor);
		usb_ep_send(&usbd.ep_ctrl_in, &device_descriptor,
			    sizeof(device_descriptor));
		usb_ep_send(&usbd.ep_ctrl_out, NULL, 0);
		break;

	case USB_DESCRIPTOR_TYPE_CONFIGURATION:
		if (setup->wLength == sizeof(configuration.configuration)) {
			size = sizeof(configuration.configuration);
			data = &configuration.configuration;
		} else {
			size = sizeof(configuration);
			data = &configuration;
		}

		usb_ep_send(&usbd.ep_ctrl_in, data, size);
		usb_ep_send(&usbd.ep_ctrl_out, NULL, 0);
		break;

	case USB_DESCRIPTOR_TYPE_STRING:
		switch (index) {
		case USB_LANGUAGE_ID:
			size = sizeof(language_ids);
			data = language_ids;
			break;

		case USB_MANUFACTURER_ID:
			size = sizeof(manufacturer);
			data = manufacturer;
			break;

		case USB_PRODUCT_ID:
			size = sizeof(product);
			data = product;
			break;

		case USB_SERIAL_ID:
			size = sizeof(serial);
			data = serial;
			break;
		}

		usb_ep_send(&usbd.ep_ctrl_in, data, size);
		usb_ep_send(&usbd.ep_ctrl_out, NULL, 0);
		break;
	}
}

static bool usb_process_setup(struct usb *usb)
{
	struct usb_setup *setup = usb_ep_get_setup(&usb->ep_ctrl_out);
	uint32_t value;

	switch (setup->bmRequestType) {
	case USB_SETUP_REQUEST_TYPE_DEVICE_TO_HOST:
		switch (setup->bRequest) {
		case USB_SETUP_REQUEST_GET_DESCRIPTOR:
			uart_printf(debug, "GET_DESCRIPTOR: %04x\n",
				    setup->wValue);
			usb_process_setup_get_descriptor(usb);
			break;
		}

	case USB_SETUP_REQUEST_TYPE_HOST_TO_DEVICE:
		switch (setup->bRequest) {
		case USB_SETUP_REQUEST_SET_ADDRESS:
			uart_printf(debug, "SET_ADDRESS: %04x\n",
				    setup->wValue);
			usb_ep_ack(&usb->ep_ctrl_in);

			value = PERIODICLISTBASE_USBADR(setup->wValue);
			usb_writel(usb, value, PERIODICLISTBASE);
			break;

		case USB_SETUP_REQUEST_SET_CONFIGURATION:
			uart_printf(debug, "SET_CONFIGURATION: %04x\n",
				    setup->wValue);
			usb_ep_ack(&usb->ep_ctrl_in);
			return true;
		}
	}

	return false;
}

static int usb_irq(unsigned int irq, void *data)
{
	struct usb *usb = data;
	uint32_t value;

	value = usb_readl(usb, USBSTS);
	usb_writel(usb, value, USBSTS);

	return IRQ_HANDLED;
}

static void nv3p_checksum_init(uint32_t *checksum)
{
	*checksum = 0;
}

static void nv3p_checksum_update(uint32_t *checksum, const void *buffer, size_t size)
{
	const uint8_t *ptr = buffer;
	size_t i;

	for (i = 0; i < size; i++)
		*checksum += ptr[i];
}

static uint32_t nv3p_checksum_done(uint32_t *checksum)
{
	*checksum = ~(*checksum) + 1;

	return *checksum;
}

#if 0
static void nv3p_send_nak(uint32_t sequence, uint32_t error)
{
	struct nv3p_packet_nak *nak = (struct nv3p_packet_nak *)send;
	uint32_t checksum;

	nv3p_checksum_init(&checksum);

	memset(nak, 0, sizeof(*nak));
	nak->header.version = NV3P_VERSION;
	nak->header.type = NV3P_PACKET_TYPE_NAK;
	nak->header.sequence = sequence;
	nak->error = error;

	nv3p_checksum_update(&checksum, nak, sizeof(*nak));
	nak->checksum = nv3p_checksum_done(&checksum);

#if 0
	uart_printf(debug, "  nak packet:\n");
	uart_printf(debug, "    header:\n");
	uart_printf(debug, "      version: %08x\n", nak->header.version);
	uart_printf(debug, "      type: %08x\n", nak->header.type);
	uart_printf(debug, "      sequence: %08x\n", nak->header.sequence);
	uart_printf(debug, "    error: %08x\n", nak->error);
	uart_printf(debug, "    checksum: %08x\n", nak->checksum);
#endif

	usb_ep_send(&ep_bulk_in, nak, sizeof(*nak));
	usb_ep_wait(&ep_bulk_in);
}
#endif

static void nv3p_send_ack(struct usb *usb, uint32_t sequence)
{
	struct nv3p_packet_ack *ack = (struct nv3p_packet_ack *)send;
	uint32_t checksum = 0;
	unsigned int i;

	memset(ack, 0, sizeof(*ack));
	ack->header.version = NV3P_VERSION;
	ack->header.type = NV3P_PACKET_TYPE_ACK;
	ack->header.sequence = sequence;

	for (i = 0; i < sizeof(*ack); i++)
		checksum += send[i];

	ack->checksum = ~checksum + 1;

#if 0
	uart_printf(debug, "  ack packet:\n");
	uart_printf(debug, "    header:\n");
	uart_printf(debug, "      version: %08x\n", ack->header.version);
	uart_printf(debug, "      type: %08x\n", ack->header.type);
	uart_printf(debug, "      sequence: %08x\n", ack->header.sequence);
	uart_printf(debug, "    checksum: %08x\n", ack->checksum);
#endif

	usb_ep_send(&usb->ep_bulk_in, ack, sizeof(*ack));
	usb_ep_wait(&usb->ep_bulk_in);
}

static void nv3p_send_status(struct usb *usb, uint32_t sequence, const char *message,
			     uint32_t code, uint32_t flags)
{
	struct nv3p_packet_status *status = (struct nv3p_packet_status *)send;
	uint32_t checksum = 0;
	unsigned int i;

	memset(status, 0, sizeof(*status));
	status->header.version = NV3P_VERSION;
	status->header.type = NV3P_PACKET_TYPE_COMMAND;
	status->header.sequence = sequence;
	status->length = sizeof(struct nv3p_command_status);
	status->command = NV3P_COMMAND_STATUS;
	strncpy(status->status.message, message, NV3P_STRING_MAX);
	status->status.code = code;
	status->status.flags = flags;

	for (i = 0; i < sizeof(*status); i++)
		checksum += send[i];

	status->checksum = ~checksum + 1;

#if 0
	uart_printf(debug, "  status packet:\n");
	uart_printf(debug, "    header:\n");
	uart_printf(debug, "      version: %08x\n", status->header.version);
	uart_printf(debug, "      type: %08x\n", status->header.type);
	uart_printf(debug, "      sequence: %08x\n", status->header.sequence);
	uart_printf(debug, "    length: %08x\n", status->length);
	uart_printf(debug, "    command: %08x\n", status->command);
	uart_printf(debug, "    status:\n");
	uart_printf(debug, "      message: %s\n", status->status.message);
	uart_printf(debug, "      code: %08x\n", status->status.code);
	uart_printf(debug, "      flags: %08x\n", status->status.flags);
	uart_printf(debug, "    checksum: %08x\n", status->checksum);
#endif

	usb_ep_send(&usb->ep_bulk_in, send, sizeof(*status));
	usb_ep_wait(&usb->ep_bulk_in);
}

static uint32_t payload_length;
static void *payload;

static void __attribute__((unused)) usb_nv3p_process(struct usb *usb)
{
	struct nv3p_header *header = (struct nv3p_header *)recv;
	uint32_t sequence = header->sequence;

	if (0) {
		uart_printf(debug, "nv3p packet:\n");
		uart_printf(debug, "  version: %08x\n", header->version);
		uart_printf(debug, "  type: %08x\n", header->type);
		uart_printf(debug, "  sequence: %08x\n", header->sequence);
	}

	/* command packet */
	if (header->type == NV3P_PACKET_TYPE_COMMAND) {
		struct nv3p_packet_command *packet = (struct nv3p_packet_command *)recv;

		if (0) {
			uart_printf(debug, "  command packet:\n");
			uart_printf(debug, "    length: %08x\n", packet->length);
			uart_printf(debug, "    command: %08x\n", packet->command);
			uart_printf(debug, "    checksum: %08x\n", packet->checksum);
		}

		if (packet->command == NV3P_COMMAND_DOWNLOAD_BCT) {
			struct nv3p_packet_download_bct *command = (struct nv3p_packet_download_bct *)recv;

			uart_printf(debug, "      NV3P_COMMAND_DOWNLOAD_BCT\n");
			uart_printf(debug, "        size: %u\n", command->size);
			uart_printf(debug, "        BCT: %u\n", sizeof(struct bct));

			payload = bct;
			payload_length = command->size;

			nv3p_send_ack(usb, sequence);
		}

		if (packet->command == NV3P_COMMAND_DOWNLOAD_BOOTLOADER) {
			struct nv3p_packet_download_bootloader *command = (struct nv3p_packet_download_bootloader *)recv;

			uart_printf(debug, "      NV3P_COMMAND_DOWNLOAD_BOOTLOADER\n");
			uart_printf(debug, "        length: %u\n", command->length);
			uart_printf(debug, "        size: %u\n", (uint32_t)command->size);
			uart_printf(debug, "        load: %x\n", command->load);
			uart_printf(debug, "        entry: %x\n", command->entry);

			nv3p_send_ack(usb, sequence);
			nv3p_send_status(usb, sequence, "OK", NV3P_STATUS_OK, 0);

			payload = (void *)command->load;
			payload_length = command->size;
			bootloader = (void (*)(void))command->entry;
		}
	}

	if (header->type == NV3P_PACKET_TYPE_DATA) {
		struct nv3p_packet_data *data = (struct nv3p_packet_data *)recv;
		unsigned int length, num, received = 0;
		uint32_t checksum;

		if (0) {
			uart_printf(debug, "  data packet:\n");
			uart_printf(debug, "    length: %u\n", data->length);
		}

		nv3p_checksum_init(&checksum);
		nv3p_checksum_update(&checksum, data, sizeof(*data));

		length = data->length;

#define BOUNCE_BUFFER

		while (received < length) {
#ifdef BOUNCE_BUFFER
			num = min(2048, sizeof(recv));
			usb_ep_recv(&usb->ep_bulk_out, recv, num);
#else
			num = 2048;
			usb_ep_recv(&usb->ep_bulk_out, payload + received, num);
#endif
			usb_ep_wait(&usb->ep_bulk_out);

			num -= ((usb->ep_bulk_out.dtd->info >> 16) & 0xffff);

			if (0)
				uart_printf(debug, "  received %u bytes\n", num);

			nv3p_checksum_update(&checksum, recv, num);

#ifdef BOUNCE_BUFFER
			if (0 && payload == bct) {
				uint32_t *ptr = (uint32_t *)recv;
				unsigned int i;

				uart_printf(debug, "verifying BCT chunk: %x (%u bytes)\n", received, num);

				for (i = 0; i < num / 4; i++) {
					if (ptr[i] != received / 4 + i)
						uart_printf(debug, "  corruption @%04x/%04x: %08x\n", i, received / 4 + i, ptr[i]);
				}
			}

			/*
			uart_printf(debug, "  copying %u bytes to %p\n", num, payload + received);
			*/
			memcpy(payload + received, recv, num);
#endif
			received += num;
		}

		nv3p_checksum_done(&checksum);
		payload_length -= received;

		if (payload == bct) {
			uart_printf(debug, "BCT checksum: %08x\n", checksum);
			bct_csum = checksum;
		}

		if (payload_length != 0)
			uart_printf(debug, "%u bytes remaining\n", payload_length);

		if (0 && payload == bct) {
			uint32_t *ptr = (uint32_t *)bct;
			unsigned int i;

			uart_printf(debug, "verifying BCT...\n");

			for (i = 0; i < sizeof(*bct) / 4; i++) {
				if (ptr[i] != i)
					uart_printf(debug, "  corruption @%04x: %p: %08x\n", i, &ptr[i], ptr[i]);
			}
		}

		if (payload == bct) {
			uint32_t value = readl(TEGRA_PMC_BASE + PMC_STRAPPING_OPT_A);
			unsigned int index, i;
			uint32_t checksum;

			for (i = 0; i < 10; i++) {
			nv3p_checksum_init(&checksum);
			nv3p_checksum_update(&checksum, bct, sizeof(*bct));
			nv3p_checksum_done(&checksum);
			uart_printf(debug, "BCT checksum: %08x\n", checksum);
			}

			uart_printf(debug, "    strapping: %08x\n", value);
			uart_printf(debug, "      RAM code: %02x\n", (value >> 4) & 0xf);
			uart_printf(debug, "      SDRAM: %02x\n", (value >> 4) & 0x3);

			index = (value >> 4) & 0x3;

			sdram_init(&bct->sdram_params[index]);

			if (0) {
				uint32_t *ptr = (uint32_t *)0x80000000;
				unsigned int i;

				uart_printf(debug, "memory: %p\n", ptr);

				for (i = 0; i < 16; i++)
					uart_printf(debug, "  %08x\n", ptr[i]);

				uart_printf(debug, "clearing: %p\n", ptr);

				for (i = 0; i < 16; i++)
					ptr[i] = 0xaa551100;

				uart_printf(debug, "memory: %p\n", ptr);

				for (i = 0; i < 16; i++)
					uart_printf(debug, "  %08x\n", ptr[i]);
				/*
				uart_hexdump(debug, ptr, 64, 16, true);
				uart_printf(debug, "  cleaning RAM...\n");
				memset(ptr, 0, 64);
				uart_printf(debug, "  done\n");
				uart_hexdump(debug, ptr, 64, 16, true);
				*/
			}

			if (1) {
				uint32_t *ptr = (uint32_t *)0x80000000;
				size_t size = 0x80000000;
				unsigned int i;

				uart_printf(debug, "writing memory %p-%p...\n", ptr, (void *)ptr + size - 1);

				for (i = 0; i < size / 4; i++) {
					if ((i % (1024 * 1024)) == 0)
						uart_printf(debug, "\r  %p...", &ptr[i]);

					ptr[i] = 0xaa551100;
				}

				uart_printf(debug, "done\n");

				uart_printf(debug, "validating memory %p-%p...\n", ptr, (void *)ptr + size - 1);

				for (i = 0; i < size / 4; i++) {
					if ((i % (1024 * 1024)) == 0)
						uart_printf(debug, "  %p\n", &ptr[i]);

					if (ptr[i] != 0xaa551100)
						uart_printf(debug, "  %p: %08x\n", &ptr[i], ptr[i]);
				}
			}
		}

		if (0 && payload == (void *)0x80108000) {
			uint32_t *ptr = (uint32_t *)payload;
			unsigned int i;

			uart_printf(debug, "verifying bootloader (%u bytes)...\n", received);

			for (i = 0; i < received / 4; i++) {
				if (ptr[i] != i)
					uart_printf(debug, "  corruption @%p: %08x\n", &ptr[i], ptr[i]);
			}
		}

		/* receive checksum */
		usb_ep_recv(&usb->ep_bulk_out, recv, sizeof(recv));
		usb_ep_wait(&usb->ep_bulk_out);

		nv3p_send_ack(usb, sequence);

		if (payload_length == 0)
			nv3p_send_status(usb, sequence, "OK", NV3P_STATUS_OK, 0);
	}

	/* queue another receive buffer */
	usb_ep_recv(&usb->ep_bulk_out, recv, sizeof(recv));
}

void usb_init(struct usb *usb)
{
	uint32_t value;

	uart_printf(debug, "> %s(usb=%p)\n", __func__, usb);
	uart_printf(debug, "  base: %#lx\n", usb->base);

#define ALIGN_MASK(value, mask) \
	(((value) + (mask)) & ~(mask))
#define ALIGN(value, align) \
	ALIGN_MASK(value, (typeof(value))(align) - 1)

	/* XXX move this somewhere more appropriate */
#if 0
	uart_printf(debug, "  safe start address: %08x\n", bit->safe_start_address);
	uart_printf(debug, "  BCT: %08x\n", bit->bct);
	bit->safe_start_address = ALIGN(bit->safe_start_address, 32);
	bct = (struct bct *)bit->safe_start_address;
	uart_printf(debug, "  putting BCT at %p\n", bct);
	bit->safe_start_address += sizeof(*bct);
	uart_printf(debug, "  safe start address: %08x\n", bit->safe_start_address);
#endif

	if (0)
		request_irq(INT_USB, usb_irq, usb, 0);

	if (1) {
		unsigned long base = TEGRA_CLK_RST_BASE;
		uint32_t value;

#if 1
		value = readl(base + 0x19c);
		uart_printf(debug, "CLK_SOURCE_EMC: %08x\n", value);
		value &= ~0x7 << 29;
		value |= 0x2 << 29;
		writel(value, base + 0x19c);

		value = readl(base + 0x014);
		value |= 1 << 25;
		writel(value, base + 0x014);

		value = readl(base + 0x014);
		value |= 1 << 0;
		writel(value, base + 0x014);

		value = readl(base + 0x008);
		value &= ~(1 << 25);
		writel(value, base + 0x008);

		value = readl(base + 0x008);
		value &= ~(1 << 0);
		writel(value, base + 0x008);

		udelay(5);

		value = readl(base + 0x3a4);
		value |= 1 << 19;
		writel(value, base + 0x3a4);

		base = TEGRA_MC_BASE;

		value = readl(base + 0x964);
		value &= ~(1 << 0);
		writel(value, base + 0x964);

		writel(0x40000000, base + 0x95c);
		writel(0x4003f000, base + 0x960);

		/* flush writes */
		readl(base + 0x964);
#endif

		base = TEGRA_CLK_RST_BASE;

		value = readl(base + 0x058);
		value &= ~(0x7 << 0);
		value |= (1 << 31);
		writel(value, base + 0x058);

		while (true) {
			value = readl(base + 0x05c);
			if ((value & (1 << 31)) == 0)
				break;
		}

		/* determine input clock frequency from value */
		uart_printf(debug, "value: %08x\n", value);

		/* assume 12 MHz for now */
		value = (0x00 << 8) | (0x0b << 0);
		writel(value, 0x60005010 + 0x004);

		value = (0x08 << 28) | (0 << 26) | (0 << 18) | (0 << 12) | (0x01 << 4) | (1 << 0);
		writel(value, base + 0x050);

		value = (0 << 7) | (1 << 4) | (0 << 3) | (0 << 0);
		writel(value, base + 0x030);
	}

	if (1) {
		unsigned int i;

		for (i = 0; i < 2; i++) {
			value = usb_readl(usb, ENDPTCTRL(i));
			uart_printf(debug, "ENDPTCTRL(%u): %08x\n", i, value);
		}

		value = usb_readl(usb, ASYNCLISTADDR);
		uart_printf(debug, "ASYNCLISTADDR: %08x\n", value);
	}

#if 0
	value = usb_readl(usb, USB_SUSP_CTRL);
	value |= USB_SUSP_CTRL_UTMIP_RESET;
	usb_writel(usb, value, USB_SUSP_CTRL);

	value = usb_readl(usb, USB_SUSP_CTRL);
	value &= ~USB_SUSP_CTRL_UTMIP_RESET;
	usb_writel(usb, value, USB_SUSP_CTRL);
#endif

	/* stop controller */
	value = usb_readl(usb, USBCMD);
	value &= ~USBCMD_RUN;
	usb_writel(usb, value, USBCMD);

	/* switch to idle mode */
	value = usb_readl(usb, USBMODE);
	value &= ~USBMODE_CM_MASK;
	usb_writel(usb, value, USBMODE);

	/* reset controller */
	value = usb_readl(usb, USBCMD);
	value |= USBCMD_ITC(8) | USBCMD_RST;
	usb_writel(usb, value, USBCMD);

	if (0)
		uart_printf(debug, "resetting USB controller\n");

	/* wait for reset */
	while (true) {
		value = usb_readl(usb, USBCMD);
		if ((value & USBCMD_RST) == 0)
			break;
	}

	if (0)
		uart_printf(debug, "done\n");

	/* wait for PHY clock to stabilize */
	while (true) {
		value = usb_readl(usb, USB_SUSP_CTRL);
		if (value & USB_SUSP_CTRL_USB_PHY_CLK_VALID)
			break;
	}

	if (0)
		uart_printf(debug, "PHY clock stabilized\n");

	/* switch to device mode */
	value = usb_readl(usb, USBMODE);
	value |= USBMODE_CM_DEVICE;
	usb_writel(usb, value, USBMODE);

	/* wait for mode switch to complete */
	while (true) {
		value = usb_readl(usb, USBMODE);
		if ((value & USBMODE_CM_MASK) == USBMODE_CM_DEVICE)
			break;
	}

	if (0)
		uart_printf(debug, "device mode\n");

	/* this will be reset to 0 during the reset sequence above */
	usb_writel(usb, usb->base + 0x1000, ASYNCLISTADDR);

	usb_ep_setup(&usb->ep_ctrl_out);
	usb_ep_setup(&usb->ep_ctrl_in);

	if (0) {
		value = usb_readl(usb, HOSTPC1_DEVLC);
		value &= ~HOSTPC1_DEVLC_ASUS;
		usb_writel(usb, value, HOSTPC1_DEVLC);
	}

	value = usb_readl(usb, USBINTR);
	value &= ~(USBINTR_SLE | USBINTR_SRE | USBINTR_AAE | USBINTR_SEE |
		   USBINTR_FRE);
	value |= USBINTR_URE | USBINTR_PCE | USBINTR_UEE | USBINTR_UE;
	usb_writel(usb, value, USBINTR);

	if (0)
		uart_printf(debug, "starting USB controller...\n");

	/* start controller */
	value = usb_readl(usb, USBCMD);
	value |= USBCMD_RUN;
	usb_writel(usb, value, USBCMD);

	while (true) {
		value = usb_readl(usb, USBCMD);
		if ((value & USBCMD_RUN) != 0)
			break;
	}

	if (0)
		uart_printf(debug, "done\n");

	if (0) {
		unsigned int i;

		for (i = 0; i < 2; i++) {
			value = usb_readl(usb, ENDPTCTRL(i));
			uart_printf(debug, "ENDPTCTRL(%u): %08x\n", i, value);
		}
	}

	uart_printf(debug, "< %s()\n", __func__);
}

static void usb_reset(struct usb *usb)
{
	uint32_t value;

	value = PERIODICLISTBASE_USBADR(0);
	usb_writel(usb, value, PERIODICLISTBASE);

	value = usb_readl(usb, ENDPTSETUPSTAT);
	usb_writel(usb, value, ENDPTSETUPSTAT);

	value = usb_readl(usb, ENDPTCOMPLETE);
	usb_writel(usb, value, ENDPTCOMPLETE);

	/* flush endpoints */
	usb_writel(usb, 0xffffffff, ENDPTFLUSH);

	while (true) {
		value = usb_readl(usb, ENDPTFLUSH);
		if (value == 0)
			break;
	}

	while (true) {
		value = usb_readl(usb, ENDPTSTATUS);
		if (value == 0)
			break;
	}

	while (true) {
		value = usb_readl(usb, ENDPTPRIME);
		if (value == 0)
			break;
	}
}

int usb_enumerate(struct usb *usb)
{
	uint32_t status, value;

	while (true) {
		status = usb_readl(usb, USBSTS);
		usb_writel(usb, status, USBSTS);

		if ((status & ~USBSTS_SRI) != 0) {
			if (0)
				uart_printf(debug, "USBSTS: %08x\n", status);
		}

		if (status & USBSTS_URI) {
			uart_printf(debug, "usb: reset detected\n");
			usb_reset(usb);
		}

		if (status & USBSTS_PCI) {
			uart_printf(debug, "usb: port change detected\n");
			value = usb_readl(usb, HOSTPC1_DEVLC);
			uart_printf(debug, "  HOSTPC1_DEVLC: %08x\n", value);
		}

		if (status & USBSTS_UEI)
			uart_printf(debug, "usb: error detected\n");

		if (status & USBSTS_UI) {
			if (0)
				uart_printf(debug, "usb: interrupt\n");

			value = usb_readl(usb, ENDPTSETUPSTAT);
			if (value) {
				usb_writel(usb, value, ENDPTSETUPSTAT);
				if (usb_process_setup(usb))
					break;
			}
		}
	}

	usb_ep_setup(&usb->ep_bulk_out);
	usb_ep_setup(&usb->ep_bulk_in);

	return 0;
}

static bool aligned(const void *ptr, unsigned int align)
{
	return ((unsigned long)ptr & (align - 1)) == 0;
}

ssize_t usb_bulk_recv(struct usb *usb, void *buffer, size_t size)
{
	void *payload;

	/*
	 * If the buffer isn't properly aligned the data must be bounced, in
	 * which case the size can't exceed that of the bounce buffer.
	 *
	 * TODO: Perhaps we can transfer in chunks in that case?
	 */
	if (size > sizeof(bounce) && !aligned(buffer, 32))
		return -EINVAL;

	if (aligned(buffer, 32) == 0)
		payload = bounce;
	else
		payload = buffer;

	usb_ep_recv(&usb->ep_bulk_out, payload, size);
	usb_ep_wait(&usb->ep_bulk_out);

	size -= (usb->ep_bulk_out.dtd->info >> 16) & 0xffff;

	if (payload == bounce)
		memcpy(buffer, bounce, size);

	return size;
}

ssize_t usb_bulk_send(struct usb *usb, const void *buffer, size_t size)
{
	const void *payload;

	/*
	 * If the buffer isn't properly aligned the data must be bounced, in
	 * which case the size can't exceed that of the bounce buffer.
	 *
	 * TODO: Perhaps we can transfer in chunks in that case?
	 */
	if (size > sizeof(bounce) && !aligned(buffer, 32))
		return -EINVAL;

	if (aligned(buffer, 32) == 0) {
		uart_printf(debug, "using bounce buffer\n");
		memcpy(bounce, buffer, size);
		payload = bounce;
	} else
		payload = buffer;

	usb_ep_send(&usb->ep_bulk_in, payload, size);
	usb_ep_wait(&usb->ep_bulk_in);

	return size - ((usb->ep_bulk_out.dtd->info >> 16) & 0xffff);
}
