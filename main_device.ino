/* ________  _______   ________  ___  ___  ________  ________     
|\   __  \|\  ___ \ |\   __  \|\  \|\  \|\   __  \|\   ___ \    
\ \  \|\  \ \   __/|\ \  \|\  \ \  \\\  \ \  \|\  \ \  \_|\ \   
 \ \   ____\ \  \_|/_\ \  \\\  \ \  \\\  \ \  \\\  \ \  \ \\ \  
  \ \  \___|\ \  \_|\ \ \  \\\  \ \  \\\  \ \  \\\  \ \  \_\\ \ 
   \ \__\    \ \_______\ \_____  \ \_______\ \_______\ \_______\
    \|__|     \|_______|\|___| \__\|_______|\|_______|\|_______|
                              \|__|                             
                  Main Device (Hook up to PC)  
*/                                             
#include <SPI.h>
#include <LoRa.h>
#include <AESLib.h>

#define LORA_FREQ 915E6     // SX1278 frequency
#define SERIAL_BAUD 9600    // Serial baud rate
#define INPUT_BUFFER_LIMIT 128 // designed for Arduino UNO, not stress-tested anymore (this works with readBuffer[129])

unsigned char cleartext[INPUT_BUFFER_LIMIT] = {0}; // THIS IS INPUT BUFFER (FOR TEXT)
unsigned char ciphertext[2*INPUT_BUFFER_LIMIT] = {0}; // THIS IS OUTPUT BUFFER (FOR BASE64-ENCODED ENCRYPTED DATA)

// AES Encryption Key (YOU MUST GENERATE YOUR OWN FOR SECURITY)
byte aes_key[] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };

// General initialization vector (same as in node-js example) (you must use your own IV's in production for full security!!!)
byte aes_iv[N_BLOCK] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };

AESLib aesLib;

// Generate IV (once)
void aes_init() {
  aesLib.gen_iv(aes_iv);
  aesLib.set_paddingmode((paddingMode)0);
}

uint16_t encrypt_to_ciphertext(char * msg, uint16_t msgLen, byte iv[]) {
  Serial.println("Calling encrypt (string)...");
  // aesLib.get_cipher64_length(msgLen);
  int cipherlength = aesLib.encrypt((byte*)msg, msgLen, (byte*)ciphertext, aes_key, sizeof(aes_key), iv);
                   // uint16_t encrypt(byte input[], uint16_t input_length, char * output, byte key[],int bits, byte my_iv[]);
  return cipherlength;
}

uint16_t decrypt_to_cleartext(byte msg[], uint16_t msgLen, byte iv[]) {
  Serial.print("Calling decrypt...; ");
  uint16_t dec_bytes = aesLib.decrypt(msg, msgLen, (byte*)cleartext, aes_key, sizeof(aes_key), iv);
  Serial.print("Decrypted bytes: "); Serial.println(dec_bytes);
  return dec_bytes;
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.setTimeout(60000);
  delay(2000);

  while (!Serial);

  //Initialize LoRa
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
  aes_init(); // generate random IV, should be called only once? causes crash if repeated...

  Serial.println("Type a message and press ENTER to send.");
}

/* non-blocking wait function */
void wait(unsigned long milliseconds) {
  unsigned long timeout = millis() + milliseconds;
  while (millis() < timeout) {
    yield();
  }
}

// Working IV buffer: Will be updated after encryption to follow up on next block.
// But we don't want/need that in this test, so we'll copy this over with enc_iv_to/enc_iv_from
// in each loop to keep the test at IV iteration 1. We could go further, but we'll get back to that later when needed.

// General initialization vector (same as in node-js example) (you must use your own IV's in production for full security!!!)
byte enc_iv[N_BLOCK] =      { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };
byte enc_iv_to[N_BLOCK]   = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };
byte enc_iv_from[N_BLOCK] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };

uint16_t encLen = 0;

//encrypt the contents of readBuffer using the shared key and iv
byte encryption_layer(unsigned char* readBuffer, int len){
  Serial.print("readBuffer length: "); Serial.println(sizeof(readBuffer));
  Serial.print("readBuffer contents: "); Serial.println((char*)readBuffer);

  // must not exceed INPUT                                                    9_BUFFER_LIMIT bytes; may contain a newline
  sprintf((char*)cleartext, "%s", readBuffer);

  // Encrypt
  // iv_block gets written to, provide own fresh copy... so each iteration of encryption will be the same.
  uint16_t msgLen = sizeof(readBuffer);
  memcpy(enc_iv, enc_iv_to, sizeof(enc_iv_to));
  encLen = encrypt_to_ciphertext((char*)cleartext, msgLen, enc_iv);
  Serial.print("Encrypted length = "); Serial.println(encLen );

  Serial.print("Encrypted contents: ");
  Serial.println((char*)ciphertext);

  //free memory
  Serial.println("---");
  memset(readBuffer, 0, len+1);
  free(readBuffer);
  readBuffer = NULL;  // Optional but good practice to avoid dangling pointer
}

//decrypt the contents of readBuffer using the shared key and iv
void decryption_layer(unsigned char* readBuffer, int len){
  //decrypt
  Serial.println("Encrypted. Decrypting..."); Serial.println(encLen ); Serial.flush();
  
  unsigned char base64decoded[50] = {0};
  base64_decode((char*)base64decoded, (char*)ciphertext, encLen);
  
  memcpy(enc_iv, enc_iv_from, sizeof(enc_iv_from));
  uint16_t decLen = decrypt_to_cleartext(base64decoded, strlen((char*)base64decoded), enc_iv);
  Serial.print("Decrypted cleartext of length: "); Serial.println(decLen);
  Serial.print("Decrypted cleartext:\n"); Serial.println((char*)cleartext);

  //compare decrypt integrity
  if (strcmp((char*)readBuffer, (char*)cleartext) == 0) {
    Serial.println("Decrypted correctly.");
  } else {
    Serial.println("Decryption test failed.");
  }

  //free memory
  Serial.println("---");
  memset(readBuffer, 0, len+1);
  free(readBuffer);
  readBuffer = NULL;  // Optional but good practice to avoid dangling pointer
}

String inputText = "";

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
    if (c == '\n') {
      if (inputText.length() > 0) {
        wait(1);
        Serial.println(inputText);

        //copy string message data over to the encryption buffer
        int len = inputText.length();
        unsigned char* readBuffer = (unsigned char*)malloc(len+1);
        if (readBuffer != NULL) {
          strcpy((char*)readBuffer, inputText.c_str());  // Copy string content
        }
        encryption_layer(readBuffer, len);

        //call lora write here (write the ciphertext, not the readBuffer)
        LoRa.beginPacket();
        LoRa.write((byte*)ciphertext, sizeof(ciphertext));
        LoRa.endPacket();

        inputText = "";
      }
    } else {
      inputText += c;
    }
  }
}
