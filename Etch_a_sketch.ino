// Arduino G-code Interpreter
// v1.0 by Mike Ellery - initial software (mellery@gmail.com)
// v1.1 by Zach Hoeken - cleaned up and did lots of tweaks (hoeken@gmail.com)
// v1.2 by Chris Meighan - cleanup / G2&G3 support (cmeighan@gmail.com)
// v1.3 by Zach Hoeken - added thermocouple support and multi-sample temp readings. (hoeken@gmail.com)

// Downloaded from http://sourceforge.net/projects/reprap/files/Arduino%20Firmware/v1.3/

#define ENABLE_LCD

#include <SD.h>
#include <SPI.h>
#ifdef ENABLE_LCD
#include <Wire.h>
#include <LiquidCrystal.h>
#endif

//our command string
#define COMMAND_SIZE 96
#define SD_SELECT 10

#define BUTTON1 2
#define BUTTON2 3

char command[COMMAND_SIZE+1];
byte char_count=0;

#ifdef ENABLE_LCD
LiquidCrystal lcd(0x0);
#endif

void setup()
{
  Serial.begin(9600);

  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(SD_SELECT, OUTPUT);
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);

#ifdef ENABLE_LCD
  lcd.begin(20, 4);
  lcd.setCursor(0,0);
  lcd.print("Init..");
#endif

  //other initialization.
  init_steppers();
  drawFullMenu();
}

void waitButtonRelease(int pin){
  while(digitalRead(pin)==LOW)
    delay(10);
}

void printFullLine(int line, char* text){
#ifdef ENABLE_LCD
  lcd.setCursor(0, line);
  if(strlen(text)>20){
    char tmp = text[20];
    text[20]=0;
    lcd.print(text);
    text[20] = tmp;
  }
  else{
    lcd.print(text);
    for(int i=strlen(text); i<20; i++){
      lcd.print(' ');
    }
  }
#else
  Serial.println(text);
#endif
}

////////////////
//  MENU CONTROL
////////////////

const int TOTAL_MENU_COUNT = 3;
const char menus[TOTAL_MENU_COUNT][21]={
  "1.Serial comm.","2.SD file", "3.Move home (0,0)"};
int selectedMenu=0;

void drawFullMenu(){
  printFullLine(0, "# Select mode:");
  drawMenu();
}

void drawMenu(){
  #ifdef ENABLE_LCD
  printFullLine(1, (char *)menus[selectedMenu]);
  #endif
}

void executeSelectedMenu(){
  #ifdef ENABLE_LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("# ");
  lcd.print(menus[selectedMenu]+2);
  #endif
  switch(selectedMenu){
  case 0:
    loopSerial();
    break;
  case 1:
    loopFile();
    break;
  case 2:
    gotoHome();
    break;
  }
  waitButtonRelease(BUTTON1);
  waitButtonRelease(BUTTON2);
}

////////////////
//  FILE
////////////////
File root;
boolean sd_loaded=false;
void loopFile(){
  if(root){
    root.close();
  }
  printFullLine(1,"SD Init..");
  delay(100);
  if(!sd_loaded){
    if (!SD.begin(SD_SELECT)) {
      printFullLine(1, "SD failed!" );
      delay(1000);
      return;
    }
  }
  sd_loaded=true;
  printFullLine(1, "File:" );

  root = SD.open("/");
  File entry;
  boolean fileSelected=false;
  while(true) {
    entry =  root.openNextFile();
    if (! entry) {
      break;
    }
    printFullLine(2, entry.name());    
    while(digitalRead(BUTTON1)==HIGH){
      delay(20); 
      if(digitalRead(BUTTON2)==LOW){
        waitButtonRelease(BUTTON2);
        fileSelected=true;
        break;
      }
    }
    if(fileSelected){
      break;
    }
    waitButtonRelease(BUTTON1);
    entry.close();
  }
  
  #ifdef ENABLE_LCD
  printFullLine(1, "Print: ");
  lcd.setCursor(7, 1);
  lcd.print(entry.name());
  printFullLine(2, "Command:");
  #endif
  
  int startTime=millis()/1000;
  if(entry){
    int linecount = 0;
    char chr;
    while (entry.available()) {
      if(digitalRead(BUTTON2)==LOW){
        entry.close();
        return;
      }
      chr = entry.read();
      if(char_count && (chr == '\n')){
        command[char_count]=0;
//        printFullLine(3, command);
        process_string(command, char_count);
        char_count=0;
      }
      else
        if(char_count<COMMAND_SIZE){
          command[char_count++] = chr;
        }
    }
    if(char_count && char_count<COMMAND_SIZE){
      command[char_count]=0;
  //    printFullLine(3, command);
      process_string(command, char_count);
      char_count=0;
    }
    entry.close();
    #ifdef ENABLE_LCD
    Serial.println("Finished ");
    printFullLine(3, " ");
    printFullLine(2, "Finished ");
    lcd.setCursor(9,2);
    lcd.print( millis() / 1000 - startTime );
    lcd.print("sec");
    #endif
    while(digitalRead(BUTTON1)==HIGH && digitalRead(BUTTON2)==HIGH){
      delay(20); 
    }
    waitButtonRelease(BUTTON1);
    waitButtonRelease(BUTTON2);
  }
}

////////////////
//  SERIAL
////////////////

void loopSerial(){
  printFullLine(1, "Listening");
  Serial.println("Listening");
  byte no_data = 0;
  char c;
  while(true){
    if (Serial.available() > 0)
    {
      c = Serial.read();
      no_data = 0;
      //newlines are ends of commands.
      if (c != '\n')
      {
        command[char_count++] = c;
      }
    }
    else
    {
      if(no_data<127)
        no_data++;
      delay(10);
    }
    //if theres a pause (1 second) or we got a real command, do it
    if (char_count && (c == '\n' || no_data > 100))
    {
//      printFullLine(3, command);
      process_string(command, char_count);
      char_count=0;
    }
    if(digitalRead(BUTTON2)==LOW){
      return;
    }
  }
}

void gotoHome(){
  process_string("G21",3);
  process_string("G90",3);
  process_string("G1 X0.0 Y0.0",12);
}


void loop()
{
  if(digitalRead(BUTTON1)==LOW){
    selectedMenu =  (selectedMenu+1) % TOTAL_MENU_COUNT;
    drawMenu();
    waitButtonRelease(BUTTON1);
  }
  else if(digitalRead(BUTTON2)==LOW){
    waitButtonRelease(BUTTON2);
    executeSelectedMenu();
    drawFullMenu();
  }
  else{
    delay(30);
  }
}


