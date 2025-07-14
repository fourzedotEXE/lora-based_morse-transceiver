# LoRa-base morse-transceiver for stealthy long range comms.

### Primary goals ✔️
1) To leverage the use of the LoRa communication protocol in order to allow for long range communication for cheap and with minimal hardware overhead
2) Demonstrate the securing of P2P (Peer-to-Peer) communication under LoRa without the use of an external network by way of AES256 encryption libraries utilized in C (Note that the encryption mode is only EBC for proof-of-concept purposes, and thus is not actually secure. Use CBC encryption for security)
4) Prototype a low-cost device using over-the-counter MCUs
   - Utilizing UART, I2C, and SPI communicationin the system
   - Harnessing the capabilities of hardware interrupts for push-button controls
   - Applying concepts of electrical engineering and computer science to utilize several different technologies (OLED Displays, Rotary encoders, step-up and step-down transformers, LoRa Modules)

### The system consists of two devices: 
1) The main device hooked up to a computer, and controlled via serial monitor on an IDE
2) The LoRa transceiver. Standalone device.

### Components and Tools ( :warning: NOTE: Some soldering may be necessary :warning: )
- 3x Arduino NANO or UNO (2 for the transceiver, 1 for the main device)
- 2x LoRa Module RYLR896 SX1276 (MUST OPERATE AT 915MHZ, DO NOT MISTAKE WITH SX1278 MODULES)
- 1x 360 Degree Rotary Encoder (CYT1100)
- 1x Push Button
- 1x 3.7V 104245 2000mAh Lithium Ion Polymer Batteries Rechargeable Lipo Battery Pack with PH2.0mm JST
- 1x MT3608 DC-DC Step Up Boost Power Converter Adjustable 2A Module Step Up Voltage Regulator Board Input 2-24V to Output 5V-28V
  - Step up 3.7V from LiPo to more stable 5V
- 2x LM2596 LM2596S DC-DC Step Down Variable Volt Regulator Input 3.0-40V Output 1.5-35V (Other step-down buck converters are sufficient; These are just the ones I had handy)
  - For powering both RYLR896 Modules, step down from Arduino's 5V to 3.3V
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

### Standards and Regulations :warning:
- United States FCC Regulations : radio operations @ 915Mhz for unliscensed users
  - https://www.ecfr.gov/current/title-47/chapter-I/subchapter-A/part-15/subpart-C/subject-group-ECFR2f2e5828339709e/section-15.247

### Results
- Ultimately, the device was successful in communicating morse code to english text over LoRa. However, the final aes-256 encryption could not be accomodated due to the lack of dynamic memory on the Arduino MCU.
- The device is not necessarily secure in communicating over long distances but for proof-of-concept, it is possible to interally encrypt and decrypt data within a single Arduino. Said differently, you can make the arduino encrypt its own data and and decrypt it as well. The only difference is had it been implemented with the LoRa functionality, the encryption would happen on one device and the decryption would happen on the other, with the extra step of transmitting the data over LoRa.
- Lessons learned:
  - Designing hardware at a system level
  - Using concepts learned in university to achieve a goal (circuit analysis, data types in C, embedded systems knowledge, signals and systems)
  - Arduino is a hobbyist-platform and you are severly limited to what you can do with just a single Arduino MCU
 
### Remaning Issues :warning:
- Message transmission on the tranceiver is inconsistent only on start-up (software-related issue)
  - Solution: Send an initial dummy message when starting the transceiver
- Debounce on rotary encoder push-button is still somewhat inconsistent
  - Solution: Debounce capacitor C3 on the schematic may perform better at 100nF instead of 10nF. Further testing is required.
- Establishing LoRa communication is inconsistent
  - Both modules consistently send data over LoRa with the SoftwareSerial.h library, but receiveing messages has proved unreliable.
    - Could potentially be a hardware related issue, with either the Arduinos themsevles, or the RYLR896 LoRa module. Further testing required.

### Potential Improvements going forward...
- Use an MCU with more memory (i.e. ESP32)
- Create smaller device footprint by creating custom PCB to contain essential electronics
- Use more sophisticated debouce methodology
- Create a dedicated housing in Fusion 360
- Use a higher memory microcontroller architecture to reduce the need for several MCUs
