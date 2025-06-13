# LoRa morse transceiver for stealthy long range comms.

The system consists of two devices: 
1) The main device hooked up to a computer, and controlled via serial monitor on an IDE
2) The LoRa transceiver. Standalone device.

### Components and Tools ( :warning: NOTE: Some soldering may be necessary :warning: )
- 3x Arduino NANO or UNO (2 for the transceiver, 1 for the main device)
- 2x LoRa Module RYLR896 SX1276 (MUST OPERATE AT 915MHZ, DO NOT MISTAKE WITH SX1278 MODULES)
- 1x 360 Degree Rotary Encoder (CYT1100)
- 1x Push Button
- 1x 3.7V 104245 2000mAh Lithium Ion Polymer Batteries Rechargeable Lipo Battery Pack with PH2.0mm JST
- 1x MT3608 DC-DC Step Up Boost Power Converter Adjustable 2A Module Step Up Voltage Regulator Board Input 2-24V to Output 5V-28V
- 1x LM2596 LM2596S DC-DC Step Down Variable Volt Regulator Input 3.0-40V Output 1.5-35V (Other step-down buck converters are sufficient; These are just the ones I had handy)
- 1x TP4056 Type-c USB 5V 1A 18650 Lithium Battery Charger Module
- 1x Micro Slide Switch (SS12F15)
- 1x 128x64 OLED LCD Display Module SSD1309 7 Pin SPI/IIC I2C
- 1x I2C OLED Display Module 0.91 Inch I2C SSD1306 OLED
- A computer + monitor with Arduino IDE or Platform.io
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
- The transceiver will automatically connect to LoRa P2P as soon as it is discoverable
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
