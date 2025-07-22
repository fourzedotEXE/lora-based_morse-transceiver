//Secondary - Controls the LoRa RYLR896 module and small OLED
//FourzeDotEXE - 5/22/25

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

SoftwareSerial lora(2, 3); // RX, TX for RYLR896

//Randomly generated addresses
const int DEST_ADDR = 27597;  // Change this to the address of the remote node
const int SELF_ADDR = 47575;  // This device's address

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

void setup() {
  //set baud
  Serial.begin(9600);

  //set lora serial baud
  lora.begin(115200);

  // Initialize the display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for most OLEDs
    Serial.println(F("SSD1306 allocation failed"));
    while(1);
  }

  //Initialize LoRa
  lora.println("AT+ADDRESS=" + String(SELF_ADDR));
  delay(100);
  lora.println("AT+NETWORKID=5");
  delay(100);
  lora.println("AT+BAND=915000000");  //in line w/ FCC standards
  delay(100);

  Serial.println("LoRa initialized");

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

  // Sending data over LoRa
  if (Serial.available()) { //first fetch data to be sent from serial input
    message += Serial.readStringUntil('\n');
    message.trim();
    String command = "";


    //generate command to send the message via RYLR896
    if (message.length() > 0){

      String final_payload = morse_translate(message);
      command = "AT+SEND=" + String(DEST_ADDR) + "," + String(final_payload.length()) + "," + final_payload;
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

    if (message.length() > 0){
      lora.println(command);
    }

    display.display(); //display update
    message = "";
  }

  delay(1000);  //induce delay to minimize data losses

}
