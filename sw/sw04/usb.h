#ifndef USB_H_
#define USB_H_

#define USB_EP_IN	0x80
#define USB_INT_MAX_PACKET_SIZE		16
#define USB_BULK_MAX_PACKET_SIZE 	64

/* This is the start of the test-block I added to test dual CDC */

#define WBVAL(x) (x & 0xFF),((x >> 8) & 0xFF)

#define USB_CONFIGUARTION_DESC_SIZE             9
#define USB_INTERFACE_DESC_SIZE                 9
#define USB_ENDPOINT_DESC_SIZE                  7
#define USB_ENDPOINT_TYPE_BULK                  0x02
#define USB_ENDPOINT_TYPE_INTERRUPT             0x03

#define USB_CONFIG_BUS_POWERED                 0x80
#define USB_CONFIG_SELF_POWERED                0xC0
#define USB_CONFIG_POWER_MA(mA)                ((mA)/2)

#define CDC_COMMUNICATION_INTERFACE_CLASS       0x02
#define CDC_DATA_INTERFACE_CLASS                0x0A
#define CDC_ABSTRACT_CONTROL_MODEL              0x02
#define CDC_CS_INTERFACE                        0x24
#define CDC_HEADER                              0x00
#define CDC_CALL_MANAGEMENT                     0x01
#define CDC_ABSTRACT_CONTROL_MANAGEMENT         0x02
#define CDC_UNION                               0x06


#define CDC_IF_DESC_SET_SIZE    ( USB_INTERFACE_DESC_SIZE + 0x05 + 0x05 + 0x04 + 0x05 + USB_ENDPOINT_DESC_SIZE \
                                + USB_INTERFACE_DESC_SIZE + 2 * USB_ENDPOINT_DESC_SIZE )

#define CDC_IF_DESC_SET( comIfNum, datIfNum, comInEp, datOutEp, datInEp )   \
/* CDC Communication Interface Descriptor */                            \
    USB_INTERFACE_DESC_SIZE,                /* bLength */               \
    USB_INTERFACE_DESCRIPTOR_TYPE,          /* bDescriptorType */       \
    comIfNum,                               /* bInterfaceNumber */      \
    0x00,                                   /* bAlternateSetting */     \
    0x01,                                   /* bNumEndpoints */         \
    CDC_COMMUNICATION_INTERFACE_CLASS,      /* bInterfaceClass */       \
    CDC_ABSTRACT_CONTROL_MODEL,             /* bInterfaceSubClass */    \
    0x01,                                   /* bInterfaceProtocol */    \
    0x00,                                   /* iInterface */            \
/* Header Functional Descriptor */                                      \
    0x05,                                   /* bLength */               \
    CDC_CS_INTERFACE,                       /* bDescriptorType */       \
    CDC_HEADER,                             /* bDescriptorSubtype */    \
    WBVAL(CDC_V1_10), /* 1.10 */            /* bcdCDC */                \
/* Call Management Functional Descriptor */                             \
    0x05,                                   /* bFunctionLength */       \
    CDC_CS_INTERFACE,                       /* bDescriptorType */       \
    CDC_CALL_MANAGEMENT,                    /* bDescriptorSubtype */    \
    0x03,                                   /* bmCapabilities */        \
    datIfNum,                               /* bDataInterface */        \
/* Abstract Control Management Functional Descriptor */                 \
    0x04,                                   /* bFunctionLength */       \
    CDC_CS_INTERFACE,                       /* bDescriptorType */       \
    CDC_ABSTRACT_CONTROL_MANAGEMENT,        /* bDescriptorSubtype */    \
    0x02,                                   /* bmCapabilities */        \
/* Union Functional Descriptor */                                       \
    0x05,                                   /* bFunctionLength */       \
    CDC_CS_INTERFACE,                       /* bDescriptorType */       \
    CDC_UNION,                              /* bDescriptorSubtype */    \
    comIfNum,                               /* bMasterInterface */      \
    datIfNum,                               /* bSlaveInterface0 */      \
/* Endpoint, Interrupt IN */                /* event notification */    \
    USB_ENDPOINT_DESC_SIZE,                 /* bLength */               \
    USB_ENDPOINT_DESCRIPTOR_TYPE,           /* bDescriptorType */       \
    comInEp,                                /* bEndpointAddress */      \
    USB_ENDPOINT_TYPE_INTERRUPT,            /* bmAttributes */          \
    WBVAL(0x000A),                          /* wMaxPacketSize */        \
    0x01,                                   /* bInterval */             \
                                                                        \
/* CDC Data Interface Descriptor */                                     \
    USB_INTERFACE_DESC_SIZE,                /* bLength */               \
    USB_INTERFACE_DESCRIPTOR_TYPE,          /* bDescriptorType */       \
    datIfNum,                               /* bInterfaceNumber */      \
    0x00,                                   /* bAlternateSetting */     \
    0x02,                                   /* bNumEndpoints */         \
    CDC_DATA_INTERFACE_CLASS,               /* bInterfaceClass */       \
    0x00,                                   /* bInterfaceSubClass */    \
    0x00,                                   /* bInterfaceProtocol */    \
    0x00,                                   /* iInterface */            \
/* Endpoint, Bulk OUT */                                                \
    USB_ENDPOINT_DESC_SIZE,                 /* bLength */               \
    USB_ENDPOINT_DESCRIPTOR_TYPE,           /* bDescriptorType */       \
    datOutEp,                               /* bEndpointAddress */      \
    USB_ENDPOINT_TYPE_BULK,                 /* bmAttributes */          \
    WBVAL(VIRTUAL_COM_PORT_DATA_SIZE),      /* wMaxPacketSize */        \
    0x00,                                   /* bInterval */             \
/* Endpoint, Bulk IN */                                                 \
    USB_ENDPOINT_DESC_SIZE,                 /* bLength */               \
    USB_ENDPOINT_DESCRIPTOR_TYPE,           /* bDescriptorType */       \
    datInEp,                                /* bEndpointAddress */      \
    USB_ENDPOINT_TYPE_BULK,                 /* bmAttributes */          \
    WBVAL(VIRTUAL_COM_PORT_DATA_SIZE),      /* wMaxPacketSize */        \
    0x00                                    /* bInterval */


#define IAD_CDC_IF_DESC_SET_SIZE    ( 8 + CDC_IF_DESC_SET_SIZE )

#define IAD_CDC_IF_DESC_SET( comIfNum, datIfNum, comInEp, datOutEp, datInEp )   \
/* Interface Association Descriptor */                                  \
    0x08,                                   /* bLength */               \
    0x0B,                                   /* bDescriptorType */       \
    comIfNum,                               /* bFirstInterface */       \
    0x02,                                   /* bInterfaceCount */       \
    CDC_COMMUNICATION_INTERFACE_CLASS,      /* bFunctionClass */        \
    CDC_ABSTRACT_CONTROL_MODEL,             /* bFunctionSubClass */     \
    0x01,                                   /* bFunctionProcotol */     \
    0x00,                                   /* iInterface */            \
/* CDC Interface descriptor set */                                      \
    CDC_IF_DESC_SET( comIfNum, datIfNum, comInEp, datOutEp, datInEp )


/* This is the end of the test-block I added to test dual CDC */



void usb_setup(void);
void usb_poll(void);
void usb_print(const char *str);

#endif
