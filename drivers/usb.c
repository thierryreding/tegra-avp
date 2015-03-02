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
#define  HOSTPC1_DEVLC_PSPD_SHIFT 25
#define  HOSTPC1_DEVLC_PSPD_MASK  3
#define  HOSTPC1_DEVLC_FULL_SPEED (0 << 25)
#define  HOSTPC1_DEVLC_LOW_SPEED  (1 << 25)
#define  HOSTPC1_DEVLC_HIGH_SPEED (2 << 25)
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
#define DQH_INFO_HALTED (1 << 6)
#define DQH_INFO_DATA_BUFFER_ERROR (1 << 5)
#define DQH_INFO_TRANSACTION_ERROR (1 << 3)
#define DQH_INFO_ERROR_MASK (DQH_INFO_HALTED | DQH_INFO_DATA_BUFFER_ERROR | \
			     DQH_INFO_TRANSACTION_ERROR)

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
static uint8_t bounce[4096] __aligned(4096);
static struct usb_dtd dtd[4] __aligned(32);

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

#define USB_LANGUAGE_ID 0x00
#define USB_MANUFACTURER_ID 0x01
#define USB_PRODUCT_ID 0x02
#define USB_SERIAL_ID 0x03

static const uint8_t language_ids[] __aligned(32) = {
	4,
	USB_DESCRIPTOR_TYPE_STRING,
	0x09, 0x04
};

static const uint8_t manufacturer[] __aligned(32) = {
	14,
	USB_DESCRIPTOR_TYPE_STRING,
	'N', 0,
	'V', 0,
	'I', 0,
	'D', 0,
	'I', 0,
	'A', 0,
};

static const uint8_t product[] __aligned(32) = {
	8,
	USB_DESCRIPTOR_TYPE_STRING,
	'A', 0,
	'P', 0,
	'X', 0,
};

static const uint8_t serial[] __aligned(32) = {
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

static struct usb_device_descriptor device_descriptor __aligned(32) = {
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
} __packed;

static struct usb_configuration configuration __aligned(32) = {
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

	usb_ep_prime(ep);
}

static void usb_ep_wait(struct usb_ep *ep)
{
	uint32_t value, mask = usb_ep_mask(ep);
	unsigned int retries = 2000;
	uint32_t prime, status;

	if (ep->dqh->info & DQH_INFO_ERROR_MASK)
		uart_printf(debug, "error detected: %08x\n", ep->dqh->info);

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

	retries = 2000;

	while (retries) {
		value = usb_readl(ep->usb, ENDPTCOMPLETE);
		if ((value & mask) != 0)
			break;

		udelay(1000);
		retries--;
	}

	if (retries == 0)
		uart_printf(debug, "timed out: complete: %08x\n", value);

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

static int __maybe_unused usb_irq(unsigned int irq, void *data)
{
	struct usb *usb = data;
	uint32_t value;

	value = usb_readl(usb, USBSTS);
	usb_writel(usb, value, USBSTS);

	return IRQ_HANDLED;
}

void usb_init(struct usb *usb)
{
	uint32_t value;

	value = usb_readl(usb, USB_SUSP_CTRL);
	value |= USB_SUSP_CTRL_UTMIP_RESET;
	usb_writel(usb, value, USB_SUSP_CTRL);

	value = usb_readl(usb, USB_SUSP_CTRL);
	value &= ~USB_SUSP_CTRL_UTMIP_RESET;
	usb_writel(usb, value, USB_SUSP_CTRL);

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

	/* wait for reset */
	while (true) {
		value = usb_readl(usb, USBCMD);
		if ((value & USBCMD_RST) == 0)
			break;
	}

	/* wait for PHY clock to stabilize */
	while (true) {
		value = usb_readl(usb, USB_SUSP_CTRL);
		if (value & USB_SUSP_CTRL_USB_PHY_CLK_VALID)
			break;
	}

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

	/* start controller */
	value = usb_readl(usb, USBCMD);
	value |= USBCMD_RUN;
	usb_writel(usb, value, USBCMD);

	while (true) {
		value = usb_readl(usb, USBCMD);
		if ((value & USBCMD_RUN) != 0)
			break;
	}
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

		if ((status & ~USBSTS_SRI) != 0)
			if (0)
				uart_printf(debug, "USBSTS: %08x\n", status);

		if (status & USBSTS_URI) {
			uart_printf(debug, "usb: reset detected\n");
			usb_reset(usb);
		}

		if (status & USBSTS_PCI) {
			unsigned int speed;

			uart_printf(debug, "usb: port change detected");

			value = usb_readl(usb, HOSTPC1_DEVLC);
			speed = (value >> HOSTPC1_DEVLC_PSPD_SHIFT) &
					HOSTPC1_DEVLC_PSPD_MASK;

			switch (speed) {
			case 0:
				uart_printf(debug, " (full-speed)");
				break;

			case 1:
				uart_printf(debug, " (low-speed)");
				break;

			case 2:
				uart_printf(debug, " (high-speed)");
				break;
			}

			uart_printf(debug, "\n");
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
		memcpy(bounce, buffer, size);
		payload = bounce;
	} else
		payload = buffer;

	usb_ep_send(&usb->ep_bulk_in, payload, size);
	usb_ep_wait(&usb->ep_bulk_in);

	return size - ((usb->ep_bulk_out.dtd->info >> 16) & 0xffff);
}
