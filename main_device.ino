 ________  _______   ________  ___  ___  ________  ________     
|\   __  \|\  ___ \ |\   __  \|\  \|\  \|\   __  \|\   ___ \    
\ \  \|\  \ \   __/|\ \  \|\  \ \  \\\  \ \  \|\  \ \  \_|\ \   
 \ \   ____\ \  \_|/_\ \  \\\  \ \  \\\  \ \  \\\  \ \  \ \\ \  
  \ \  \___|\ \  \_|\ \ \  \\\  \ \  \\\  \ \  \\\  \ \  \_\\ \ 
   \ \__\    \ \_______\ \_____  \ \_______\ \_______\ \_______\
    \|__|     \|_______|\|___| \__\|_______|\|_______|\|_______|
                              \|__|                             
                //Main Device (Hook up to PC)//                                               
#include <SPI.h>
#include <LoRa.h>

#define LORA_FREQ 433E6     // SX1278 frequency
#define SERIAL_BAUD 9600    // Serial baud rate

String inputText = "";

void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial);

  Serial.println("LoRa Two-Way Chat");

  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // Optional tuning
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);

  Serial.println("LoRa Init OK");
  Serial.println("Type a message and press ENTER to send.");
}

void loop() {
  // Check if a message was received
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }
    Serial.println();
  }

  // Check if user typed something
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (inputText.length() > 0) {
        // Send the message over LoRa
        LoRa.beginPacket();
        LoRa.print(" " + inputText);
        LoRa.endPacket();

        Serial.println(inputText);

        inputText = ""; // clear buffer
      }
    } else {
      inputText += c;
    }
  }
}
