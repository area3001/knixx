#ifndef USB_H_
#define USB_H_

#define USB_EP_IN	0x80
#define USB_INT_MAX_PACKET_SIZE		16
#define USB_BULK_MAX_PACKET_SIZE 	64

void usb_setup(void);
void usb_poll(void);

#endif
