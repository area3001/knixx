/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Bart Van Der Meerssche <bart@flukso.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include "usb.h"
#include "cli.h"


// Interface numbers
enum {
    USB_CDC_CIF_NUM0,
    USB_CDC_DIF_NUM0,
    USB_CDC_CIF_NUM1,
    USB_CDC_DIF_NUM1,
    USB_NUM_INTERFACES        // number of interfaces
};

usbd_device *dev;

static const struct usb_device_descriptor descr = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0xEF, 		// changed to Misc Device Class
	.bDeviceSubClass = 0x02,	// changed to Common
	.bDeviceProtocol = 0x01,	// changed to IAD protocol
	.bMaxPacketSize0 = 64,		// Max packet size
	.idVendor = 0x1209,		// InterBiometrics
	.idProduct = 0xa7ea,		// Our own
	.bcdDevice = 0x0200,		// Contains device release number
	.iManufacturer = 1, 		/* Index of string descriptor describing manufacturer */
	.iProduct = 2, 			/* Index of string descriptor describing product */
	.iSerialNumber = 3,		/* Index of string descriptor describing the device's serial number */
	.bNumConfigurations = 1,
};

/*
 * This notification endpoint isn't implemented. According to CDC spec its
 * optional, but its absence causes a NULL pointer dereference in Linux
 * cdc_acm driver.
 */
static const struct usb_endpoint_descriptor comm_endp1[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x82,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = USB_INT_MAX_PACKET_SIZE,
	.bInterval = 128,
}};

static const struct usb_endpoint_descriptor comm_endp2[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x84,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = USB_INT_MAX_PACKET_SIZE,
	.bInterval = 128,
}};

static const struct usb_endpoint_descriptor data_endp1[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = USB_BULK_MAX_PACKET_SIZE,
	.bInterval = 0,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x81,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = USB_BULK_MAX_PACKET_SIZE,
	.bInterval = 0,
}};

static const struct usb_endpoint_descriptor data_endp2[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x03,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = USB_BULK_MAX_PACKET_SIZE,
	.bInterval = 0,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x83,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = USB_BULK_MAX_PACKET_SIZE,
	.bInterval = 0,
}};

static const struct {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) cdcacm_functional_descriptors1 = {
	.header = {
		.bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_HEADER,
		.bcdCDC = 0x0110,
	},
	.call_mgmt = {
		.bFunctionLength =
			sizeof(struct usb_cdc_call_management_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
		.bmCapabilities = 0,
		.bDataInterface = 1, // check this one!
	},
	.acm = {
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_ACM,
		.bmCapabilities = 0, // check shouldn't this be 0x02?
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 0,
		.bSubordinateInterface0 = 1,
	 },
};

static const struct {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) cdcacm_functional_descriptors2 = {
	.header = {
		.bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_HEADER,
		.bcdCDC = 0x0110,
	},
	.call_mgmt = {
		.bFunctionLength =
			sizeof(struct usb_cdc_call_management_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
		.bmCapabilities = 0,
		.bDataInterface = 3,
	},
	.acm = {
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_ACM,
		.bmCapabilities = 0, //check shouldn't this be 0x02?
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 2,
		.bSubordinateInterface0 = 3,
	 },
};

static const struct usb_interface_descriptor comm_iface1[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = USB_CDC_CIF_NUM0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface = 0,

	.endpoint = comm_endp1,
	.extra = &cdcacm_functional_descriptors1,
	.extralen = sizeof(cdcacm_functional_descriptors1),
}};

static const struct usb_interface_descriptor data_iface1[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = USB_CDC_DIF_NUM0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = data_endp1,
}};

static const struct usb_interface_descriptor comm_iface2[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = USB_CDC_CIF_NUM1,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface = 0,

	.endpoint = comm_endp2,
	.extra = &cdcacm_functional_descriptors2,
	.extralen = sizeof(cdcacm_functional_descriptors2),
}};

static const struct usb_interface_descriptor data_iface2[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = USB_CDC_DIF_NUM1,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = data_endp2,
}};

static const struct usb_iface_assoc_descriptor iad_iface[] = {{
	.bLength = 0x08,		// 8 bytes in this struct
	.bDescriptorType = 0x08, 	// IAD
	.bFirstInterface = 0x00,
	.bInterfaceCount = 0x02,
	.bFunctionClass = 0x02, 	// Function class CDC
	.bFunctionSubClass = 0x02,
	.bFunctionProtocol = 0x01,
	.iFunction = 0x02,
}};

static const struct usb_interface ifaces[] = {
{
	.num_altsetting = 1,
	.altsetting = iad_iface,	
},
{
	.num_altsetting = 1,
	.altsetting = comm_iface1,
}, 
{
	.num_altsetting = 1,
	.altsetting = data_iface1,
},
{
	.num_altsetting = 1,
	.altsetting = comm_iface2,
}, 
{
	.num_altsetting = 1,
	.altsetting = data_iface2,
}
};

//const uint8_t Virtual_Com_Port_ConfigDescriptor[] =
static const struct usb_config_descriptor config = {
/* Configuration 1 */
  .bLength 		= USB_DT_CONFIGURATION_SIZE,       /* bLength */
  .bDescriptorType 	= USB_DT_CONFIGURATION, /* bDescriptorType */
  .wTotalLength = WBVAL(                             /* wTotalLength */
      USB_CONFIGUARTION_DESC_SIZE
    + 2 * IAD_CDC_IF_DESC_SET_SIZE
  ), // TODO: This definitly needs to be checked!
  .bNumInterfaces 	= USB_NUM_INTERFACES,                /* bNumInterfaces */
  .bConfigurationValue 	= 0x01,                              /* bConfigurationValue: 0x01 is used to select this configuration */
  .iConfiguration 	= 0x00,                              /* iConfiguration: no string to describe this configuration */
  .bmAttributes 	= USB_CONFIG_BUS_POWERED, /*|*/       /* bmAttributes */
  .bMaxPower 		= USB_CONFIG_POWER_MA(100),          /* bMaxPower, device power consumption is 100 mA */

<<<<<<< HEAD
=======

//TODO: PROBLEM IS THAT WE NEED TO INCLUDE THE IAD HERE !!!
/*

Found this struct: http://libopencm3.github.io/docs/latest/usb/html/structusb__iface__assoc__descriptor.html

interface point should contain the IAD as well and as a first

    // IAD
    0x08,	// bLength: Interface Descriptor size
    0x0B,	// bDescriptorType: IAD
    0x00,	// bFirstInterface
    0x02,	// bInterfaceCount
    0x02,	// bFunctionClass: CDC
    0x02,	// bFunctionSubClass
    0x01,	// bFunctionProtocol 
    0x02, // iFunction
*/

>>>>>>> 73bb1f1c84ed87c1d722f76a244652cb5afdf9b5
  .interface = ifaces,
/*  .interface = IAD_CDC_IF_DESC_SET( USB_CDC_CIF_NUM0, USB_CDC_DIF_NUM0, USB_ENDPOINT_IN(1), USB_ENDPOINT_OUT(2), USB_ENDPOINT_IN(2) ),
  IAD_CDC_IF_DESC_SET( USB_CDC_CIF_NUM1, USB_CDC_DIF_NUM1, USB_ENDPOINT_IN(3), USB_ENDPOINT_OUT(4), USB_ENDPOINT_IN(4) )
*/
};

static const char *usb_strings[3] = {
	"Knixx",
	"SW04A",
	"KX01000001",
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

static int cdcacm_control_request(usbd_device *usbd_dev,
	struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
	void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
	(void)complete;
	(void)buf;
	(void)usbd_dev;

	switch (req->bRequest) {
	case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
		/*
		 * This Linux cdc_acm driver requires this to be implemented
		 * even though it's optional in the CDC spec, and we don't
		 * advertise it in the ACM functional descriptor.
		 */
		char local_buf[10];
		struct usb_cdc_notification *notif = (void *)local_buf;

		/* We echo signals back to host as notification. */
		notif->bmRequestType = 0xa1;
		notif->bNotification = USB_CDC_NOTIFY_SERIAL_STATE;
		notif->wValue = 0;
		notif->wIndex = 0;
		notif->wLength = 2;
		local_buf[8] = req->wValue & 3;
		local_buf[9] = 0;
		// usbd_ep_write_packet(0x82, buf, 10);
		return 1;
		}
	case USB_CDC_REQ_SET_LINE_CODING:
		if (*len < sizeof(struct usb_cdc_line_coding))
			return 0;
		return 1;
	}
	return 0;
}

static void cdcacm_data_rx_cb1(usbd_device *usbd_dev, uint8_t ep)
{
	int i, len;
	char buf[USB_BULK_MAX_PACKET_SIZE];

	len = usbd_ep_read_packet(usbd_dev, ep, buf, sizeof(buf));
	if (len) {
		for (i = 0; i < len; i++) {
			cli_insert_char(buf[i]);
		}
	}
}

static void cdcacm_data_rx_cb2(usbd_device *usbd_dev, uint8_t ep)
{
	int i, len;
	char buf[USB_BULK_MAX_PACKET_SIZE];

	len = usbd_ep_read_packet(usbd_dev, ep, buf, sizeof(buf));
	if (len) {
		for (i = 0; i < len; i++) {
			cli_insert_char(buf[i]);
		}
	}
}

static void cdcacm_set_config(usbd_device *usbd_dev, uint16_t wValue)
{
	(void)wValue;

	usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, USB_BULK_MAX_PACKET_SIZE, cdcacm_data_rx_cb1);
	usbd_ep_setup(usbd_dev, 0x81, USB_ENDPOINT_ATTR_BULK, USB_BULK_MAX_PACKET_SIZE, NULL);
	usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_INTERRUPT, USB_INT_MAX_PACKET_SIZE, NULL);

	usbd_ep_setup(usbd_dev, 0x03, USB_ENDPOINT_ATTR_BULK, USB_BULK_MAX_PACKET_SIZE, cdcacm_data_rx_cb2);
	usbd_ep_setup(usbd_dev, 0x83, USB_ENDPOINT_ATTR_BULK, USB_BULK_MAX_PACKET_SIZE, NULL);
	usbd_ep_setup(usbd_dev, 0x84, USB_ENDPOINT_ATTR_INTERRUPT, USB_INT_MAX_PACKET_SIZE, NULL);

	usbd_register_control_callback(usbd_dev,
		USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
		USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
		cdcacm_control_request);
}

void usb_setup(void)
{
	rcc_periph_clock_enable(RCC_USB);
	dev = usbd_init(&st_usbfs_v2_usb_driver, &descr, &config, usb_strings, 3,
		usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(dev, cdcacm_set_config);
}

void usb_poll(void)
{
	usbd_poll(dev);
}

void usb_print(const char *str)
{
	size_t i, len;

	len = strlen(str);
	for (i = 0; i < len / USB_BULK_MAX_PACKET_SIZE; i++) {
		while (!usbd_ep_write_packet(dev, 0x81,
			str + i * USB_BULK_MAX_PACKET_SIZE, USB_BULK_MAX_PACKET_SIZE));
	}
	while (!usbd_ep_write_packet(dev, 0x81,
			str + i * USB_BULK_MAX_PACKET_SIZE,
			len % USB_BULK_MAX_PACKET_SIZE));
}
