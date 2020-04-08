/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

To request to license the code under the MLA license (www.microchip.com/mla_license), 
please contact mla_licensing@microchip.com
*******************************************************************************/

/********************************************************************
-usb_descriptors.c-
-------------------------------------------------------------------
Filling in the descriptor values in the usb_descriptors.c file:
-------------------------------------------------------------------

[Device Descriptors]
The device descriptor is defined as a USB_DEVICE_DESCRIPTOR type.
This type is defined in usb_ch9.h  Each entry into this structure
needs to be the correct length for the data type of the entry.

[Configuration Descriptors]
The configuration descriptor was changed in v2.x from a structure
to a uint8_t array.  Given that the configuration is now a byte array
each byte of multi-byte fields must be listed individually.  This
means that for fields like the total size of the configuration where
the field is a 16-bit value "64,0," is the correct entry for a
configuration that is only 64 bytes long and not "64," which is one
too few bytes.

The configuration attribute must always have the _DEFAULT
definition at the minimum. Additional options can be ORed
to the _DEFAULT attribute. Available options are _SELF and _RWU.
These definitions are defined in the usb_device.h file. The
_SELF tells the USB host that this device is self-powered. The
_RWU tells the USB host that this device supports Remote Wakeup.

[Endpoint Descriptors]
Like the configuration descriptor, the endpoint descriptors were
changed in v2.x of the stack from a structure to a uint8_t array.  As
endpoint descriptors also has a field that are multi-byte entities,
please be sure to specify both bytes of the field.  For example, for
the endpoint size an endpoint that is 64 bytes needs to have the size
defined as "64,0," instead of "64,"

Take the following example:
    // Endpoint Descriptor //
    0x07,                       //the size of this descriptor //
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    _EP02_IN,                   //EndpointAddress
    _INT,                       //Attributes
    0x08,0x00,                  //size (note: 2 bytes)
    0x02,                       //Interval

The first two parameters are self-explanatory. They specify the
length of this endpoint descriptor (7) and the descriptor type.
The next parameter identifies the endpoint, the definitions are
defined in usb_device.h and has the following naming
convention:
_EP<##>_<dir>
where ## is the endpoint number and dir is the direction of
transfer. The dir has the value of either 'OUT' or 'IN'.
The next parameter identifies the type of the endpoint. Available
options are _BULK, _INT, _ISO, and _CTRL. The _CTRL is not
typically used because the default control transfer endpoint is
not defined in the USB descriptors. When _ISO option is used,
addition options can be ORed to _ISO. Example:
_ISO|_AD|_FE
This describes the endpoint as an isochronous pipe with adaptive
and feedback attributes. See usb_device.h and the USB
specification for details. The next parameter defines the size of
the endpoint. The last parameter in the polling interval.

-------------------------------------------------------------------
Adding a USB String
-------------------------------------------------------------------
A string descriptor array should have the following format:

rom struct{byte bLength;byte bDscType;word string[size];}sdxxx={
sizeof(sdxxx),DSC_STR,<text>};

The above structure provides a means for the C compiler to
calculate the length of string descriptor sdxxx, where xxx is the
index number. The first two bytes of the descriptor are descriptor
length and type. The rest <text> are string texts which must be
in the unicode format. The unicode format is achieved by declaring
each character as a word type. The whole text string is declared
as a word array with the number of characters equals to <size>.
<size> has to be manually counted and entered into the array
declaration. Let's study this through an example:
if the string is "USB" , then the string descriptor should be:
(Using index 02)
rom struct{byte bLength;byte bDscType;word string[3];}sd002={
sizeof(sd002),DSC_STR,'U','S','B'};

A USB project may have multiple strings and the firmware supports
the management of multiple strings through a look-up table.
The look-up table is defined as:
rom const unsigned char *rom USB_SD_Ptr[]={&sd000,&sd001,&sd002};

The above declaration has 3 strings, sd000, sd001, and sd002.
Strings can be removed or added. sd000 is a specialized string
descriptor. It defines the language code, usually this is
US English (0x0409). The index of the string must match the index
position of the USB_SD_Ptr array, &sd000 must be in position
USB_SD_Ptr[0], &sd001 must be in position USB_SD_Ptr[1] and so on.
The look-up table USB_SD_Ptr is used by the get string handler
function.

-------------------------------------------------------------------

The look-up table scheme also applies to the configuration
descriptor. A USB device may have multiple configuration
descriptors, i.e. CFG01, CFG02, etc. To add a configuration
descriptor, user must implement a structure similar to CFG01.
The next step is to add the configuration descriptor name, i.e.
cfg01, cfg02,.., to the look-up table USB_CD_Ptr. USB_CD_Ptr[0]
is a dummy place holder since configuration 0 is the un-configured
state according to the definition in the USB specification.

********************************************************************/

/*********************************************************************
 * Descriptor specific type definitions are defined in:
 * usb_device.h
 *
 * Configuration options are defined in:
 * usb_config.h
 ********************************************************************/
#ifndef __USB_DESCRIPTORS_C
#define __USB_DESCRIPTORS_C

/** INCLUDES *******************************************************/
#include "usb.h"
#include "usb_device_audio.h"
#include "usb_device_cdc.h"

/** CONSTANTS ******************************************************/
#if defined(COMPILER_MPLAB_C18)
#pragma romdata
#endif

/* Device Descriptor */
const USB_DEVICE_DESCRIPTOR device_dsc=
{
    0x12,                   // Size of this descriptor in bytes
    USB_DESCRIPTOR_DEVICE,  // DEVICE descriptor type
    0x0200,                 // USB Spec Release Number in BCD format
    0xEF,                   // Class Code - Miscellaneous
    0x02,                   // Subclass code - Common Class
    0x01,                   // Protocol code - Interface Association Descriptor
    USB_EP0_BUFF_SIZE,      // Max packet size for EP0, see usb_config.h
    MY_VID,                 // Vendor ID
    MY_PID,                 // Product ID: Custom HID device demo
    0x0100,                 // Device release number in BCD format
    0x01,                   // Manufacturer string index
    0x02,                   // Product string index
    0x00,                   // Device serial number string index
    0x01                    // Number of possible configurations
};

/* Configuration 1 Descriptor */
/* copied from the midi10.pdf USB Device Class Specification for MIDI Devices */
const uint8_t configDescriptor1[]={
    /* Configuration Descriptor */
    0x09,//sizeof(USB_CFG_DSC),    // Size of this descriptor in bytes
    USB_DESCRIPTOR_CONFIGURATION,  // CONFIGURATION descriptor type
    0xA7,0x00,                     // Total length of data for this cfg
    0x04,                          // Number of interfaces in this cfg
    0x01,                          // Index value of this configuration
    0x00,                          // Configuration string index
    _DEFAULT | _SELF,              // Attributes, see usb_device.h
    50,                            // Max power consumption (2X mA)
    
    /* Interface Descriptor */
    0x09,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,      // INTERFACE descriptor type
    AUDIO_CONTROL_INTF_ID,         // Interface Number
    0x00,                          // Alternate Setting Number
    0x00,                          // Number of endpoints in this intf
    AUDIO_DEVICE,                  // Class code
    AUDIOCONTROL,                  // Subclass code
    0x00,                          // Protocol code
    0x00,                          // Interface string index

    /* MIDI Adapter Class-specific AC Interface Descriptor */
    0x09,                          //bLength
    CS_INTERFACE,                  //bDescriptorType - CS_INTERFACE
    HEADER,                        //bDescriptorSubtype - HEADER
    0x00,0x01,                     //bcdADC
    0x09,0x00,                     //wTotalLength
    0x01,                          //bInCollection
    0x01,                          //baInterfaceNr(1)

    /* MIDI Adapter Standard MS Interface Descriptor */
    0x09,                          //bLength
    USB_DESCRIPTOR_INTERFACE,      //bDescriptorType
    AUDIO_MIDISTREAMING_INTF_ID,   //bInterfaceNumber
    0x00,                          //bAlternateSetting
    0x02,                          //bNumEndpoints
    AUDIO_DEVICE,                  //bInterfaceClass
    MIDISTREAMING,                 //bInterfaceSubclass
    0x00,                          //bInterfaceProtocol
    0x00,                          //iInterface

    /* MIDI Adapter Class-specific MS Interface Descriptor */
    0x07,                          //bLength
    CS_INTERFACE,                  //bDescriptorType - CS_INTERFACE
    HEADER,                        //bDescriptorSubtype - MS_HEADER
    0x00,0x01,                     //BcdADC
    0x41,0x00,                     //wTotalLength

    /* MIDI Adapter MIDI IN Jack Descriptor (Embedded) */
    0x06,                          //bLength
    CS_INTERFACE,                  //bDescriptorType - CS_INTERFACE
    INPUT_TERMINAL,                //bDescriptorSubtype - MIDI_IN_JACK
    0x01,                          //bJackType - EMBEDDED
    0x01,                          //bJackID
    0x00,                          //iJack

    /* MIDI Adapter MIDI IN Jack Descriptor (External) */
    0x06,                          //bLength
    CS_INTERFACE,                  //bDescriptorType - CS_INTERFACE
    INPUT_TERMINAL,                //bDescriptorSubtype - MIDI_IN_JACK
    0x02,                          //bJackType - EXTERNAL
    0x02,                          //bJackID
    0x00,                          //iJack

    /* MIDI Adapter MIDI OUT Jack Descriptor (Embedded) */
    0x09,                          //bLength
    CS_INTERFACE,                  //bDescriptorType - CS_INTERFACE
    OUTPUT_TERMINAL,               //bDescriptorSubtype - MIDI_OUT_JACK
    0x01,                          //bJackType - EMBEDDED
    0x03,                          //bJackID
    0x01,                          //bNrInputPins
    0x02,                          //BaSourceID(1)
    0x01,                          //BaSourcePin(1)
    0x00,                          //iJack

    /* MIDI Adapter MIDI OUT Jack Descriptor (External) */
    0x09,                          //bLength
    CS_INTERFACE,                  //bDescriptorType - CS_INTERFACE
    OUTPUT_TERMINAL,               //bDescriptorSubtype - MIDI_OUT_JACK
    0x02,                          //bJackType - EXTERNAL
    0x04,                          //bJackID
    0x01,                          //bNrInputPins
    0x01,                          //BaSourceID(1)
    0x01,                          //BaSourcePin(1)
    0x00,                          //iJack

    /* MIDI Adapter Standard Bulk OUT Endpoint Descriptor */
    0x09,                          //bLength
    USB_DESCRIPTOR_ENDPOINT,       //bDescriptorType - ENDPOINT
    AUDIO_MIDI_EP | _EP_OUT,       //bEndpointAddress - OUT
    0x02,                          //bmAttributes
    AUDIO_MIDI_OUT_EP_SIZE,0x00,   //wMaxPacketSize
    0x00,                          //bInterval
    0x00,                          //bRefresh
    0x00,                          //bSynchAddress

    /* MIDI Adapter Class-specific Bulk OUT Endpoint Descriptor */
    0x05,                          //bLength
    CS_ENDPOINT,                   //bDescriptorType - CS_ENDPOINT
    EP_GENERAL,                    //bDescriptorSubtype - MS_GENERAL
    0x01,                          //bNumEmbMIDIJack
    0x01,                          //BaAssocJackID(1)

    /* MIDI Adapter Standard Bulk IN Endpoint Descriptor */
    0x09,                          //bLength
    USB_DESCRIPTOR_ENDPOINT,       //bDescriptorType
    AUDIO_MIDI_EP | _EP_IN,        //bEndpointAddress
    0x02,                          //bmAttributes
    AUDIO_MIDI_IN_EP_SIZE,0x00,    //wMaxPacketSize
    0x00,                          //bInterval
    0x00,                          //bRefresh
    0x00,                          //bSynchAddress

    /* MIDI Adapter Class-specific Bulk IN Endpoint Descriptor */
    0x05,                          //bLength
    CS_ENDPOINT,                   //bDescriptorType - CS_ENDPOINT
    EP_GENERAL,                    //bDescriptorSubtype - MS_GENERAL
    0x01,                          //bNumEmbMIDIJack
    0x03,                          //BaAssocJackID(1)
            
    /* Interface Association Descriptor - IAD */
    0x08,                          // bLength
    0x0B,                          // bDescriptorType - IAD
    CDC_COMM_INTF_ID,              // bFirstInterface
    0x02,                          // bInterfaceCount
    COMM_INTF,                     // bDeviceClass
    ABSTRACT_CONTROL_MODEL,        // bDeviceSubClass
    V25TER,                        // bDeviceProtocol
    0x00,                          // iFunction 
    
    /* Interface Descriptor */
    0x09,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,      // INTERFACE descriptor type
    CDC_COMM_INTF_ID,              // Interface Number
    0x00,                          // Alternate Setting Number
    0x01,                          // Number of endpoints in this intf
    COMM_INTF,                     // Class code
    ABSTRACT_CONTROL_MODEL,        // Subclass code
    V25TER,                        // Protocol code
    0x00,                          // Interface string index

    /* CDC Class-Specific Descriptors */
    sizeof(USB_CDC_HEADER_FN_DSC),
    CS_INTERFACE,                  // bDescriptorType
    DSC_FN_HEADER,                 // bDescriptorSubtype
    0x20,0x01,                     // bcdCDC

    sizeof(USB_CDC_ACM_FN_DSC),
    CS_INTERFACE,                  // bDescriptorType
    DSC_FN_ACM,                    // bDescriptorSubtype
    USB_CDC_ACM_FN_DSC_VAL,        // bmCapabilities

    sizeof(USB_CDC_UNION_FN_DSC),
    CS_INTERFACE,                  // bDescriptorType
    DSC_FN_UNION,                  // bDescriptorSubtype
    CDC_COMM_INTF_ID,              // bControlInterface
    CDC_DATA_INTF_ID,              // bSubordinateInterface0

    sizeof(USB_CDC_CALL_MGT_FN_DSC),
    CS_INTERFACE,                  // bDescriptorType
    DSC_FN_CALL_MGT,               // bDescriptorSubtype
    0x00,                          // bmCapabilities
    CDC_DATA_INTF_ID,              // bDataInterface

    /* Endpoint Descriptor */
    //sizeof(USB_EP_DSC),DSC_EP,_EP02_IN,_INT,CDC_INT_EP_SIZE,0x02,
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,       //Endpoint Descriptor
    CDC_COMM_EP | _EP_IN,          //EndpointAddress
    _INTERRUPT,                    //Attributes
    CDC_COMM_IN_EP_SIZE,0x00,      //Size
    0x02,                          //Interval

    /* Interface Descriptor */
    0x09,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,      // INTERFACE descriptor type
    CDC_DATA_INTF_ID,              // Interface Number
    0x00,                          // Alternate Setting Number
    0x02,                          // Number of endpoints in this intf
    DATA_INTF,                     // Class code
    0x00,                          // Subclass code
    NO_PROTOCOL,                   // Protocol code
    0x00,                          // Interface string index
    
    /* Endpoint Descriptor */
    //sizeof(USB_EP_DSC),DSC_EP,_EP03_OUT,_BULK,CDC_BULK_OUT_EP_SIZE,0x00,
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,       //Endpoint Descriptor
    CDC_DATA_EP | _EP_OUT,         //EndpointAddress
    _BULK,                         //Attributes
    CDC_DATA_OUT_EP_SIZE,0x00,     //size
    0x00,                          //Interval

    /* Endpoint Descriptor */
    //sizeof(USB_EP_DSC),DSC_EP,_EP03_IN,_BULK,CDC_BULK_IN_EP_SIZE,0x00
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,       //Endpoint Descriptor
    CDC_DATA_EP | _EP_IN,          //EndpointAddress
    _BULK,                         //Attributes
    CDC_DATA_IN_EP_SIZE,0x00,      //size
    0x00,                          //Interval
};


//Language code string descriptor
const struct{uint8_t bLength;uint8_t bDscType;uint16_t string[1];}sd000={
sizeof(sd000),USB_DESCRIPTOR_STRING,{0x0409
}};

//Manufacturer string descriptor
const struct{uint8_t bLength;uint8_t bDscType;uint16_t string[25];}sd001={
sizeof(sd001),USB_DESCRIPTOR_STRING,
{'M','i','c','r','o','c','h','i','p',' ',
'T','e','c','h','n','o','l','o','g','y',' ','I','n','c','.'
}};

//Product string descriptor
const struct{uint8_t bLength;uint8_t bDscType;uint16_t string[11];}sd002={
sizeof(sd002),USB_DESCRIPTOR_STRING,
{'M','I','D','I',' ','B','r','i','d','g','e'}};

//Array of configuration descriptors
const uint8_t *const USB_CD_Ptr[]=
{
    (const uint8_t *const)&configDescriptor1
};

//Array of string descriptors
const uint8_t *const USB_SD_Ptr[]=
{
    (const uint8_t *const)&sd000,
    (const uint8_t *const)&sd001,
    (const uint8_t *const)&sd002
};

/** EOF usb_descriptors.c ***************************************************/

#endif

