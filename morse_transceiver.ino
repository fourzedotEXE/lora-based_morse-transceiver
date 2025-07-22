//Transceiver - Controls the rotary encoder and big OLED
//FourzeDotEXE - 5/22/25

#include <avr/interrupt.h>  //lib for Pin Change Interrupts
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define MAX_VISIBLE_CHARS 10 //maximum number of inputted chars shown on the screen at any time

#define MORSE_TRANSMIT 4
#define BUZZER 6

#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13

#define FRAME_DELAY (42)
#define FRAME_WIDTH (128)
#define FRAME_HEIGHT (64)

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

const int dot_time = 200;
const int dash_time = 600;

void setup() {
  Serial.begin(9600);
  pulses = 4;

  pinMode(MORSE_TRANSMIT, INPUT);
  pinMode(BUZZER, OUTPUT);

  attachInterrupt(0, A_RISE, RISING);
  attachInterrupt(1, B_RISE, RISING);
  
  //Pin Change Interrupt on pin 5 (PCINT21)
  cli();
  PCICR |= 0b00000100;    // turn on port d (PCINT[16-23])
  PCMSK2 |= 0b00110000;   //Interrupt on Pin 5 and Pin 4 (6th and 5th MSB bit respectively)
  sei();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while(1);
  }

  delay(2000);
  display.clearDisplay();
  
  // Display Letter Board 3 rows 9 character in each row 
  display.setTextSize(2);            
  display.setTextColor(SSD1306_WHITE);       
  for (int j=0; j<3;j++){
    for (int i=0; i<9;i++){
      display.setCursor(i*12+2*i+1,j*16+17);           
      display.println(Letters[i+j*9]);
      display.display();
    }
  }

  // Display filled in rect in the top section of the display when To_Transfer would be output
  display.fillRect(0, 0, 128, 15, SSD1306_INVERSE);
  // Highlight character A by displaying Inverse rect at first position
  display.fillRect(0, 16, 12, 16, SSD1306_INVERSE);
}

//highlight the selected letter on the keyboard
void Highlight_letter(int New_Pos, int Old_Pos){
  int X_pos;
  int Y_pos;
  
  // Calculate X and Y coordinates of the New_Pos on the letter board
  X_pos=New_Pos - ((int)New_Pos/9)*9;
  Y_pos=(int)New_Pos/9;
  // Displaying Inverse rect over the current curson positon
  display.fillRect(X_pos*12+2*X_pos, Y_pos*16 +16, 12, 16, SSD1306_INVERSE);
  
  // Calculate X and Y coordinates of the Old_Pos on the letter board
  X_pos=Old_Pos - ((int)Old_Pos/9)*9;
  Y_pos=(int)Old_Pos/9;
  // Revert the rect over the old cursor position
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

  // Display filled in rect in the top section of the display
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
 //Serial.println(pulses);
 attachInterrupt(0, A_FALL, FALLING);
}

void A_FALL(){
 detachInterrupt(0);
 A_SIG=0;
 
 pulses_prev = pulses;
 if(B_SIG==1)
 pulses++;
 if(B_SIG==0)
 pulses--;
 attachInterrupt(0, A_RISE, RISING);  
}

void B_RISE(){
 detachInterrupt(1);
 B_SIG=1;
 
 pulses_prev = pulses;
 if(A_SIG==1)
 pulses++;
 if(A_SIG==0)
 pulses--;
 attachInterrupt(1, B_FALL, FALLING);
}

void B_FALL(){
 detachInterrupt(1);
 B_SIG=0;
 
 pulses_prev = pulses;
 if(A_SIG==0)
 pulses++;
 if(A_SIG==1)
 pulses--;
 attachInterrupt(1, B_RISE, RISING);
}

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
      transmit_now = 1;
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
      buttonState = 1;
      Letter_Entered = 1;
      button5Pressed = 1;
     } else if (state5 == HIGH && button5Pressed) {
      button5Pressed = 0;
    }
    lastInterruptTime5 = currentTime;
  }
}

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

  // If letter was entered add the letter to To_Transmit buffer and output it on the display
  if (Letter_Entered==1){
    if (Letters[New_Position] == '#'){
      keyboard = !(keyboard);
      update_keys(keyboard);

      display.setCursor(3,0);
      display.setTextColor(BLACK);
      display.fillRect(0, 0, 128, 15, SSD1306_WHITE);

      if (char_counter > 10){
        String buffer = "";
        buffer = To_Transmit.substring(To_Transmit.length() - MAX_VISIBLE_CHARS);
        display.println(buffer);
      } else {
        display.println(To_Transmit);
      }
      
      display.display();

    } else {

      if (keyboard == 0){
        To_Transmit=To_Transmit + Letters[New_Position];
      } else {
        To_Transmit=To_Transmit + Numbers[New_Position];
      }

      char_counter++;

      display.setCursor(3,0);
      display.setTextColor(BLACK);
      display.fillRect(0, 0, 128, 15, SSD1306_WHITE);
      
      //scroll password to the left if the string size exceeds 10
      if (char_counter > 10){
        String buffer = "";
        buffer = To_Transmit.substring(To_Transmit.length() - MAX_VISIBLE_CHARS);
        display.println(buffer);
      } else {
        display.println(To_Transmit);
      }
    }
  } else if (transmit_now == 1) {

    Serial.println(To_Transmit);
    To_Transmit="";

    transmit_now = 0;
    char_counter=0;
  }
  display.display();
  Letter_Entered=0;
  delay(100);
}
