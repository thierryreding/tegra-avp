#include <common.h>
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
};

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

#if 0
void usb_ep_dqh_read(struct usb_ep *ep, struct usb_dqh *dqh)
{
	dqh->capabilities = usb_readl(ep->usb, ep->qh + 0x00);
	dqh->current = usb_readl(ep->usb, ep->qh + 0x04);
	dqh->next = usb_readl(ep->usb, ep->qh + 0x08);
	dqh->token = usb_readl(ep->usb, ep->qh + 0x0c);
	dqh->buffers[0] = usb_readl(ep->usb, ep->qh + 0x10);
	dqh->buffers[1] = usb_readl(ep->usb, ep->qh + 0x14);
	dqh->buffers[2] = usb_readl(ep->usb, ep->qh + 0x18);
	dqh->buffers[3] = usb_readl(ep->usb, ep->qh + 0x1c);
	dqh->buffers[4] = usb_readl(ep->usb, ep->qh + 0x20);
	dqh->reserved0 = usb_readl(ep->usb, ep->qh + 0x24);
	dqh->setup[0] = usb_readl(ep->usb, ep->qh + 0x28);
	dqh->setup[1] = usb_readl(ep->usb, ep->qh + 0x2c);
	dqh->reserved1 = usb_readl(ep->usb, ep->qh + 0x30);
	dqh->reserved2 = usb_readl(ep->usb, ep->qh + 0x34);
	dqh->reserved3 = usb_readl(ep->usb, ep->qh + 0x38);
	dqh->reserved4 = usb_readl(ep->usb, ep->qh + 0x3c);
}

void usb_ep_dqh_write(struct usb_ep *ep, struct usb_dqh *dqh)
{
	usb_writel(ep->usb, dqh->capabilities, ep->qh + 0x00);
	usb_writel(ep->usb, dqh->current, ep->qh + 0x04);
	usb_writel(ep->usb, dqh->next, ep->qh + 0x08);
	usb_writel(ep->usb, dqh->token, ep->qh + 0x0c);
	usb_writel(ep->usb, dqh->buffers[0], ep->qh + 0x10);
	usb_writel(ep->usb, dqh->buffers[1], ep->qh + 0x14);
	usb_writel(ep->usb, dqh->buffers[2], ep->qh + 0x18);
	usb_writel(ep->usb, dqh->buffers[3], ep->qh + 0x1c);
	usb_writel(ep->usb, dqh->buffers[4], ep->qh + 0x20);
	usb_writel(ep->usb, dqh->reserved0, ep->qh + 0x24);
	usb_writel(ep->usb, dqh->setup[0], ep->qh + 0x28);
	usb_writel(ep->usb, dqh->setup[1], ep->qh + 0x2c);
	usb_writel(ep->usb, dqh->reserved1, ep->qh + 0x30);
	usb_writel(ep->usb, dqh->reserved2, ep->qh + 0x34);
	usb_writel(ep->usb, dqh->reserved3, ep->qh + 0x38);
	usb_writel(ep->usb, dqh->reserved4, ep->qh + 0x3c);
}

static void usb_dqh_dump(struct uart *uart, struct usb_dqh *dqh)
{
	uart_printf(uart, "caps: %08x\n", dqh->caps);
	uart_printf(uart, "current: %08x\n", dqh->current);
	uart_printf(uart, "next: %08x\n", dqh->next);
	uart_printf(uart, "info: %08x\n", dqh->info);
	uart_printf(uart, "buffers: 0: %08x\n", dqh->buffers[0]);
	uart_printf(uart, "         1: %08x\n", dqh->buffers[1]);
	uart_printf(uart, "         2: %08x\n", dqh->buffers[2]);
	uart_printf(uart, "         3: %08x\n", dqh->buffers[3]);
	uart_printf(uart, "         4: %08x\n", dqh->buffers[4]);
	uart_printf(uart, "reserved0: %08x\n", dqh->reserved0);
	uart_printf(uart, "setup: 0: %08x\n", dqh->setup[0]);
	uart_printf(uart, "       1: %08x\n", dqh->setup[1]);
	uart_printf(uart, "reserved1: %08x\n", dqh->reserved1);
	uart_printf(uart, "reserved2: %08x\n", dqh->reserved2);
	uart_printf(uart, "reserved3: %08x\n", dqh->reserved3);
	uart_printf(uart, "reserved4: %08x\n", dqh->reserved4);
}
#endif

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

struct usb_ep ep_ctrl_out = {
	.usb = &usbd,
	.index = 0,
	.direction = OUT,
	.type = CTRL,
	.dtd = &dtd[0],
};

struct usb_ep ep_ctrl_in = {
	.usb = &usbd,
	.index = 0,
	.direction = IN,
	.type = CTRL,
	.dtd = &dtd[1],
};

struct usb_ep ep_bulk_out = {
	.usb = &usbd,
	.index = 1,
	.direction = OUT,
	.type = BULK,
	.dtd = &dtd[2],
};

struct usb_ep ep_bulk_in = {
	.usb = &usbd,
	.index = 1,
	.direction = IN,
	.type = BULK,
	.dtd = &dtd[3],
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

	/*
	uart_printf(debug, "ep: %u %s dqh: %p\n", ep->index,
		    (ep->direction == IN) ? "IN" : "OUT", ep->dqh);
	*/

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

	//uart_printf(debug, "> %s(ep=%p)\n", __func__, ep);
	//uart_printf(debug, "  mask: %08x\n", mask);

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

	//uart_printf(debug, "< %s()\n", __func__);
}

static void usb_ep_prime(struct usb_ep *ep)
{
	uint32_t mask = usb_ep_mask(ep), value;

	//uart_printf(debug, "priming: %08x\n", mask);
	usb_writel(ep->usb, mask, ENDPTPRIME);

	while (true) {
		value = usb_readl(ep->usb, ENDPTPRIME);
		if ((value & mask) == 0)
			break;
	}
}

static uint8_t recv[4096] __attribute__((aligned(32)));
static uint8_t send[4096] __attribute__((aligned(32)));
static struct bit *bit = (struct bit *)0x40000000;
static void (*bootloader)(void) = NULL;
static struct bct _bct __attribute__((aligned(32)));
static struct bct *bct = &_bct;

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

#if 0
static void usb_ep_ack(struct usb_ep *ep)
{
	struct usb_dqh *dqh = (struct usb_dqh *)(ep->usb->base + ep->qh);
	uint32_t value;

	uart_printf(debug, "> %s(ep=%p)\n", __func__, ep);
	uart_printf(debug, "  dqh: %p\n", dqh);

	dqh->capabilities = DQH_CAPS_ZLT | (512 << 16) | DQH_CAPS_IOC;
	dqh->current = 0;
	dqh->next = ((uint32_t)&dtd) & 0xffffff80;
	dqh->token = 0;
	dqh->buffers[0] = 0;
	dqh->buffers[1] = 0;
	dqh->buffers[2] = 0;
	dqh->buffers[3] = 0;
	dqh->buffers[4] = 0;
	dqh->reserved0 = 0;
	dqh->setup[0] = 0;
	dqh->setup[1] = 0;
	dqh->reserved1 = 0;
	dqh->reserved2 = 0;
	dqh->reserved3 = 0;
	dqh->reserved4 = 0;

	dtd.next = DTD_NEXT_TERMINATE;
	/* transfer 0 bytes */
	dtd.token = (0 << 16) | DTD_TOKEN_ACTIVE;
	dtd.buffers[0] = 0;

	uart_printf(debug, "    next: %08x\n", dqh->next);

	value = usb_readl(ep->usb, ENDPTPRIME);
	value |= usb_ep_mask(ep);
	uart_printf(debug, "  ENDPTPRIME: %08x\n", value);
	usb_writel(ep->usb, value, ENDPTPRIME);

#if 0
	while (retries--) {
		value = usb_readl(ep->usb, USBSTS);
		uart_printf(debug, "  USBSTS: %08x\n", value);
		value = usb_readl(ep->usb, ENDPTSTATUS);
		uart_printf(debug, "  ENDPTSTATUS: %08x\n", value);
		value = usb_readl(ep->usb, ENDPTPRIME);
		uart_printf(debug, "  ENDPTPRIME: %08x\n", value);
		value = usb_readl(ep->usb, ENDPTCOMPLETE);
		uart_printf(debug, "  ENDPTCOMPLETE: %08x\n", value);
		udelay(1000000);
	}
#endif

	uart_printf(debug, "< %s()\n", __func__);
}
#endif

static void usb_ep_send(struct usb_ep *ep, const void *buffer, size_t size)
{
	uart_printf(debug, "> %s(ep=%p, buffer=%p, size=%zu)\n", __func__, ep,
		    buffer, size);

	ep->dqh->caps = DQH_CAPS_ZLT | DQH_CAPS_MAX_PKT_LEN(ep->max_pkt_len) |
			DQH_CAPS_IOC;
	ep->dqh->next = (uint32_t)ep->dtd & ~0x1f;

	ep->dtd->next = DTD_NEXT_TERMINATE;
	ep->dtd->info = DTD_INFO_TOTAL_BYTES(size) | DTD_INFO_IOC |
			DTD_INFO_ACTIVE;
	ep->dtd->buffers[0] = (uint32_t)buffer;

	usb_ep_prime(ep);

	uart_printf(debug, "< %s()\n", __func__);
}

static void usb_ep_recv(struct usb_ep *ep, void *buffer, size_t size)
{
	if (0) {
		uart_printf(debug, "> %s(ep=%p, buffer=%p, size=%zu)\n",
			    __func__, ep, buffer, size);
	}

	memset(ep->dqh, 0, sizeof(*ep->dqh));
	ep->dqh->caps = DQH_CAPS_ZLT | DQH_CAPS_MAX_PKT_LEN(ep->max_pkt_len);
	ep->dqh->next = (uint32_t)ep->dtd & ~0x1f;

	memset(ep->dtd, 0, sizeof(*ep->dtd));
	ep->dtd->next = DTD_NEXT_TERMINATE;
	ep->dtd->info = DTD_INFO_TOTAL_BYTES(size) | DTD_INFO_IOC |
			DTD_INFO_ACTIVE;
	ep->dtd->buffers[0] = (uint32_t)buffer & ~0x1f;

	if (0) {
		uart_printf(debug, "  dqh: %p\n", ep->dqh);
		uart_printf(debug, "    caps: %08x\n", ep->dqh->caps);
		uart_printf(debug, "    next: %08x\n", ep->dqh->next);
		uart_printf(debug, "    info: %08x\n", ep->dqh->info);
	}

	if (0) {
		unsigned int i;

		uart_printf(debug, "  dtd: %p\n", ep->dtd);
		uart_printf(debug, "    next: %08x\n", ep->dtd->next);
		uart_printf(debug, "    info: %08x\n", ep->dtd->info);
		uart_printf(debug, "    buffers:\n");

		for (i = 0; i < 5; i++)
			uart_printf(debug, "      [%u]: %08x\n", i, ep->dtd->buffers[i]);

		uart_printf(debug, "    reserved: %08x\n", ep->dtd->reserved);
	}

	usb_ep_prime(ep);

	if (1)
		uart_printf(debug, "< %s()\n", __func__);
}

static void usb_ep_wait(struct usb_ep *ep)
{
	uint32_t value, mask = usb_ep_mask(ep);

#if 1
	uart_printf(debug, "> %s(ep=%p)\n", __func__, ep);
#endif

	while (true) {
		value = usb_readl(ep->usb, ENDPTCOMPLETE);
		if ((value & mask) != 0)
			break;
	}

#if 0
	uart_printf(debug, "endpoint %p ready\n", ep);
	uart_printf(debug, "  dqh: %p\n", ep->dqh);
	uart_printf(debug, "    caps: %08x\n", ep->dqh->caps);
	uart_printf(debug, "    curr: %08x\n", ep->dqh->current);
	uart_printf(debug, "    next: %08x\n", ep->dqh->next);
	uart_printf(debug, "    info: %08x\n", ep->dqh->info);
	uart_printf(debug, "    buffers:\n");
	uart_printf(debug, "      [0]: %08x\n", ep->dqh->buffers[0]);
	uart_printf(debug, "      [1]: %08x\n", ep->dqh->buffers[1]);
	uart_printf(debug, "      [2]: %08x\n", ep->dqh->buffers[2]);
	uart_printf(debug, "      [3]: %08x\n", ep->dqh->buffers[3]);
	uart_printf(debug, "      [4]: %08x\n", ep->dqh->buffers[4]);
#endif

	usb_writel(ep->usb, mask, ENDPTCOMPLETE);

	value = usb_readl(ep->usb, ENDPTSTATUS);
	uart_printf(debug, "  ENDPTSTATUS: %08x\n", value);

	value = usb_readl(ep->usb, ENDPTPRIME);
	uart_printf(debug, "  ENDPTPRIME: %08x\n", value);

#if 1
	uart_printf(debug, "< %s()\n", __func__);
#endif
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

static void handle_setup_get_descriptor(struct usb_ep *ep)
{
	struct usb_setup *setup = (struct usb_setup *)ep->dqh->setup;
	uint8_t index = setup->wValue & 0xff;
	uint8_t type = setup->wValue >> 8;
	const void *data = NULL;
	size_t size = 0;

	uart_printf(debug, "GET_DESCRIPTOR: %02x %02x\n", type, index);

	switch (type) {
	case USB_DESCRIPTOR_TYPE_DEVICE:
		uart_printf(debug, "  device: %u\n", setup->wLength);

		prepare_device_descriptor(&device_descriptor);
		usb_ep_send(&ep_ctrl_in, &device_descriptor,
			    sizeof(device_descriptor));
		usb_ep_send(&ep_ctrl_out, NULL, 0);
		break;

	case USB_DESCRIPTOR_TYPE_CONFIGURATION:
		uart_printf(debug, "  configuration: %u\n", setup->wLength);

		if (setup->wLength == sizeof(configuration.configuration)) {
			size = sizeof(configuration.configuration);
			data = &configuration.configuration;
		} else {
			size = sizeof(configuration);
			data = &configuration;
		}

		usb_ep_send(&ep_ctrl_in, data, size);
		usb_ep_send(&ep_ctrl_out, NULL, 0);
		break;

	case USB_DESCRIPTOR_TYPE_STRING:
		uart_printf(debug, "  string: %u\n", setup->wLength);

		switch (setup->wValue & 0xff) {
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

		usb_ep_send(&ep_ctrl_in, data, size);
		usb_ep_send(&ep_ctrl_out, NULL, 0);
		break;
	}
}

static int handle_setup(struct usb_ep *ep)
{
	struct usb_setup *setup = (struct usb_setup *)ep->dqh->setup;
	uint32_t value;

	if (0) {
		uart_printf(debug, "setup: %p (dQH: %p)\n", setup, ep->dqh);
		uart_printf(debug, "  bmRequestType: %02x\n", setup->bmRequestType);
		uart_printf(debug, "  bRequest: %02x\n", setup->bRequest);
		uart_printf(debug, "  wValue: %04x\n", setup->wValue);
		uart_printf(debug, "  wIndex: %04x\n", setup->wIndex);
		uart_printf(debug, "  wLength: %04x\n", setup->wLength);
	}

	switch (setup->bmRequestType) {
	case USB_SETUP_REQUEST_TYPE_DEVICE_TO_HOST:
		switch (setup->bRequest) {
		case USB_SETUP_REQUEST_GET_DESCRIPTOR:
			handle_setup_get_descriptor(ep);

			break;
		}

	case USB_SETUP_REQUEST_TYPE_HOST_TO_DEVICE:
		switch (setup->bRequest) {
		case USB_SETUP_REQUEST_SET_ADDRESS:
			uart_printf(debug, "SET_ADDRESS: %04x\n", setup->wValue);
			usb_ep_send(&ep_ctrl_in, NULL, 0);
			usb_ep_wait(&ep_ctrl_in);

			value = PERIODICLISTBASE_USBADR(setup->wValue);
			usb_writel(ep->usb, value, PERIODICLISTBASE);
			break;

		case USB_SETUP_REQUEST_SET_CONFIGURATION:
			uart_printf(debug, "SET_CONFIGURATION: %04x\n", setup->wValue);
			usb_ep_send(&ep_ctrl_in, NULL, 0);
			break;
		}
	}

	return 0;
}

#if 0
static bool usb_ep_ready(struct usb_ep *ep)
{
	uint32_t status, prime, mask;

	uart_printf(debug, "> %s(ep=%p)\n", __func__, ep);

	status = usb_readl(ep->usb, ENDPTSTATUS);
	prime = usb_readl(ep->usb, ENDPTPRIME);
	mask = usb_ep_mask(ep);

	uart_printf(debug, "  status: %08x, prime: %08x, mask: %08x\n", status, prime, mask);

	uart_printf(debug, "< %s()\n", __func__);

	return (status & mask) == 0 && (prime & mask) == 0;
}
#endif

static struct usb_ep *usb_ep_get(unsigned int index)
{
	switch (index) {
	case 0:
		return &ep_ctrl_out;

	case 1:
		return &ep_ctrl_in;

	case 2:
		return &ep_bulk_out;

	case 3:
		return &ep_bulk_in;
	}

	return NULL;
}

#define NV3P_VERSION 0x01

#define NV3P_PACKET_TYPE_COMMAND 0x01
#define NV3P_PACKET_TYPE_DATA 0x02
#define NV3P_PACKET_TYPE_ACK 0x04

#define NV3P_COMMAND_GET_PLATFORM_INFO 0x01
#define NV3P_COMMAND_DOWNLOAD_BCT 0x04
#define NV3P_COMMAND_DOWNLOAD_BOOTLOADER 0x06
#define NV3P_COMMAND_STATUS 0x0a

#define NV3P_STATUS_OK 0x00

struct nv3p_header {
	uint32_t version;
	uint32_t type;
	uint32_t sequence;
};

struct nv3p_packet_command {
	struct nv3p_header header;
	uint32_t length;
	uint32_t command;
	uint32_t checksum;
};

struct nv3p_packet_download_bct {
	struct nv3p_header header;
	uint32_t length;
	uint32_t command;
	uint32_t size;
	uint32_t checksum;
};

struct nv3p_packet_download_bootloader {
	struct nv3p_header header;
	uint32_t length;
	uint32_t command;
	uint64_t size;
	uint32_t load;
	uint32_t entry;
	uint32_t checksum;
} __attribute__((packed));

struct nv3p_packet_ack {
	struct nv3p_header header;
	uint32_t checksum;
};

#define NV3P_STRING_MAX 32

struct nv3p_command_status {
	char message[NV3P_STRING_MAX];
	uint32_t code;
	uint32_t flags;
};

struct nv3p_packet_status {
	struct nv3p_header header;
	uint32_t length;
	uint32_t command;
	struct nv3p_command_status status;
	uint32_t checksum;
};

struct nv3p_packet_data {
	struct nv3p_header header;
	uint32_t length;
};

struct nv3p_chip_id {
	uint16_t id;
	uint8_t major;
	uint8_t minor;
} __attribute__((packed));

struct nv3p_board_id {
	uint32_t board_no;
	uint32_t fab;
	uint32_t mem_type;
	uint32_t freq;
} __attribute__((packed));

struct nv3p_platform_info {
	uint32_t uid[4];
	struct nv3p_chip_id chip;
	uint32_t sku;
	uint32_t version;
	uint32_t boot_device;
	uint32_t op_mode;
	uint32_t dev_conf_strap;
	uint32_t dev_conf_fuse;
	uint32_t sdram_conf_strap;
	uint32_t reserved[2];
	struct nv3p_board_id board_id;
	/*
	uint32_t warranty_fuse;
	uint32_t skip_auto_detect;
	*/
} __attribute__((packed));

struct nv3p_packet_platform_info {
	struct nv3p_header header;
	uint32_t length;
	struct nv3p_platform_info info;
	uint32_t checksum;
} __attribute__((packed));

static void prepare_platform_info(struct nv3p_platform_info *info)
{
	uint32_t value;

	value = readl(TEGRA_APB_MISC_BASE + 0x804);
	info->chip.id = (value >> 8) & 0xff;
	info->chip.minor = (value >> 16) & 0xf;
	info->chip.major = (value >> 4) & 0xf;

	value = readl(TEGRA_FUSE_BASE + 0x110);
	info->sku = value & 0xf;

	info->version = 0x1;
	info->boot_device = 0x3;

#define TEGRA_PMC_BASE 0x7000e400
#define PMC_STRAPPING_OPT_A 0x64

	value = readl(TEGRA_PMC_BASE + PMC_STRAPPING_OPT_A);
	uart_printf(debug, "strapping options: %08x\n", (value >> 26) & 0xf);
	/* TODO: map stapping options to boot device */
	info->boot_device = 0x2;

	value = readl(TEGRA_FUSE_BASE + 0x1a0);
	if (value) {
		uart_printf(debug, "security mode\n");

		value = readl(TEGRA_FUSE_BASE + 0x268);
		if (value) {
			unsigned int i;

			value = 0;

			for (i = 0; i < 4; i++)
				value |= readl(TEGRA_FUSE_BASE + 0x050 + (i * 4));

			if (value)
				info->op_mode = 0x4;
			else
				info->op_mode = 0x5;
		} else {
			info->op_mode = 0x4;
		}
	} else {
		value = readl(TEGRA_FUSE_BASE + 0x100);
		if (value) {
			info->op_mode = 0x3;
		} else {
			info->op_mode = 0x1;
		}
	}
}

static int usb_irq(unsigned int irq, void *data)
{
	struct usb *usb = data;
	uint32_t value;

	uart_printf(debug, "> %s(irq=%u, data=%p)\n", __func__, irq, data);

	value = usb_readl(usb, USBSTS);
	uart_printf(debug, "  USBSTS: %08x\n", value);
	usb_writel(usb, value, USBSTS);

	uart_printf(debug, "< %s()\n", __func__);

	return IRQ_HANDLED;
}

static void nv3p_send_ack(uint32_t sequence)
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

	usb_ep_send(&ep_bulk_in, ack, sizeof(*ack));
	usb_ep_wait(&ep_bulk_in);
}

static void nv3p_send_status(uint32_t sequence, const char *message,
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

	usb_ep_send(&ep_bulk_in, send, sizeof(*status));
	usb_ep_wait(&ep_bulk_in);
}

static uint32_t payload_length;
static void *payload;

static void nv3p_process(struct usb_ep *ep)
{
	struct nv3p_header *header = (struct nv3p_header *)recv;
	uint32_t sequence = header->sequence;

	uart_printf(debug, "nv3p packet:\n");
	uart_printf(debug, "  version: %08x\n", header->version);
	uart_printf(debug, "  type: %08x\n", header->type);
	uart_printf(debug, "  sequence: %08x\n", header->sequence);

	/* command packet */
	if (header->type == NV3P_PACKET_TYPE_COMMAND) {
		struct nv3p_packet_command *packet = (struct nv3p_packet_command *)recv;

		uart_printf(debug, "  command packet:\n");
		uart_printf(debug, "    length: %08x\n", packet->length);
		uart_printf(debug, "    command: %08x\n", packet->command);
		uart_printf(debug, "    checksum: %08x\n", packet->checksum);

		if (packet->command == NV3P_COMMAND_GET_PLATFORM_INFO) {
			struct nv3p_packet_platform_info *info = (struct nv3p_packet_platform_info *)send;
			struct nv3p_packet_status *status = (struct nv3p_packet_status *)send;
			uint32_t checksum = 0;
			unsigned int i;

			uart_printf(debug, "      NV3P_CMD_GET_PLATFORM_INFO\n");

			nv3p_send_ack(sequence);

			memset(info, 0, sizeof(*info));
			info->header.version = NV3P_VERSION;
			info->header.type = NV3P_PACKET_TYPE_DATA;
			info->header.sequence = sequence;
			info->length = sizeof(struct nv3p_platform_info);

			prepare_platform_info(&info->info);

			checksum = 0;

			for (i = 0; i < sizeof(*info); i++)
				checksum += send[i];

			info->checksum = ~checksum + 1;

			uart_printf(debug, "  data packet:\n");
			uart_printf(debug, "    header:\n");
			uart_printf(debug, "      version: %08x\n", info->header.version);
			uart_printf(debug, "      type: %08x\n", info->header.type);
			uart_printf(debug, "      sequence: %08x\n", info->header.sequence);
			uart_printf(debug, "    length: %08x\n", info->length);
			uart_printf(debug, "    checksum: %08x\n", info->checksum);

			usb_ep_send(&ep_bulk_in, send, sizeof(*info));
			usb_ep_wait(&ep_bulk_in);

			/* queue new receive buffer */
			//usb_ep_flush(&ep_bulk_out);
			usb_ep_recv(&ep_bulk_out, recv, sizeof(recv));
			usb_ep_wait(&ep_bulk_out);

			memset(status, 0, sizeof(*status));
			status->header.version = NV3P_VERSION;
			status->header.type = NV3P_PACKET_TYPE_COMMAND;
			status->header.sequence = sequence;
			status->length = sizeof(struct nv3p_command_status);
			status->command = NV3P_COMMAND_STATUS;
			status->status.message[0] = 'O';
			status->status.message[1] = 'K';
			status->status.message[2] = '\0';
			status->status.code = NV3P_STATUS_OK;
			status->status.flags = 0;

			checksum = 0;

			for (i = 0; i < sizeof(*status); i++)
				checksum += send[i];

			status->checksum = ~checksum + 1;

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

			usb_ep_send(&ep_bulk_in, send, sizeof(*status));
			usb_ep_wait(&ep_bulk_in);

			/* queue new receive buffer */
			usb_ep_recv(&ep_bulk_out, recv, sizeof(recv));
			usb_ep_wait(&ep_bulk_out);
		}

		if (packet->command == NV3P_COMMAND_DOWNLOAD_BCT) {
			struct nv3p_packet_download_bct *command = (struct nv3p_packet_download_bct *)recv;

			uart_printf(debug, "      NV3P_COMMAND_DOWNLOAD_BCT\n");
			uart_printf(debug, "        size: %u\n", command->size);
			uart_printf(debug, "        BCT: %u\n", sizeof(struct bct));

			payload = bct;
			payload_length = command->size;

			nv3p_send_ack(sequence);
		}

		if (packet->command == NV3P_COMMAND_DOWNLOAD_BOOTLOADER) {
			struct nv3p_packet_download_bootloader *command = (struct nv3p_packet_download_bootloader *)recv;

			uart_printf(debug, "      NV3P_COMMAND_DOWNLOAD_BOOTLOADER\n");
			uart_printf(debug, "        length: %u\n", command->length);
			uart_printf(debug, "        size: %u\n", (uint32_t)command->size);
			uart_printf(debug, "        load: %x\n", command->load);
			uart_printf(debug, "        entry: %x\n", command->entry);

			nv3p_send_ack(sequence);
			nv3p_send_status(sequence, "OK", NV3P_STATUS_OK, 0);

			payload = (void *)command->load;
			payload_length = command->size;
			bootloader = (void (*)(void))command->entry;

			if (0) {
				uint32_t value = readl(TEGRA_PMC_BASE + PMC_STRAPPING_OPT_A);
				unsigned int index;

				uart_printf(debug, "    strapping: %08x\n", value);
				uart_printf(debug, "      RAM code: %02x\n", (value >> 4) & 0xf);
				uart_printf(debug, "      SDRAM: %02x\n", (value >> 4) & 0x3);

				index = (value >> 4) & 0x3;

				sdram_init(&bct->sdram_params[index]);

				if (1) {
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
			}
		}
	}

	if (header->type == NV3P_PACKET_TYPE_DATA) {
		struct nv3p_packet_data *data = (struct nv3p_packet_data *)recv;
		unsigned int length, num, received = 0;

		uart_printf(debug, "  data packet:\n");
		uart_printf(debug, "    length: %u\n", data->length);
		length = data->length;

#define BOUNCE_BUFFER

		while (received < length) {
			num = min(2048, sizeof(recv));

#ifdef BOUNCE_BUFFER
			usb_ep_recv(ep, recv, num);
#else
			usb_ep_recv(ep, payload + received, num);
#endif
			usb_ep_wait(ep);

			uart_printf(debug, "  dQH: %p\n", ep->dqh);
			uart_printf(debug, "    current: %08x\n", ep->dqh->current);
			uart_printf(debug, "    info: %08x\n", ep->dqh->info);

			num -= ((ep->dtd->info >> 16) & 0xffff);
			uart_printf(debug, "  received %u bytes\n", num);

			//uart_printf(debug, "  copying %u bytes to %p\n", num, payload + received);
#ifdef BOUNCE_BUFFER
			memcpy(payload + received, recv, num);
#endif
			received += num;
		}

		payload_length -= received;

		if (1 && payload == bct) {
			uint32_t *ptr = (uint32_t *)bct;
			unsigned int i;

			uart_printf(debug, "verifying BCT...\n");

			for (i = 0; i < sizeof(*bct) / 4; i++) {
				if (ptr[i] != i)
					uart_printf(debug, "  corruption @%u: %08x\n", i, ptr[i]);
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
		usb_ep_recv(ep, recv, sizeof(recv));
		usb_ep_wait(ep);

		nv3p_send_ack(sequence);

		if (payload_length == 0)
			nv3p_send_status(header->sequence, "OK", NV3P_STATUS_OK, 0);
	}

	/* queue another receive buffer */
	usb_ep_recv(&ep_bulk_out, recv, sizeof(recv));
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
		value = (0x00 << 8) | (0x0c << 0);
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

	uart_printf(debug, "resetting USB controller\n");

	/* wait for reset */
	while (true) {
		value = usb_readl(usb, USBCMD);
		if ((value & USBCMD_RST) == 0)
			break;
	}

	uart_printf(debug, "done\n");

	/* wait for PHY clock to stabilize */
	while (true) {
		value = usb_readl(usb, USB_SUSP_CTRL);
		if (value & USB_SUSP_CTRL_USB_PHY_CLK_VALID)
			break;
	}

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

	uart_printf(debug, "device mode\n");

	/* this will be reset to 0 during the reset sequence above */
	usb_writel(usb, usb->base + 0x1000, ASYNCLISTADDR);

	usb_ep_setup(&ep_ctrl_out);
	usb_ep_setup(&ep_ctrl_in);

	if (1) {
		value = usb_readl(usb, HOSTPC1_DEVLC);
		value &= ~HOSTPC1_DEVLC_ASUS;
		usb_writel(usb, value, HOSTPC1_DEVLC);
	}

	value = usb_readl(usb, USBINTR);
	value &= ~(USBINTR_SLE | USBINTR_SRE | USBINTR_AAE | USBINTR_SEE |
		   USBINTR_FRE);
	value |= USBINTR_URE | USBINTR_PCE | USBINTR_UEE | USBINTR_UE;
	usb_writel(usb, value, USBINTR);

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

	uart_printf(debug, "done\n");

	if (1) {
		unsigned int i;

		for (i = 0; i < 2; i++) {
			value = usb_readl(usb, ENDPTCTRL(i));
			uart_printf(debug, "ENDPTCTRL(%u): %08x\n", i, value);
		}
	}

	/* loop */
	while (!uart_tstc(debug)) {
		uint32_t status;

		status = usb_readl(usb, USBSTS);
		usb_writel(usb, status, USBSTS);

		if ((status & ~USBSTS_SRI) != 0)
			uart_printf(debug, "USBSTS: %08x\n", status);

		if (status & USBSTS_URI) {
			uart_printf(debug, "usb: reset detected\n");

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

		if (status & USBSTS_PCI) {
			uart_printf(debug, "usb: port change detected\n");
			value = usb_readl(usb, HOSTPC1_DEVLC);
			uart_printf(debug, "  HOSTPC1_DEVLC: %08x\n", value);
		}

		if (status & USBSTS_UEI)
			uart_printf(debug, "usb: error detected\n");

		if (status & USBSTS_UI) {
			uart_printf(debug, "usb: interrupt\n");

			value = usb_readl(usb, ENDPTSETUPSTAT);
			if (value) {
				struct usb_ep *ep = NULL;
				unsigned int i;
				int err;

				uart_printf(debug, "ENDPTSETUPSTAT: %08x\n", value);
				usb_writel(usb, value, ENDPTSETUPSTAT);

				for (i = 0; i < 16; i++) {
					if (value & (1 << i)) {
						uart_printf(debug, "SETUP on endpoint %u\n", i);

						ep = usb_ep_get(i);
						if (ep) {
							/* handle endpoint */
							err = handle_setup(ep);
							if (!err) {
								/* XXX: setting these up earlier will make them not work */
								usb_ep_setup(&ep_bulk_out);
								usb_ep_setup(&ep_bulk_in);

								usb_ep_flush(&ep_bulk_out);
								usb_ep_recv(&ep_bulk_out, recv, sizeof(recv));
							}
						}
					}
				}
			}

			value = usb_readl(usb, ENDPTCOMPLETE);
			if (value) {
				struct usb_ep *ep = NULL;
				unsigned int i;

				/*
				uart_printf(debug, "ENDPTCOMPLETE: %08x\n", value);
				*/
				usb_writel(usb, value, ENDPTCOMPLETE);

				for (i = 0; i < 32; i++) {
					if (value & (1 << i)) {
						unsigned int index = (i % 16) * 2 + i / 16;

						uart_printf(debug, "endpoint %u ready\n", index);

						ep = usb_ep_get(index);
						if (ep && ep->type == BULK && ep->direction == OUT) {
#if 0
							unsigned int length;

							uart_printf(debug, "endpoint %p ready\n", ep);
							uart_printf(debug, "  dqh: %p\n", ep->dqh);
							uart_printf(debug, "    caps: %08x\n", ep->dqh->caps);
							uart_printf(debug, "    curr: %08x\n", ep->dqh->current);
							uart_printf(debug, "    next: %08x\n", ep->dqh->next);
							uart_printf(debug, "    info: %08x\n", ep->dqh->info);
							uart_printf(debug, "    buffers:\n");
							uart_printf(debug, "      [0]: %08x\n", ep->dqh->buffers[0]);
							uart_printf(debug, "      [1]: %08x\n", ep->dqh->buffers[1]);
							uart_printf(debug, "      [2]: %08x\n", ep->dqh->buffers[2]);
							uart_printf(debug, "      [3]: %08x\n", ep->dqh->buffers[3]);
							uart_printf(debug, "      [4]: %08x\n", ep->dqh->buffers[4]);

							length = sizeof(recv) - ((ep->dtd->info >> 16) & 0x7fff);
							uart_printf(debug, "  length: %u\n", length);
							uart_hexdump(debug, recv, length, 16, true);
#endif

							nv3p_process(ep);
						}
					}
				}
			}
		}
	}

	uart_printf(debug, "BIT: %p\n", bit);
	uart_printf(debug, "  BootROM version: %08x\n", bit->boot_rom_version);
	uart_printf(debug, "  Data version: %08x\n", bit->data_version);
	uart_printf(debug, "  RCM version: %08x\n", bit->rcm_version);
	uart_printf(debug, "  Boot type: %08x\n", bit->boot_type);
	uart_printf(debug, "  Primary device: %08x\n", bit->primary_device);
	uart_printf(debug, "  ...\n");
	uart_printf(debug, "  Device initialized: %02x\n", bit->device_initialized);
	uart_printf(debug, "  SDRAM initialized: %02x\n", bit->sdram_initialized);
	uart_printf(debug, "  ...\n");
	uart_printf(debug, "  BCT valid: %02x\n", bit->bct_valid);
	uart_printf(debug, "  ...\n");
	uart_printf(debug, "  Safe start address: %08x\n", bit->safe_start_address);

	uart_printf(debug, "executing bootloader at %p\n", bootloader);

	if (1) {
		uint32_t *ptr = (uint32_t *)bootloader;
		unsigned int i;

		for (i = 0; i < 16; i++)
			uart_printf(debug, "  %p: %08x\n", &ptr[i], ptr[i]);
	}

	//bootloader();

	uart_printf(debug, "< %s()\n", __func__);
}
