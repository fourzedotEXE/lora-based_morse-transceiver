# LoRa morse transceiver for stealthy long range comms.

The system consists of two devices: 
1) The main device hooked up to a computer, and controlled via serial monitor on an IDE
2) The LoRa transceiver. Standalone device.

### Components ( :warning: NOTE: Some soldering may be necessary :warning: )
- Arduino Nano 3x (2 for the transceiver, 1 for the main device)
- LoRa Module SX1278 2x
- 360 Degree Rotary Encoder (CYT1100)
- Push Button
- 3.7V 1100mAh Lithium Rechargeable Battery
- TP4056 Type-c USB 5V 1A 18650 Lithium Battery Charger Module
- Micro Slide Switch (SS12F15)
- A computer + monitor with Arduino IDE or Platform.io
- 128x64 OLED LCD Display Module SSD1309 7 Pin SPI/IIC I2C
- I2C OLED Display Module 0.91 Inch I2C SSD1306 OLED
- Breadboard Jumper wires

### Installation
- Download the .ino scripts as a part of this repository
- Install the required libraries as defined by each of the scripts
- Use the Arduino IDE or Platform.io w/VSCode in order to upload the .ino scripts to both the transceiver and the main devices
- Make sure the transceiver is properly charged and power on
- Allow the transceiver to search for the main device before attempting to communicate

### Libraries Used
- https://github.com/sandeepmistry/arduino-LoRa

### How to operate
- The transceiver uses an encoder to scroll select characters through a keyboard shown on the main display
- Sent and received messages are seen on the secondary display
- The transceiver will automatically connect to the LoRa P2P as soon as it is discoverable
  - Successful connection will be indicated by a pop-up message on the seconary display of the transceiver
  - The serial monitor from the main device will also notify that a connection has been established
- Use the push-button to transmit a message from the transceiver
- Type into the serial monitor to transmit a message from the main device

### Remaning Issues :warning:
- Message transmission on the tranceiver is inconsistent only on start-up
  - Solution: Send an initial dummy message when starting the transceiver
