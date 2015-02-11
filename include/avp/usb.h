#ifndef AVP_USB_H
#define AVP_USB_H

struct usb {
	unsigned long base;
};

void usb_init(struct usb *usb);

extern struct usb usbd;

#endif
