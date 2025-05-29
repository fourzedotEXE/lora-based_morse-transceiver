# morse-communicator
LoRa-based morse transceiver for stealthy communication. Consists of two devices: 
1) The main device hooked up to a computer, and controlled via serial monitor on an IDE
2) The LoRa transceiver. Standalone device

### Components
- Arduino Uno
- Arduino Nano 2x
- LoRa Module SX1278 2x
- 360 Degree Rotary Encoder (CYT1100)
- Push Button
- 3.7V 1100mAh Lithium Rechargeable Battery
- TP4056 Type-c USB 5V 1A 18650 Lithium Battery Charger Module
- Micro Slide Switch (SS12F15)
- 
- A computer with Arduino IDE or Platform.io


### Installation
- Download the .ino scripts as a part of this repository
- Use the Arduino IDE or Platform.io w/VSCode in order to upload the .ino scripts to both the transceiver and the main devices
- Make sure the transceiver is properly charged and power on
- Allow the transceiver to search for the main device before attempting to communicate

### How to operate
- The device uses an encoder to scroll select characters through a keyboard shown on the main display
- Sent and received messages are seen on the seconrdary display
- The device will automatically connect to the LoRa end-point as soon as it is discoverable
- Use the push-button to transmit a message once you are finished typing
