# RP2350-LCD-scripts
<img width="420" height="206" alt="image" src="https://github.com/user-attachments/assets/459e7fd4-54c3-4834-a5c4-7357a8808431" />


## Setup

### Arduino IDE Installation
1. Download and install the [Arduino IDE](https://www.arduino.cc/en/software)
2. read Installation step in https://github.com/earlephilhower/arduino-pico
3. Open Arduino IDE and go to **Sketch → Board Manager**
4. Search for "Raspberry Pi Pico" and install **Raspberry Pi Pico/RP2040/RP2350** by Earle F. Philhower, III
5. Select the board from **Tools → Board → Raspberry Pi Pico/RP2040/RP2350 → Generic RP2350**
6. Select the UF2_Board port from **Tools → Port**

### Flashing the Device
When connecting the RP2350 to your computer via USB for the first time, **hold the BOOT button** while plugging in the USB cable. This puts the device in bootloader mode, allowing the Arduino IDE to flash the code.


## Run Scripts

Open folder of desired script and double click on the .ino file ad press upload in the arduino IDE

### show_ip

Display network information (hostname and IP addresses) on the LCD from Windows or iPhone.

#### Windows
- Flash [show_ip_win_US.ino](show_ip/show_ip_win_US.ino) (US keyboard) or [show_ip_win_SWE.ino](show_ip/show_ip_win_SWE.ino) (Swedish keyboard)
- The sketch automatically triggers a PowerShell script to fetch and send the host name and IPv4 addresses over USB serial


### Mouse_wiggler

Random mouse movement simulator that keeps your device awake by wiggling the mouse pointer at random intervals with random movements.

- Flash [Mouse_wiggler.ino](show_ip/Mouse_wiggler.ino)
- The LCD displays the current movement vector (dx/dy) and countdown to the next move
- All movements are 100% randomized:
  - Random movement direction and magnitude
  - Random number of steps per movement
  - Random pause duration (300ms - 20 seconds)

### iPhone Keyboard Test

Send text to an iPhone via USB HID keyboard by automatically launching the Notes app and typing a message.

- Flash [iphone_keyboard_test.ino](show_ip/iphone_keyboard_test.ino)
- Automatically opens Spotlight, searches for Notes, launches it, and types a test message
- Demonstrates USB HID keyboard support on iOS
