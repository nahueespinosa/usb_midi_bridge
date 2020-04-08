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

/** INCLUDES *******************************************************/
#include <stdint.h>
#include <stdbool.h>

#include "system.h"

#include "usb.h"
#include "usb_device_midi.h"

/** VARIABLES ******************************************************/
/* Some processors have a limited range of RAM addresses where the USB module
 * is able to access.  The following section is for those devices.  This section
 * assigns the buffers that need to be used by the USB module into those
 * specific areas.
 */
#if defined(FIXED_ADDRESS_MEMORY)
    #if defined(COMPILER_MPLAB_C18)
        #pragma udata DEVICE_AUDIO_MIDI_RX_DATA_BUFFER=DEVCE_AUDIO_MIDI_RX_DATA_BUFFER_ADDRESS
            static uint8_t ReceivedDataBuffer[64];
        #pragma udata DEVICE_AUDIO_MIDI_EVENT_DATA_BUFFER=DEVCE_AUDIO_MIDI_EVENT_DATA_BUFFER_ADDRESS
            static USB_AUDIO_MIDI_EVENT_PACKET midiData;
        #pragma udata
    #elif defined(__XC8)
        static uint8_t ReceivedDataBuffer[64] @ DEVCE_AUDIO_MIDI_RX_DATA_BUFFER_ADDRESS;
        static USB_AUDIO_MIDI_EVENT_PACKET midiData @ DEVCE_AUDIO_MIDI_EVENT_DATA_BUFFER_ADDRESS;
    #endif
#else
    static uint8_t ReceivedDataBuffer[64];
    static USB_AUDIO_MIDI_EVENT_PACKET midiData;
#endif

static USB_HANDLE USBTxHandle;
static USB_HANDLE USBRxHandle;

extern uint8_t readBuffer[CDC_DATA_OUT_EP_SIZE];
extern uint8_t writeBuffer[CDC_DATA_IN_EP_SIZE];

static uint8_t pitch;
static bool sentNoteOff;
bool midiTX;
bool midiRX;

static USB_VOLATILE uint16_t msCounter;

extern volatile uint16_t blinkTime;

/*********************************************************************
* Function: void APP_DeviceAudioMIDIInitialize(void);
*
* Overview: Initializes the demo code
*
* PreCondition: None
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceAudioMIDIInitialize()
{
    USBTxHandle = NULL;
    USBRxHandle = NULL;

    pitch = 0x3C;
    sentNoteOff = true;
    midiTX = false;
    midiRX = false;

    msCounter = 0;

    //enable the HID endpoint
    USBEnableEndpoint(AUDIO_MIDI_EP,USB_OUT_ENABLED|USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);

    //Re-arm the OUT endpoint for the next packet
    USBRxHandle = USBRxOnePacket(AUDIO_MIDI_EP,(uint8_t*)&ReceivedDataBuffer,64);
}

/*********************************************************************
* Function: void APP_DeviceAudioMIDIInitialize(void);
*
* Overview: Initializes the demo code
*
* PreCondition: None
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceAudioMIDISOFHandler()
{
    if(msCounter != 0)
    {
        msCounter--;
    }
}


/*********************************************************************
* Function: void APP_DeviceAudioMIDITasks(void);
*
* Overview: Keeps the Custom HID demo running.
*
* PreCondition: The demo should have been initialized and started via
*   the APP_DeviceAudioMIDIInitialize() and APP_DeviceAudioMIDIStart() demos
*   respectively.
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceAudioMIDITasks()
{
     uint8_t numBytesRead;
    
    /* If the device is not configured yet, or the device is suspended, then
     * we don't need to run the demo since we can't send any data.
     */
    if( (USBGetDeviceState() < CONFIGURED_STATE) ||
        (USBIsDeviceSuspended() == true))
    {
        return;
    }

    if(!USBHandleBusy(USBRxHandle))
    {
        //We have received a MIDI packet from the host, process it and then
        //  prepare to receive the next packet

        if( midiData.DATA_2 > 0 ) {             // velocity
            blinkTime = (0x4A - midiData.DATA_1) * 10;    // pitch * 10
        } else {
            blinkTime = 0;
        }
        
        if( midiTX == false ) {
            writeBuffer[0] = midiData.v[0];
            writeBuffer[1] = midiData.v[1];
            writeBuffer[2] = midiData.v[2];
            writeBuffer[3] = midiData.v[3];

            midiTX = true;
        }
        
        //Get ready for next packet (this will overwrite the old data)
        USBRxHandle = USBRxOnePacket(AUDIO_MIDI_EP,(uint8_t*)&ReceivedDataBuffer,64);
    }  

    if( midiRX == true )
    {
        /* and we aren't currently trying to transmit data... */
        if(!USBHandleBusy(USBTxHandle))
        {
            midiData.Val = 0;   //must set all unused values to 0 so go ahead
                                //  and set them all to 0
            midiData.v[0] = readBuffer[0];
            midiData.v[1] = readBuffer[1];
            midiData.v[2] = readBuffer[2];
            midiData.v[3] = readBuffer[3];

            midiRX = false;

            USBTxHandle = USBTxOnePacket(AUDIO_MIDI_EP,(uint8_t*)&midiData,4);
        }
    }
    
    /* If the user button is pressed... */
    if(BUTTON_IsPressed(BUTTON_DEVICE_AUDIO_MIDI) == true)
    {
        /* and we haven't sent a transmission in the past 100ms... */
        if(msCounter == 0)
        {
            /* and we have sent the NOTE_OFF for the last note... */
            if(sentNoteOff == true)
            {
                /* and we aren't currently trying to transmit data... */
                if(!USBHandleBusy(USBTxHandle))
                {
                    //Then reset the 1000ms counter
                    msCounter = 100;

                    midiData.Val = 0;   //must set all unused values to 0 so go ahead
                                        //  and set them all to 0

                    midiData.CableNumber = 0;
                    midiData.CodeIndexNumber = MIDI_CIN_NOTE_ON;
                    midiData.DATA_0 = 0x90;         //Note on
                    midiData.DATA_1 = pitch;        //pitch
                    midiData.DATA_2 = 0x7F;         //velocity

                    USBTxHandle = USBTxOnePacket(AUDIO_MIDI_EP,(uint8_t*)&midiData,4);
                    /* we now need to send the NOTE_OFF for this note. */
                    sentNoteOff = false;
                }
            }
        }
    }
    else
    {
        if(msCounter == 0)
        {
            if(sentNoteOff == false)
            {
                if(!USBHandleBusy(USBTxHandle))
                {
                    //Debounce counter for 100ms
                    msCounter = 100;

                    midiData.Val = 0;   //must set all unused values to 0 so go ahead
                                        //  and set them all to 0

                    midiData.CableNumber = 0;
                    midiData.CodeIndexNumber = MIDI_CIN_NOTE_ON;
                    midiData.DATA_0 = 0x90;         //Note off
                    midiData.DATA_1 = pitch++;      //pitch
                    midiData.DATA_2 = 0x00;         //velocity

                    if(pitch == 0x49)
                    {
                        pitch = 0x3C;
                    }

                    USBTxHandle = USBTxOnePacket(AUDIO_MIDI_EP,(uint8_t*)&midiData,4);         
                    sentNoteOff = true;
                }
            }
        }
    } 
}
