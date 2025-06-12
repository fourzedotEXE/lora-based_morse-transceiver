# LoRa morse transceiver for stealthy long range comms.

The system consists of two devices: 
1) The main device hooked up to a computer, and controlled via serial monitor on an IDE
2) The LoRa transceiver. Standalone device.

### Components and Tools ( :warning: NOTE: Some soldering may be necessary :warning: )
- Arduino NANO or UNO 3x (2 for the transceiver, 1 for the main device)
- LoRa Module SX1276 2x (MUST OPERATE AT 915MHZ, DO NOT MISTAKE WITH SX1278 MODULES)
- 360 Degree Rotary Encoder (CYT1100)
- Push Button
- 3.7V 104245 2000mAh Lithium Ion Polymer Batteries Rechargeable Lipo Battery Pack with PH2.0mm JST
- MT3608 DC-DC Step Up Boost Power Converter Adjustable 2A Module Step Up Voltage Regulator Board Input 2-24V to Output 5V-28V
- TP4056 Type-c USB 5V 1A 18650 Lithium Battery Charger Module
- Micro Slide Switch (SS12F15)
- A computer + monitor with Arduino IDE or Platform.io
- 128x64 OLED LCD Display Module SSD1309 7 Pin SPI/IIC I2C
- I2C OLED Display Module 0.91 Inch I2C SSD1306 OLED
- Breadboard Jumper wires
- 3D Printer for enclosure (STL File included in the repo)
- Soldering (KiCad Schematic included in the repo)

### Installation
- Download the .ino scripts as a part of this repository
- Install the required libraries as defined by each of the scripts
- Use the Arduino IDE or Platform.io w/VSCode in order to upload the .ino scripts to both the transceiver and the main devices
- Make sure the transceiver is properly charged and power on
- Allow the transceiver to search for the main device before attempting to communicate

### Libraries Used / Referenced Sources
- LoRa Interfacing Library
  - https://github.com/sandeepmistry/arduino-LoRa
- CBC Encryption for Low-memory devices
  - https://github.com/suculent/thinx-aes-lib
- 3D Printed Housing Model
  - https://www.printables.com/model/1113713-klipper-case/files

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

### Standards and Regulations :warning:
- United States FCC Regulations : radio operations @ 915Mhz for unliscensed users
  - https://www.ecfr.gov/current/title-47/chapter-I/subchapter-A/part-15/subpart-C/subject-group-ECFR2f2e5828339709e/section-15.247
