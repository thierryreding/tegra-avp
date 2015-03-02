#ifndef AVP_USB_H
#define AVP_USB_H

#if 0
struct usb {
	unsigned long base;
};
#else
struct usb;
#endif

void usb_init(struct usb *usb);
int usb_enumerate(struct usb *usb);

ssize_t usb_bulk_recv(struct usb *usb, void *buffer, size_t size);
ssize_t usb_bulk_send(struct usb *usb, const void *buffer, size_t size);

extern struct usb usbd;

#endif
