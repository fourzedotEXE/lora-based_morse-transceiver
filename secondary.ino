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

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // Change to 64 if you have a 128x64 display

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

  // Initialize the display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for most OLEDs
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();

  // Display text
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 0);            // Start at top-left corner

  
  //display.println(F("Ahab: Hello, Fuckers!"));
  //display.println(F("Pequod: This is Fucker #1, over!"));


  display.display();                  // Actually display everything we just wrote
}

//function that translates the inputted text to morse code
void morse_translate(String input){
    for (unsigned int i = 6; i < input.length(); i++) {
      char c = input.charAt(i);

      for (int j = 0;j < MORSE_TABLE_LEN;j++){
        if (c == morseMap[j]){
           Serial.print(morseTable[j]);
        } else if (c == ' ') {
          Serial.print(" / ");  // Slash for space between words
        } else {
          //Serial.print("\n ");
        }
      }
    }
    Serial.println();  // End of Morse output
}

void loop() {
  if (Serial.available()) {
    String message = Serial.readStringUntil('\n');
    Serial.println("Received: " + message);
  

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
    morse_translate(message);
  }
  delay(1000);

}
