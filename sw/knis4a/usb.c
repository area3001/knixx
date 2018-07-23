/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Bart Van Der Meerssche <bart@flukso.net>
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
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/usb/dfu.h>
#include "usb.h"
#include "cli.h"

static usbd_device *dev;
static bool usb_connected = false;

static const struct usb_device_descriptor descr = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0xef,   /* miscellaneous device */
	.bDeviceSubClass = 2,   /* common class */
	.bDeviceProtocol = 1,   /* interface association */
	.bMaxPacketSize0 = 64,
	.idVendor = 0x1209,
	.idProduct = 0xa7ea,
	.bcdDevice = 0x0100,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

/*
 * This notification endpoint isn't implemented. According to CDC spec its
 * optional, but its absence causes a NULL pointer dereference in Linux
 * cdc_acm driver.
 */
static const struct usb_endpoint_descriptor console_comm_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = EP_COMM(USB_ITF_CONSOLE),
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = USB_INT_MAX_PACKET_SIZE,
	.bInterval = 128,
}};

static const struct usb_endpoint_descriptor console_data_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = EP_DATA_OUT(USB_ITF_CONSOLE),
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = USB_BULK_MAX_PACKET_SIZE,
	.bInterval = 0,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = EP_DATA_IN(USB_ITF_CONSOLE),
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = USB_BULK_MAX_PACKET_SIZE,
	.bInterval = 0,
}};

static const struct {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) console_cdcacm_functional_descriptors = {
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
		.bDataInterface = 1,
	},
	.acm = {
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_ACM,
		.bmCapabilities = 0,
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 0,
		.bSubordinateInterface0 = 1,
	},
};

static const struct usb_interface_descriptor console_comm_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface = 4,

	.endpoint = console_comm_endp,
	.extra = &console_cdcacm_functional_descriptors,
	.extralen = sizeof(console_cdcacm_functional_descriptors),
}};

static const struct usb_interface_descriptor console_data_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 1,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = console_data_endp,
}};

static const struct usb_iface_assoc_descriptor console_assoc = {
	.bLength = USB_DT_INTERFACE_ASSOCIATION_SIZE,
	.bDescriptorType = USB_DT_INTERFACE_ASSOCIATION,
	.bFirstInterface = 0,
	.bInterfaceCount = 2,
	.bFunctionClass = USB_CLASS_CDC,
	.bFunctionSubClass = USB_CDC_SUBCLASS_ACM,
	.bFunctionProtocol = USB_CDC_PROTOCOL_AT,
	.iFunction = 0,
};

static const struct usb_endpoint_descriptor log_comm_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = EP_COMM(USB_ITF_LOG),
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = USB_INT_MAX_PACKET_SIZE,
	.bInterval = 128,
}};

static const struct usb_endpoint_descriptor log_data_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = EP_DATA_OUT(USB_ITF_LOG),
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = USB_BULK_MAX_PACKET_SIZE,
	.bInterval = 0,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = EP_DATA_IN(USB_ITF_LOG),
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = USB_BULK_MAX_PACKET_SIZE,
	.bInterval = 0,
}};

static const struct {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) log_cdcacm_functional_descriptors = {
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
		.bmCapabilities = 0,
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 2,
		.bSubordinateInterface0 = 3,
	},
};

static const struct usb_interface_descriptor log_comm_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 2,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface = 5,

	.endpoint = log_comm_endp,
	.extra = &log_cdcacm_functional_descriptors,
	.extralen = sizeof(log_cdcacm_functional_descriptors),
}};

static const struct usb_interface_descriptor log_data_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 3,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = log_data_endp,
}};

static const struct usb_iface_assoc_descriptor log_assoc = {
	.bLength = USB_DT_INTERFACE_ASSOCIATION_SIZE,
	.bDescriptorType = USB_DT_INTERFACE_ASSOCIATION,
	.bFirstInterface = 2,
	.bInterfaceCount = 2,
	.bFunctionClass = USB_CLASS_CDC,
	.bFunctionSubClass = USB_CDC_SUBCLASS_ACM,
	.bFunctionProtocol = USB_CDC_PROTOCOL_AT,
	.iFunction = 0,
};

static const struct usb_endpoint_descriptor tpuart_comm_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = EP_COMM(USB_ITF_TPUART),
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = USB_INT_MAX_PACKET_SIZE,
	.bInterval = 128,
}};

static const struct usb_endpoint_descriptor tpuart_data_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = EP_DATA_OUT(USB_ITF_TPUART),
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = USB_BULK_MAX_PACKET_SIZE,
	.bInterval = 0,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = EP_DATA_IN(USB_ITF_TPUART),
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = USB_BULK_MAX_PACKET_SIZE,
	.bInterval = 0,
}};

static const struct {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) tpuart_cdcacm_functional_descriptors = {
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
		.bDataInterface = 5,
	},
	.acm = {
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_ACM,
		.bmCapabilities = 0,
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 4,
		.bSubordinateInterface0 = 5,
	},
};

static const struct usb_interface_descriptor tpuart_comm_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 4,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface = 6,

	.endpoint = tpuart_comm_endp,
	.extra = &tpuart_cdcacm_functional_descriptors,
	.extralen = sizeof(tpuart_cdcacm_functional_descriptors),
}};

static const struct usb_interface_descriptor tpuart_data_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 5,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = tpuart_data_endp,
}};

static const struct usb_iface_assoc_descriptor tpuart_assoc = {
	.bLength = USB_DT_INTERFACE_ASSOCIATION_SIZE,
	.bDescriptorType = USB_DT_INTERFACE_ASSOCIATION,
	.bFirstInterface = 4,
	.bInterfaceCount = 2,
	.bFunctionClass = USB_CLASS_CDC,
	.bFunctionSubClass = USB_CDC_SUBCLASS_ACM,
	.bFunctionProtocol = USB_CDC_PROTOCOL_AT,
	.iFunction = 0,
};

const struct usb_dfu_descriptor dfu_function = {
	.bLength = sizeof(struct usb_dfu_descriptor),
	.bDescriptorType = DFU_FUNCTIONAL,
	.bmAttributes = USB_DFU_CAN_DOWNLOAD | USB_DFU_WILL_DETACH,
	.wDetachTimeout = 255,
	.wTransferSize = 1024,
	.bcdDFUVersion = 0x011a,
};

const struct usb_interface_descriptor dfu_iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 6,
	.bAlternateSetting = 0,
	.bNumEndpoints = 0,
	.bInterfaceClass = 0xfe,
	.bInterfaceSubClass = 1,
	.bInterfaceProtocol = 1,
	.iInterface = 7,

	.extra = &dfu_function,
	.extralen = sizeof(dfu_function),
};

static const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.iface_assoc = &console_assoc,
	.altsetting = console_comm_iface,
}, {
	.num_altsetting = 1,
	.altsetting = console_data_iface,
}, {
	.num_altsetting = 1,
	.iface_assoc = &log_assoc,
	.altsetting = log_comm_iface,
}, {
	.num_altsetting = 1,
	.altsetting = log_data_iface,
}, {
	.num_altsetting = 1,
	.iface_assoc = &tpuart_assoc,
	.altsetting = tpuart_comm_iface,
}, {
	.num_altsetting = 1,
	.altsetting = tpuart_data_iface,
}, {
	.num_altsetting = 1,
	.altsetting = &dfu_iface,
}};

static const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 7,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,
	.bMaxPower = 0x32,

	.interface = ifaces,
};

static const char *usb_strings[7] = {
	"Knixx",
	"KNIS4A",
	"KX01000001",
	"knixx/console",
	"knixx/log",
	"knixx/tpuart",
	"DfuSe-specific"
};

static void cdcacm_set_modem_state(usbd_device *usbd_dev, int iface, bool dsr,
	bool dcd)
{
	char buf[10];
	struct usb_cdc_notification *notif = (void*)buf;
	/* We echo signals back to host as notification */
	notif->bmRequestType = 0xA1;
	notif->bNotification = USB_CDC_NOTIFY_SERIAL_STATE;
	notif->wValue = 0;
	notif->wIndex = iface;
	notif->wLength = 2;
	buf[8] = (dsr ? 2 : 0) | (dcd ? 1 : 0);
	buf[9] = 0;
	usbd_ep_write_packet(usbd_dev, EP_COMM(iface), buf, 10);
}

static void dfu_detach_complete(usbd_device *usbd_dev, struct usb_setup_data *req)
{
	(void)req;
	(void)usbd_dev;

	/* TODO jump into bootloader */
}

static int cdcacm_control_request(usbd_device *usbd_dev,
	struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
	void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
	(void)complete;
	(void)buf;
	(void)len;

	switch (req->bRequest) {
	case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
		cdcacm_set_modem_state(usbd_dev, req->wIndex, true, true);
		return 1;
		break;
	case USB_CDC_REQ_SET_LINE_CODING:
		if (*len < sizeof(struct usb_cdc_line_coding))
			return 0;
		return 1;
		break;
	case DFU_GETSTATUS:
		if(req->wIndex == USB_ITF_DFU) {
			(*buf)[0] = DFU_STATUS_OK;
			(*buf)[1] = 0;
			(*buf)[2] = 0;
			(*buf)[3] = 0;
			(*buf)[4] = STATE_APP_IDLE;
			(*buf)[5] = 0; /* iString not used here */
			*len = 6;
			return 1;
		}
		return 0;
		break;
	case DFU_DETACH:
		if(req->wIndex == USB_ITF_DFU) {
			*complete = dfu_detach_complete;
			return 1;
		}
		return 0;
		break;
	}
	return 0;
}

static void console_cdcacm_data_rx_cb(usbd_device *usbd_dev, uint8_t ep)
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

static void cdcacm_suspend_cb(void)
{
	usb_connected = false;
}

static void cdcacm_set_config(usbd_device *usbd_dev, uint16_t wValue)
{
	(void)wValue;
	int i;

	usbd_ep_setup(usbd_dev, EP_DATA_OUT(USB_ITF_CONSOLE), USB_ENDPOINT_ATTR_BULK,
		USB_BULK_MAX_PACKET_SIZE, console_cdcacm_data_rx_cb);
	usbd_ep_setup(usbd_dev, EP_DATA_IN(USB_ITF_CONSOLE), USB_ENDPOINT_ATTR_BULK,
		USB_BULK_MAX_PACKET_SIZE, NULL);
	usbd_ep_setup(usbd_dev, EP_COMM(USB_ITF_CONSOLE), USB_ENDPOINT_ATTR_INTERRUPT,
		USB_INT_MAX_PACKET_SIZE, NULL);
	usbd_ep_setup(usbd_dev, EP_DATA_OUT(USB_ITF_LOG), USB_ENDPOINT_ATTR_BULK,
		USB_BULK_MAX_PACKET_SIZE, NULL);
	usbd_ep_setup(usbd_dev, EP_DATA_IN(USB_ITF_LOG), USB_ENDPOINT_ATTR_BULK,
		USB_BULK_MAX_PACKET_SIZE, NULL);
	usbd_ep_setup(usbd_dev, EP_COMM(USB_ITF_LOG), USB_ENDPOINT_ATTR_INTERRUPT,
		USB_INT_MAX_PACKET_SIZE, NULL);
	usbd_ep_setup(usbd_dev, EP_DATA_OUT(USB_ITF_TPUART), USB_ENDPOINT_ATTR_BULK,
		USB_BULK_MAX_PACKET_SIZE, NULL);
	usbd_ep_setup(usbd_dev, EP_DATA_IN(USB_ITF_TPUART), USB_ENDPOINT_ATTR_BULK,
		USB_BULK_MAX_PACKET_SIZE, NULL);
	usbd_ep_setup(usbd_dev, EP_COMM(USB_ITF_TPUART), USB_ENDPOINT_ATTR_INTERRUPT,
		USB_INT_MAX_PACKET_SIZE, NULL);
	usbd_register_control_callback(usbd_dev,
		USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
		USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
		cdcacm_control_request);
	for (i = 0; i < 3; i++) {
		cdcacm_set_modem_state(dev, i * 2, true, true);
	}
	usb_connected = true;
	usbd_register_suspend_callback(usbd_dev, cdcacm_suspend_cb);
}

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[256];

void usb_setup(void)
{
	rcc_periph_clock_enable(RCC_USB);
	dev = usbd_init(&st_usbfs_v2_usb_driver, &descr, &config, usb_strings, 7,
		usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(dev, cdcacm_set_config);
	nvic_enable_irq(NVIC_USB_IRQ);
}

void usb_isr(void)
{
	usbd_poll(dev);
}

static void usb_print(enum usb_itf itf, const char *str)
{
	size_t i, len;

	if (!usb_connected) {
		return;
	}
	len = strlen(str);
	for (i = 0; i < len / USB_BULK_MAX_PACKET_SIZE; i++) {
		while (!usbd_ep_write_packet(dev, EP_DATA_IN(itf),
			str + i * USB_BULK_MAX_PACKET_SIZE, USB_BULK_MAX_PACKET_SIZE));
	}
	while (!usbd_ep_write_packet(dev, EP_DATA_IN(itf),
			str + i * USB_BULK_MAX_PACKET_SIZE,
			len % USB_BULK_MAX_PACKET_SIZE));
}

void usb_print_console(const char *str)
{
	usb_print(USB_ITF_CONSOLE, str);
}

void usb_print_log(const char *str)
{
	usb_print(USB_ITF_LOG, str);
}

void usb_print_tpuart(const char *str)
{
	usb_print(USB_ITF_TPUART, str);
}
