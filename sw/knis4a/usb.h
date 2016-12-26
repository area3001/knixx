#ifndef USB_H_
#define USB_H_

#define USB_INT_MAX_PACKET_SIZE		16
#define USB_BULK_MAX_PACKET_SIZE 	64

#define EP_DATA_OUT(x)	x
#define EP_DATA_IN(x)	(x | 0x80)
#define EP_COMM(x)		((x + 1) | 0x80)

enum usb_itf {
	USB_ITF_CONSOLE	= 0x01,
	USB_ITF_DEBUG = 0x03,
	USB_ITF_TPUART = 0x05
};

void usb_setup(void);
void usb_print_console(const char *str);
void usb_print_debug(const char *str);
void usb_print_tpuart(const char *str);

#endif
