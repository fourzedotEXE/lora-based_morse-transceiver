/*
 ______     __  __     ______     ______    
/\  __ \   /\ \_\ \   /\  __ \   /\  == \   
\ \  __ \  \ \  __ \  \ \  __ \  \ \  __<   
 \ \_\ \_\  \ \_\ \_\  \ \_\ \_\  \ \_____\ 
  \/_/\/_/   \/_/\/_/   \/_/\/_/   \/_____/ 
          Secondary // FourzeDotEXE
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LoRa.h>
#include <AESLib.h>

#define LORA_FREQ 915E6     // SX1278 frequency
#define SERIAL_BAUD 9600    // Serial baud rate
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // Change to 64 if you have a 128x64 display
#define INPUT_BUFFER_LIMIT (128 + 1) // designed for Arduino UNO, not stress-tested anymore (this works with readBuffer[129])


#define MORSE_TABLE_LEN (38)
#define MORSE_WORD_LEN (6)
#define MORSE_TRANSMIT 4
#define BUZZER 6

#define MAX_LINES (SCREEN_HEIGHT / 8) // 8 pixels per line for size 1 font
String lines[MAX_LINES];
int lineIndex = 0;

// Create an SSD1306 display object connected to I2C (default address 0x3C)
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// morse characters
char morseTable[MORSE_TABLE_LEN][MORSE_WORD_LEN] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..",     // A-I
  ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.",    // J-R
  "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",            // S-Z
  ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.", "-----", //1-0
  ".-.-.-" //period
};

char morseMap[MORSE_TABLE_LEN]="ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890.";

String To_Morse="";

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
  //start up procedure
  Serial.begin(SERIAL_BAUD);
  Serial.setTimeout(60000);
  delay(2000);

  while (!Serial);

  // Initialize the display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for most OLEDs
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  //Initialize LoRa
  Serial.println("LoRa Nano Two-Way Chat");

  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  //long range settings
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);

  Serial.println("LoRa Init OK");
  aes_init(); // generate random IV, should be called only once? causes crash if repeated...

  Serial.println("Type a message and press ENTER to send.");

  display.clearDisplay();

  // Display text
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 0);            // Start at top-left corner

  display.display();                  // Actually display everything we just wrote
}

/* non-blocking wait function */
void wait(unsigned long milliseconds) {
  unsigned long timeout = millis() + milliseconds;
  while (millis() < timeout) {
    yield();
  }
}

//function that translates the inputted text to morse code
String morse_translate(String input){
    String build_morse = "";
    for (int i = 6; i < input.length(); i++) {
      char c = input.charAt(i);

      for (int j = 0;j < MORSE_TABLE_LEN;j++){
        if (c == morseMap[j]){
           Serial.print(morseTable[j]);
           build_morse += morseTable[j];
           build_morse += " ";
           Serial.print(" ");
        } else if (c == ' ') {
          Serial.print(" / ");  // Slash for space between words
        } else {
          //Serial.print("\n ");
        }
      }
    }
    Serial.println();  // End of Morse output
    return build_morse;
}

//function that translates the inbound morse character to english text
String morse_reverse(String input){
  String build_text = "";
  int x = 0;
  int y = 0;
  for (x = 0; x <= input.length(); x++){
    if ((input.charAt(x) == ' ' || x == input.length()) && x >0){
      Serial.println(input.substring(y, x));
      for (int i = 0; i < MORSE_TABLE_LEN; i++) {
        if (input.substring(y, x) == morseTable[i]){
          build_text += (char)morseMap[i];
          //Serial.println(morseMap[i]);
        }
      }
      y = x+1;
    }
  }
  return build_text;
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
  Serial.println((char*)readBuffer);

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

String message = "";
String received = "Pequod: ";
String analyze_morse = " ";

void loop() {

  // Receiving
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    while (LoRa.available()) {
        analyze_morse += ((char)LoRa.read());
    }

    received += morse_reverse(analyze_morse);

    // Scroll existing lines up
    for (int i = 0; i < MAX_LINES - 1; i++) {
      lines[i] = lines[i + 1];
    }

    lines[MAX_LINES - 1] = received;  //add new text to the screen

    // Redraw screen
    display.clearDisplay();

    for (int i = 0; i < MAX_LINES; i++) {
      display.setCursor(0, i * 8);
      display.println(lines[i]);
    }

    display.display(); //display update

    received = "Pequod: ";
    analyze_morse = "";
  }

  
  // Sending
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      if (message.length() > 0) {
        LoRa.beginPacket();

        //add encryption layer here
        LoRa.print(morse_translate(message));
        LoRa.endPacket();
      }

      String build_disp_message = "Ahab: " + message;
      Serial.println(build_disp_message + message);

      // Scroll existing lines up
      for (int i = 0; i < MAX_LINES - 1; i++) {
        lines[i] = lines[i + 1];
      }

      lines[MAX_LINES - 1] = message;  //add new text to the screen

      // Redraw screen
      display.clearDisplay();

      for (int i = 0; i < MAX_LINES; i++) {
        display.setCursor(0, i * 8);
        display.println(lines[i]);
      }

      display.display(); //display update
      message = "";
    } else {
      message += c;
    }
  }

  delay(1000);  //induce delay to minimize data losses

}
