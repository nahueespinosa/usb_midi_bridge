# USB MIDI Bridge for PIC18F2550/4550

Implementation of a composite USB device containing a CDC interface and a audio MIDI interface based on examples from Microchip Libraries for Application (MLA).
It works as a bridge. Every packet that comes from one interface goes to the other.

- Device: PIC18F2550 or PIC18F4550
- IDE: MPLAB X IDE v5.30
- Compiler: XC8 v2.10
- Written in C 90 standard

I tried to modify as little as possible the examples provided by MLA, but the structure is quite messy.

# Usage

Before programming a device, check the files `bsp/leds.c` and `bsp/buttons.c` to configure the right GPIOs.

`app_device_cdc_basic.c` contains the main task for the CDC interface.

`app_device_audio_midi.c` contains the main task for the MIDI interface. Also, if `BUTTON_DEVICE_AUDIO_MIDI` is pressed, generates a MIDI packet.

`app_led_usb_status.c` contains the status LED update task to reflect the status of the USB connection.

# Descriptor

If you're looking for a descriptor for the composite device is located at `usb/usb_descriptors.c`.