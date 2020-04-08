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

`app_device_audio_midi.c` contains the main task for the MIDI interface.

# Descriptor

If you're looking for the descriptor of the composite device is located in `usb/usb_descriptors.c`.