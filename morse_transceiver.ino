/*
 ______     __  __     ______     ______    
/\  __ \   /\ \_\ \   /\  __ \   /\  == \   
\ \  __ \  \ \  __ \  \ \  __ \  \ \  __<   
 \ \_\ \_\  \ \_\ \_\  \ \_\ \_\  \ \_____\ 
  \/_/\/_/   \/_/\/_/   \/_/\/_/   \/_____/ 
          Transceiver // FourzeDotEXE
*/
#include <avr/interrupt.h>  //lib for Pin Change Interrupts
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13

#define FRAME_DELAY (42)
#define FRAME_WIDTH (128)
#define FRAME_HEIGHT (64)

#define MORSE_TABLE_LEN (38)
#define MORSE_WORD_LEN (6)
#define MORSE_TRANSMIT 4
#define BUZZER 6

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

int pulses=0, pulses_prev=0, A_SIG=0, B_SIG=1;
int cursor_map;
int buttonState = 0; //flag to indicate if the encoder button is pressed (inputs a letter into the buffer)
bool transmit_now = 0; //flag to indicate when transmit should occur (uses morse_translate function and clears the input buffer)

// Variables capturing current and newly calculated position on the letter board (Values 0-26)
int New_Position=0;
int Old_Position=0;

// Flag indicating the letter was entered on the letter board to be added to curr_text string
int Letter_Entered=0;
int Transmit=0;
String To_Transmit="";
String To_Morse="";

//track current keyboard in use (mode 0-3)
bool keyboard = 0;

//track current number of characters inputted
int char_counter = 0;

// Used for displaying Leter board (mode 0-3)
char Letters[28]="ABCDEFGHIJKLMNOPQRSTUVWXYZ#";
char Numbers[28]="1234567890.               #";
char u_Letters[28]="abcdefghijklmnopqrstuvwxyz|";
char a_Symbols[28]="1234567890!?,.@#$%^&*()_-+|";
char b_Symbols[28]=":;=~`{}[]\/<>             |";

// morse characters
char morseTable[MORSE_TABLE_LEN][MORSE_WORD_LEN] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..",     // A-I
  ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.",    // J-R
  "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",            // S-Z
  ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.", "-----", //1-0
  ".-.-.-" //period
};

char morseMap[MORSE_TABLE_LEN]="ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890.";

const int dot_time = 200;
const int dash_time = 600;

void setup() {
  Serial.begin(9600);
  pulses = 4;

  //define pinmode for morse transmit button and buzzer
  pinMode(MORSE_TRANSMIT, INPUT);
  pinMode(BUZZER, OUTPUT);
  //digitalWrite(BUZZER, LOW);

  //digital pin 2 and 3 serve as encoder interrupts
  attachInterrupt(0, A_RISE, RISING);
  attachInterrupt(1, B_RISE, RISING);
  
  //Pin Change Interrupt on pin 5 (PCINT21)
  cli();
  PCICR |= 0b00000100;    // turn on port d (PCINT[16-23])
  PCMSK2 |= 0b00110000;   //Interrupt on Pin 5 and Pin 4 (6th and 5th MSB bit respectively)
  sei();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  //------------------------------------------------------------
  //initialize SPI OLED
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  delay(2000); // Pause for 2 seconds
  
  //display.display();

  // Clear the buffer
  display.clearDisplay();
  
  
  // Display Letter Board 3 rows 9 character in each row 
  display.setTextSize(2);            
  display.setTextColor(SSD1306_WHITE);       
  for (int j=0; j<3;j++){
    for (int i=0; i<9;i++){
      display.setCursor(i*12+2*i+1,j*16+17);           
      display.println(Letters[i+j*9]);
      //display.fillRect(i*12+2*i, j*16 +16, 12, 16, SSD1306_INVERSE);
      display.display();
    }
  }
  // Display filled in rect in the top section of the display when To_Transfer would be output
  display.fillRect(0, 0, 128, 15, SSD1306_INVERSE);
  // Highlight character A by displaying Inverse rect at first position
  display.fillRect(0, 16, 12, 16, SSD1306_INVERSE);

  //display.display();
  //------------------------------------------------------------
}

void Add_To_String(){
  Letter_Entered=1;
}

//highlight the selected letter on the keyboard
void Highlight_letter(int New_Pos, int Old_Pos){
  // When position changes from Old_Pos to New_Pos
  // Draw the inverse rect in the Old_pos to deactivate  the highlight in the old spot
  // Draw the inverse rect to Highlite the new spot
  int X_pos;
  int Y_pos;
  
  // Calculate X and Y coordinates of the New_Pos on the letter board
  X_pos=New_Pos - ((int)New_Pos/9)*9;
  Y_pos=(int)New_Pos/9;
  // Displaying Inverse rect
  display.fillRect(X_pos*12+2*X_pos, Y_pos*16 +16, 12, 16, SSD1306_INVERSE);
  
  // Calculate X and Y coordinates of the Old_Pos on the letter board
  X_pos=Old_Pos - ((int)Old_Pos/9)*9;
  Y_pos=(int)Old_Pos/9;
  // Displaying Inverse rect
  display.fillRect(X_pos*12+2*X_pos, Y_pos*16 +16, 12, 16, SSD1306_INVERSE);
  
  display.display();
}

//update the keyboard to the correct selected mode
void update_keys(bool mode_selected){
  display.clearDisplay();

  display.setTextSize(2);            
  display.setTextColor(SSD1306_WHITE);       
  for (int j=0; j<3;j++){
    for (int i=0; i<9;i++){
      display.setCursor(i*12+2*i+1,j*16+17);
      switch(mode_selected){
        case 0:
          display.println(Letters[i+j*9]);
          break;
        case 1:
          display.println(Numbers[i+j*9]);
          break;
        default:
          display.println(Letters[i+j*9]);
          break;
      }
      display.display();
    }
  }

  // Display filled in rect in the top section of the display when To_Transfer would be output
  display.fillRect(0, 0, 128, 15, SSD1306_INVERSE);
}
void A_RISE(){
 detachInterrupt(0);
 A_SIG=1;
 
 pulses_prev = pulses; //capture previous pulse count
 if(B_SIG==0)
 pulses++;//moving forward
 if(B_SIG==1)
 pulses--;//moving reverse
 Serial.println(pulses);
 attachInterrupt(0, A_FALL, FALLING);
}

void A_FALL(){
 detachInterrupt(0);
 A_SIG=0;
 
 pulses_prev = pulses; //capture previous pulse count
 if(B_SIG==1)
 pulses++;//moving forward
 if(B_SIG==0)
 pulses--;//moving reverse
 Serial.println(pulses);
 attachInterrupt(0, A_RISE, RISING);  
}

void B_RISE(){
 detachInterrupt(1);
 B_SIG=1;
 
 pulses_prev = pulses; //capture previous pulse count
 if(A_SIG==1)
 pulses++;//moving forward
 if(A_SIG==0)
 pulses--;//moving reverse
 Serial.println(pulses);
 attachInterrupt(1, B_FALL, FALLING);
}

void B_FALL(){
 detachInterrupt(1);
 B_SIG=0;
 
 pulses_prev = pulses; //capture previous pulse count
 if(A_SIG==0)
 pulses++;//moving forward
 if(A_SIG==1)
 pulses--;//moving reverse
 Serial.println(pulses);
 attachInterrupt(1, B_RISE, RISING);
}


/*
ISR(PCINT2_vect){
  PCIFR |= (1 << PCIF2);  // Clear the interrupt flag
  if (buttonState == 0){
    Serial.println("BUTTON PRESSED");
    Add_To_String();
    buttonState = 1;
    Letter_Entered = 1;
  } else {
    buttonState = 0;
    Letter_Entered = 0;
  }
}    // Port D, PCINT16 - PCINT23
*/

volatile bool button4Pressed = false;
volatile bool button5Pressed = false;
const int debounceDelay = 50; // ms
unsigned long lastInterruptTime4 = 0;
unsigned long lastInterruptTime5 = 0;

ISR(PCINT2_vect) {
  unsigned long currentTime = millis();
  if ((currentTime - lastInterruptTime4 > debounceDelay)) {
    bool state4 = digitalRead(4);
    if (state4 == LOW && !button4Pressed) {
      Serial.println("TRANSMISSION");
      transmit_now = 1;
      //morse_translate(To_Transmit);
      //delay(dot_time);
      button4Pressed = 1;
     } else if (state4 == HIGH && button4Pressed) {
       button4Pressed = 0;
    }
     lastInterruptTime4 = currentTime;
  }

   // Pin 5 logic
   if ((currentTime - lastInterruptTime5 > debounceDelay)) {
     bool state5 = digitalRead(5);
     if (state5 == LOW && !button5Pressed) {
      Serial.println("BUTTON PRESSED");
      Add_To_String();
      buttonState = 1;
      Letter_Entered = 1;
      button5Pressed = 1;
     } else if (state5 == HIGH && button5Pressed) {
      button5Pressed = 0;
    }
    lastInterruptTime5 = currentTime;
  }
}

//function that translates the inputted text to morse code
/*
void morse_translate(String input){
  int string_length = input.length();
  
  for(int i=0;i<string_length;i++){
    for(int j=0;j<MORSE_TABLE_LEN;j++){
      if(morseMap[j] == input[i]){  //check if the letters match
        for(int x=0;x<MORSE_WORD_LEN;x++){

          if(morseTable[j][x] == '.') {

            digitalWrite(BUZZER, HIGH);
            delay(dot_time);

          } else if (morseTable[j][x] == '-') {

            digitalWrite(BUZZER, HIGH);
            delay(dash_time);

          }
          digitalWrite(BUZZER, LOW);
          delay(dot_time);
        }
      }
    }
    delay(dash_time);  //space between letter inputs are 600ms
  }
}*/

//do loop
void loop() {
  //apply limits to the encoder scrolling through the keyboard
  if (pulses < -4){
    pulses = -4;
  }
  if (pulses > 100){
    pulses = 100;
  }

  //use a map function to advance the cursor forward or backward per 1 tick of the encoder
  if(pulses_prev != pulses){
    if (pulses == 100) {
      New_Position = 26;
    } else if (pulses == -4){
      New_Position = 0;
    } else {
      New_Position = map(pulses,-4,100,0,26);
    }
    Highlight_letter(New_Position,Old_Position);
    Old_Position = New_Position;
  }

  delay(5);

  //debugging statements
  Serial.print("LetterEntered: \n");
  Serial.println(Letter_Entered);

  Serial.print("String: \n");
  Serial.println(To_Transmit);

  // If letter was entered add the letter to To_Transmit and output it on the display
  if (Letter_Entered==1){
    if (Letters[New_Position] == '#'){
      Serial.println("SWITCHING KEYBOARDS");
      keyboard = !(keyboard);
      update_keys(keyboard);

      display.setCursor(3,0);
      display.setTextColor(BLACK);
      display.fillRect(0, 0, 128, 15, SSD1306_WHITE);
      display.println(To_Transmit);
      display.display();

    } else {

      if (keyboard == 0){
        To_Transmit=To_Transmit + Letters[New_Position];
      } else {
        To_Transmit=To_Transmit + Numbers[New_Position];
      }

      char_counter++;
      Serial.print("char_counter: \n");
      Serial.print(char_counter);

      display.setCursor(3,0);
      display.setTextColor(BLACK);
      display.fillRect(0, 0, 128, 15, SSD1306_WHITE);
      
      //scroll password to the left if the string size exceeds 10
      if (char_counter > 10){
        //display.println(To_Transmit[char_counter]);
      } else {
        display.println(To_Transmit);
      }
      display.display();
    }
  } else if (transmit_now == 1) {

    //------------------------------------------
    for(int i=0;i<char_counter;i++){
      for(int j=0;j<MORSE_TABLE_LEN;j++){
        if(morseMap[j] == To_Transmit[i]){  //check if the letters match
          for(int x=0;x<MORSE_WORD_LEN;x++){

            if(morseTable[j][x] == '.') {
              //To_Morse + '.';
            } else if (morseTable[j][x] == '-') {
              //To_Morse + '-';
            } else {
              //To_Morse + '-';
            }
          }
        }
      }
      delay(dash_time);  //space between letter inputs are 600ms
    }
    To_Transmit="";

    //------------------------------------------
    transmit_now = 0;
    char_counter=0;
    Serial.println("PLAYING TRANSMISSION");
    display.display();
  }

  Letter_Entered=0;
  delay(100);
}
