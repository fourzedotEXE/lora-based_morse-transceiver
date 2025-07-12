/*
 ______     __  __     ______     ______    
/\  __ \   /\ \_\ \   /\  __ \   /\  == \   
\ \  __ \  \ \  __ \  \ \  __ \  \ \  __<   
 \ \_\ \_\  \ \_\ \_\  \ \_\ \_\  \ \_____\ 
  \/_/\/_/   \/_/\/_/   \/_/\/_/   \/_____/ 
          Secondary // FourzeDotEXE
*/

/*
THIS IS AN UNFINISHED, UNTESTED VERSION OF THE SECONDARY ARDUINO CODE.
THIS CODE DOES NOT RUN ON ARDUINO, BUT CONTAINS THE CODE FOR ENCRYPTION AND
DECRYPTION OF MESSAGES
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AESLib.h>
#include <SoftwareSerial.h>

#define SERIAL_BAUD 9600    // Serial baud rate
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // Change to 64 if you have a 128x64 display
#define INPUT_BUFFER_LIMIT (128 + 1)

SoftwareSerial lora(2, 3); // RX, TX for RYLR896

//Randomly generated addresses
const int DEST_ADDR = 27597;  // Change this to the address of the remote node
const int SELF_ADDR = 47575;  // This device's address

unsigned char cleartext[INPUT_BUFFER_LIMIT] = {0}; // THIS IS INPUT BUFFER (FOR TEXT)
unsigned char ciphertext[2*INPUT_BUFFER_LIMIT] = {0}; // THIS IS OUTPUT BUFFER (FOR BASE64-ENCODED ENCRYPTED DATA)

// AES Encryption Key (YOU MUST GENERATE YOUR OWN FOR SECURITY)
byte aes_key[] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };

// General initialization vector (same as in node-js example) (you must use your own IV's in production for full security!!!)
byte aes_iv[N_BLOCK] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };

AESLib aesLib;

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
  //set baud
  Serial.begin(9600);

  //set lora serial baud
  lora.begin(115200);

  // Initialize the display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for most OLEDs
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  //Initialize LoRa
  lora.println("AT+ADDRESS=" + String(SELF_ADDR));
  delay(100);
  lora.println("AT+NETWORKID=5");
  delay(100);
  lora.println("AT+BAND=915000000");  //in line w/ FCC standards
  delay(100);

  Serial.println("LoRa initialized");

  aes_init(); // generate random IV, should be called only once? causes crash if repeated...

  //initialize secondary display
  display.clearDisplay();

  // Display text
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 0);            // Start at top-left corner

  display.display();                  // Actually display everything we just wrote
}

//function that translates the inputted text to morse code
String morse_translate(String input){
    String build_morse = "";
    for (int i = 6; i < input.length(); i++) {
      char c = input.charAt(i);

      for (int j = 0;j < MORSE_TABLE_LEN;j++){
        if (c == morseMap[j]){
           //Serial.print(morseTable[j]);
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

//function that translates the inbound morse character to text
String morse_reverse(String input){
  String build_text = "";
  int x = 0;
  int y = 0;
  for (x = 0; x <= input.length(); x++){
    if ((input.charAt(x) == ' ' || x == input.length()) && x >0){
      //Serial.println(input.substring(y, x));
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

/*
//encrypt the contents of readBuffer using the shared key and iv
String encryption_layer(unsigned char* readBuffer, int len){
  //Serial.print("readBuffer length: "); Serial.println(sizeof(readBuffer));
  //Serial.print("readBuffer contents: "); Serial.println((char*)readBuffer);

  // must not exceed INPUT                                                    9_BUFFER_LIMIT bytes; may contain a newline
  sprintf((char*)cleartext, "%s", readBuffer);

  // Encrypt
  // iv_block gets written to, provide own fresh copy... so each iteration of encryption will be the same.
  uint16_t msgLen = sizeof(readBuffer);
  memcpy(enc_iv, enc_iv_to, sizeof(enc_iv_to));
  encLen = encrypt_to_ciphertext((char*)cleartext, msgLen, enc_iv);
  //Serial.print("Encrypted length = "); Serial.println(encLen );

  //Serial.print("Encrypted contents: ");
  //Serial.println((char*)ciphertext);

  String hex_input = byteArrayToHex(readBuffer, len);

  //Serial.print("Hex encrypted contents: ");
  //Serial.println(hex_input);

  //free memory
  //Serial.println("---");
  memset(readBuffer, 0, len+1);
  free(readBuffer);
  readBuffer = NULL;  // Optional but good practice to avoid dangling pointer

  return hex_input;
}

//decrypt the contents of readBuffer using the shared key and iv

String decryption_layer(String hex_data){
  //decode byte data from hex
  int len = hex_data.length();
  int out_len = 0;
  if (len%2 == 0){
    out_len = 0;
    return ""; 
  }
  //1 byte == 2 hex chars
  out_len = len/2;
  //allocate appropriate amount of data for byte string
  byte* readBuffer = (byte*)malloc(out_len);
  readBuffer = hexToByteArray(hex_data, readBuffer, out_len);

  //decrypt ciphertext
  //Serial.println("Encrypted. Decrypting..."); Serial.println(encLen ); Serial.flush();
  
  unsigned char base64decoded[50] = {0};
  base64_decode((char*)base64decoded, (char*)ciphertext, encLen);
  
  memcpy(enc_iv, enc_iv_from, sizeof(enc_iv_from));
  uint16_t decLen = decrypt_to_cleartext(base64decoded, strlen((char*)base64decoded), enc_iv);
  //Serial.print("Decrypted cleartext of length: "); Serial.println(decLen);
  //Serial.print("Decrypted cleartext:\n"); Serial.println((char*)cleartext);

  //free hex decoded bytes
  //Serial.println("---");
  memcpy(readBuffer, 0, out_len+1);   //MIGHT NEED TO DELETE IF PROGRAM CRASHES
  free(readBuffer);
  readBuffer = NULL;
}
*/

String byteArrayToHex(const byte* data, int len) {
  String hex = "";
  for (int i = 0; i < len; i++) {
    if (data[i] < 16) hex += "0";
    hex += String(data[i], HEX);
  }
  return hex;
}

byte* hexToByteArray(String hex_data, byte* hex_decode, int out_len){
  //loop over each 2 hex chars
  for (int i = 0; i < out_len; i++) {
    String byte_as_string = hex_data.substring((2*i), (2*i+2));  //ex: i = 1  -> hex_data.substring(2, 4) = 0xA4

    //to convert to byte, use .c_str(), and then convert that to an unsigned long
      //strtoul is only compatible with c-style strings
    //then cast the long to a byte and add that to the proper index in the array
    hex_decode[i] = (byte)strtoul(byte_as_string.c_str(), nullptr, 16);  //add the newly decoded byte to the array
  }

  return hex_decode;
}

String message = "";
String received = "Peqd: ";
String incoming = " ";

void loop() {
  
  // Receiving messages over LoRa
  if (lora.available()) {
    incoming = lora.readStringUntil('\n');
    //Serial.println("Peqd: " + incoming);

    //String morse_data = decryption_layer(incoming);

    received += morse_reverse(incoming);

    //Scroll existing lines up
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

    received = "Peqd: ";
  }

    //-------------------------------------------------

  // Sending data over LoRa
  if (Serial.available()) { //first fetch data to be sent from serial input
    message += lora.readStringUntil('\n');
    message.trim();

    //generate command to send the message via RYLR896
    if (message.length() > 0){

      /*
       //copy string message data over to the encryption buffer
      int len = message.length();
      unsigned char* readBuffer = (unsigned char*)malloc(len+1);
      if (readBuffer != NULL) {
        strcpy((char*)readBuffer, message.c_str());  // Copy string content
      }
      */

      //note that the encrpytion later first performs a byte encode, and then a hex encode of those bytes
      //String final_payload = encryption_layer(readBuffer, len);

      String final_payload = morse_translate(message);
      String command = "AT+SEND=" + String(DEST_ADDR) + "," + String(final_payload.length()) + "," + final_payload;
      lora.println(command);
      Serial.println("Ahab: " + message);
    }

    // Scroll existing lines up
    for (int i = 0; i < MAX_LINES - 1; i++) {
      lines[i] = lines[i + 1];
    }

    lines[MAX_LINES - 1] = ("Ahab: " + message);  //add new text to the screen

    // Redraw screen
    display.clearDisplay();

    for (int i = 0; i < MAX_LINES; i++) {
      display.setCursor(0, i * 8);
      display.println(lines[i]);
    }

    display.display(); //display update
    message = "";
  }

  delay(1000);  //induce delay to minimize data losses

}
