#include <string.h>

#include <avp/io.h>
#include <avp/iomap.h>
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

struct usb_setup {
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
};

struct usb usbd = {
	.base = TEGRA_USBD_BASE,
};

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

	unsigned int qh;
};

struct usb_dqh {
	uint32_t capabilities;
	uint32_t current;
	uint32_t next;
	uint32_t token;
	uint32_t buffers[5];
	uint32_t reserved0;
	uint32_t setup[2];
	uint32_t reserved1;
	uint32_t reserved2;
	uint32_t reserved3;
	uint32_t reserved4;
};

#define DQH_CAPS_ZLT (1 << 29)
#define DQH_CAPS_IOC (1 << 15)
#define DQH_TERMINATE (1 << 0)

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

void usb_dqh_dump(struct uart *uart, struct usb_dqh *dqh)
{
	uart_printf(uart, "capabilities: %08x\n", dqh->capabilities);
	uart_printf(uart, "current: %08x\n", dqh->current);
	uart_printf(uart, "next: %08x\n", dqh->next);
	uart_printf(uart, "token: %08x\n", dqh->token);
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

struct usb_ep ep_ctrl_out = {
	.usb = &usbd,
	.index = 0,
	.direction = OUT,
	.type = CTRL,
	.qh = 0x1000,
};

struct usb_ep ep_ctrl_in = {
	.usb = &usbd,
	.index = 0,
	.direction = IN,
	.type = CTRL,
	.qh = 0x1040,
};

struct usb_ep ep_bulk_out = {
	.usb = &usbd,
	.index = 1,
	.direction = OUT,
	.type = BULK,
	.qh = 0x1080,
};

struct usb_ep ep_bulk_in = {
	.usb = &usbd,
	.index = 1,
	.direction = IN,
	.type = BULK,
	.qh = 0x10c0,
};

static uint32_t usb_ep_mask(struct usb_ep *ep)
{
	uint32_t value = (ep->direction == IN) ? 0x00010000 : 0x00000001;

	return value << ep->index;
}

#if 0
static void usb_ep_flush(struct usb_ep *ep)
{
	uint32_t mask = usb_ep_mask(ep);

	uart_printf(debug, "mask: %08x\n", mask);
	usb_writel(ep->usb, mask, ENDPTFLUSH);

	usb_writel(ep->usb, mask, ENDPTCOMPLETE);
}
#endif

static void usb_ep_prime(struct usb_ep *ep)
{
	uint32_t mask = usb_ep_mask(ep);

	uart_printf(debug, "priming: %08x\n", mask);
	usb_writel(ep->usb, mask, ENDPTPRIME);
}

struct usb_dtd {
	uint32_t next;
	uint32_t token;
	uint32_t buffers[5];
	uint32_t reserved;
};

#define DTD_NEXT_TERMINATE (1 << 0)
#define DTD_TOKEN_ACTIVE (1 << 7)

#define ALIGN(align) __attribute__((aligned(align)))

static struct usb_dtd dtd __attribute__((aligned(4096)));
static uint8_t buf[4096] __attribute__((aligned(4096)));

#define USB_LANGUAGE_ID 0x00
#define USB_MANUFACTURER_ID 0x01
#define USB_PRODUCT_ID 0x02
#define USB_SERIAL_ID 0x03

struct usb_device_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
} __attribute__((packed));

struct usb_configuration_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t bMaxPower;
} __attribute__((packed));

struct usb_interface_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
} __attribute__((packed));

struct usb_endpoint_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;
} __attribute__((packed));

#define USB_SETUP_REQUEST_TYPE_HOST_TO_DEVICE 0x00
#define USB_SETUP_REQUEST_TYPE_DEVICE_TO_HOST 0x80
#define USB_SETUP_REQUEST_SET_ADDRESS 0x05
#define USB_SETUP_REQUEST_GET_DESCRIPTOR 0x06
#define USB_SETUP_REQUEST_SET_CONFIGURATION 0x09

#define USB_DESCRIPTOR_TYPE_DEVICE 0x01
#define USB_DESCRIPTOR_TYPE_CONFIGURATION 0x02
#define USB_DESCRIPTOR_TYPE_STRING 0x03
#define USB_DESCRIPTOR_TYPE_INTERFACE 0x04
#define USB_DESCRIPTOR_TYPE_ENDPOINT 0x05

static const uint8_t language_ids[] ALIGN(32) = {
	4,
	USB_DESCRIPTOR_TYPE_STRING,
	0x09, 0x04
};

static const uint8_t manufacturer[] ALIGN(32) = {
	14,
	USB_DESCRIPTOR_TYPE_STRING,
	'N', 0,
	'V', 0,
	'I', 0,
	'D', 0,
	'I', 0,
	'A', 0,
};

static const uint8_t product[] ALIGN(32) = {
	8,
	USB_DESCRIPTOR_TYPE_STRING,
	'A', 0,
	'P', 0,
	'X', 0,
};

static const uint8_t serial[] ALIGN(32) = {
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

static struct usb_device_descriptor device_descriptor ALIGN(32) = {
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
	struct usb_endpoint_descriptor ep1_in;
	struct usb_endpoint_descriptor ep1_out;
} __attribute__((packed));

static struct usb_configuration configuration ALIGN(32) = {
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
	.ep1_in = {
		.bLength = sizeof(struct usb_endpoint_descriptor),
		.bDescriptorType = USB_DESCRIPTOR_TYPE_ENDPOINT,
		.bEndpointAddress = 0x81,
		.bmAttributes = 0x02,
		.wMaxPacketSize = 0x0040,
		.bInterval = 0x00,
	},
	.ep1_out = {
		.bLength = sizeof(struct usb_endpoint_descriptor),
		.bDescriptorType = USB_DESCRIPTOR_TYPE_ENDPOINT,
		.bEndpointAddress = 0x01,
		.bmAttributes = 0x02,
		.wMaxPacketSize = 0x0040,
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
	struct usb_dqh *dqh = (struct usb_dqh *)(ep->usb->base + ep->qh);
	uint32_t value;

	uart_printf(debug, "> %s(ep=%p, buffer=%p, size=%zu)\n", __func__, ep,
		    buffer, size);

	dqh->capabilities = DQH_CAPS_ZLT | (64 << 16) | DQH_CAPS_IOC;
	dqh->next = ((uint32_t)&dtd) & 0xffffff80;

	dtd.next = DTD_NEXT_TERMINATE;
	dtd.token = (size << 16) | DTD_TOKEN_ACTIVE;
	dtd.buffers[0] = (uint32_t)buffer;

	value = usb_readl(ep->usb, ENDPTPRIME);
	value |= usb_ep_mask(ep);
	usb_writel(ep->usb, value, ENDPTPRIME);

	uart_printf(debug, "< %s()\n", __func__);
}

void usb_init(struct usb *usb)
{
	uint32_t value;

	uart_printf(debug, "> %s(usb=%p)\n", __func__, usb);
	uart_printf(debug, "  base: %#lx\n", usb->base);

	value = usb_readl(usb, ASYNCLISTADDR);
	uart_printf(debug, "ASYNCLISTADDR: %08x\n", value);

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

	if (0) {
		//unsigned int offset;
		struct usb_dqh dqh;

		usb_ep_dqh_read(&ep_bulk_out, &dqh);
		usb_dqh_dump(debug, &dqh);

		memset(&dqh, 0, sizeof(dqh));

		/* 512 bytes for high-speed mode (XXX parameterize) */
		dqh.capabilities |= 1 << 29 | 512 << 16;
		dqh.next = (uint32_t)&dtd & 0xffffff80;

		uart_printf(debug, "dtd: %p\n", &dtd);
		memset(&dtd, 0, sizeof(dtd));
		dtd.next = 1 << 0; /* terminate */
		dtd.token = sizeof(buf) << 16 | 1 << 7;
		dtd.buffers[0] = (uint32_t)buf;

		uart_printf(debug, "  payload: %p\n", buf);

		usb_dqh_dump(debug, &dqh);
		usb_ep_dqh_write(&ep_bulk_out, &dqh);

		usb_ep_prime(&ep_bulk_out);

		/*
		offset = 0x21c + ep_bulk_out.index * 4;
		value = usb_readl(usb, offset);
		uart_printf(debug, "EP status: %08x > %08x\n", offset, value);

		usb_ep_dqh_read(&ep_bulk_out, &dqh);
		uart_printf(debug, "  token: %08x\n", dqh.token);

		value = usb_readl(usb, ENDPTSTATUS);
		uart_printf(debug, "  ENDPTSTATUS: %08x\n", value);
		*/
	}

	if (1) {
		struct usb_dqh *dqh;

		dqh = (struct usb_dqh *)(usb->base + ep_ctrl_out.qh);

		memset(dqh, 0, sizeof(*dqh));
		dqh->capabilities = (64 << 16) | DQH_CAPS_IOC;
		dqh->next = DQH_TERMINATE;

		value = usb_readl(usb, ENDPTCTRL(ep_ctrl_out.index));

		value &= ~ENDPTCTRL_RXT_MASK;
		value &= ~ENDPTCTRL_RXS;

		value |= ENDPTCTRL_RXT_CTRL;
		value |= ENDPTCTRL_RXE;

		usb_writel(usb, value, ENDPTCTRL(ep_ctrl_out.index));

		dqh = (struct usb_dqh *)(usb->base + ep_ctrl_in.qh);

		memset(dqh, 0, sizeof(*dqh));
		dqh->capabilities = (64 << 16);
		dqh->next = DQH_TERMINATE;

		value = usb_readl(usb, ENDPTCTRL(ep_ctrl_in.index));

		value &= ~ENDPTCTRL_TXT_MASK;
		value &= ~ENDPTCTRL_TXS;

		value |= ENDPTCTRL_TXT_CTRL;
		value |= ENDPTCTRL_TXE;

		usb_writel(usb, value, ENDPTCTRL(ep_ctrl_in.index));
	}

	if (1) {
		struct usb_dqh *dqh;

		dqh = (struct usb_dqh *)(usb->base + ep_bulk_out.qh);

		memset(dqh, 0, sizeof(*dqh));
		dqh->capabilities = (64 << 16) | DQH_CAPS_IOC;
		dqh->next = DQH_TERMINATE;

		value = usb_readl(usb, ENDPTCTRL(ep_bulk_out.index));
		value &= ~ENDPTCTRL_RXT_MASK;
		value &= ~ENDPTCTRL_RXS;
		value |= ENDPTCTRL_RXT_BULK;
		value |= ENDPTCTRL_RXE;
		usb_writel(usb, value, ENDPTCTRL(ep_bulk_out.index));

		dqh = (struct usb_dqh *)(usb->base + ep_bulk_in.qh);

		memset(dqh, 0, sizeof(*dqh));
		dqh->capabilities = (64 << 16) | DQH_CAPS_IOC;
		dqh->next = DQH_TERMINATE;

		value = usb_readl(usb, ENDPTCTRL(ep_bulk_in.index));
		value &= ~ENDPTCTRL_TXT_MASK;
		value &= ~ENDPTCTRL_TXS;
		value |= ENDPTCTRL_TXT_BULK;
		value |= ENDPTCTRL_TXE;
		usb_writel(usb, value, ENDPTCTRL(ep_bulk_in.index));
	}

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
				struct usb_setup *setup;
				struct usb_dqh *dqh;

				uart_printf(debug, "ENDPTSETUPSTAT: %08x\n", value);
				usb_writel(usb, value, ENDPTSETUPSTAT);

				dqh = (struct usb_dqh *)(usb->base + ep_ctrl_out.qh);
				setup = (struct usb_setup *)dqh->setup;

				uart_printf(debug, "setup: %p (dQH: %p)\n", setup, dqh);
				uart_printf(debug, "  bmRequestType: %02x\n", setup->bmRequestType);
				uart_printf(debug, "  bRequest: %02x\n", setup->bRequest);
				uart_printf(debug, "  wValue: %04x\n", setup->wValue);
				uart_printf(debug, "  wIndex: %04x\n", setup->wIndex);
				uart_printf(debug, "  wLength: %04x\n", setup->wLength);

				if (setup->bmRequestType == USB_SETUP_REQUEST_TYPE_DEVICE_TO_HOST) {
					if (setup->bRequest == USB_SETUP_REQUEST_GET_DESCRIPTOR) {
						uint8_t type = setup->wValue >> 8;
						uint8_t index = setup->wValue & 0xff;

						uart_printf(debug, "GET_DESCRIPTOR: %02x %02x\n", type, index);

						if (type == USB_DESCRIPTOR_TYPE_DEVICE) {
							uart_printf(debug, "  device descriptor: %u\n", setup->wLength);

							usb_ep_send(&ep_ctrl_in, &device_descriptor, sizeof(device_descriptor));
							usb_ep_send(&ep_ctrl_out, NULL, 0);
						}

						if (type == USB_DESCRIPTOR_TYPE_CONFIGURATION) {
							uart_printf(debug, "  configuration descriptor: %u\n", setup->wLength);

							if (setup->wLength == sizeof(configuration.configuration))
								usb_ep_send(&ep_ctrl_in, &configuration.configuration, sizeof(configuration.configuration));
							else
								usb_ep_send(&ep_ctrl_in, &configuration, sizeof(configuration));

							usb_ep_send(&ep_ctrl_out, NULL, 0);
						}

						if (type == USB_DESCRIPTOR_TYPE_STRING) {
							const void *data = NULL;
							size_t size = 0;

							uart_printf(debug, "  string descriptor: %u\n", setup->wLength);

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
						}
					}
				}

				if (setup->bmRequestType == USB_SETUP_REQUEST_TYPE_HOST_TO_DEVICE) {
					if (setup->bRequest == USB_SETUP_REQUEST_SET_ADDRESS) {
						uart_printf(debug, "SET_ADDRESS: %04x\n", setup->wValue);

						usb_ep_send(&ep_ctrl_in, NULL, 0);

						usb_writel(usb, setup->wValue << 25, PERIODICLISTBASE);
					}

					if (setup->bRequest == USB_SETUP_REQUEST_SET_CONFIGURATION) {
						uart_printf(debug, "SET_CONFIGURATION: %04x\n", setup->wValue);

						usb_ep_send(&ep_ctrl_in, NULL, 0);
					}
				}
			}
		}

		//udelay(1000000);
	}

	uart_printf(debug, "< %s()\n", __func__);
}
