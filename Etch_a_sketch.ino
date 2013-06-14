// Arduino G-code Interpreter
// v1.0 by Mike Ellery - initial software (mellery@gmail.com)
// v1.1 by Zach Hoeken - cleaned up and did lots of tweaks (hoeken@gmail.com)
// v1.2 by Chris Meighan - cleanup / G2&G3 support (cmeighan@gmail.com)
// v1.3 by Zach Hoeken - added thermocouple support and multi-sample temp readings. (hoeken@gmail.com)

// Downloaded from http://sourceforge.net/projects/reprap/files/Arduino%20Firmware/v1.3/

#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal.h>

//our command string
#define COMMAND_SIZE 128
#define SD_SELECT 10

char command[COMMAND_SIZE];
byte char_count;

LiquidCrystal lcd(0x0);

void setup()
{
  //Do startup stuff here
  Serial.begin(9600);

  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(SD_SELECT, OUTPUT);

  lcd.begin(20, 4);
  lcd.setCursor(0,0);
  lcd.print("Starting...");

  //other initialization.
  init_process_string();
  init_steppers();

}
const int FILE_MODE=1;
const int SERIAL_MODE=2;

int option = FILE_MODE;
void loop()
{
  switch(option){
  case SERIAL_MODE:
    loopSerial();
    break;
  case FILE_MODE:
    loopFile();
    break;

  }
  lcd.clear();
  lcd.print("Job finished");

}

////////////////
//  FILE
////////////////
void loopFile(){
  lcd.setCursor(0,0);
  lcd.print("Initializing SD card");
  int _try=0;
  while (!SD.begin(SD_SELECT)) {
    lcd.setCursor(0,1);
    lcd.print("init failed! " );
    lcd.print(++_try);
    Serial.println("sd failed");
    delay(1000);
  }
  Serial.println("sd loaded");
  lcd.setCursor(0,1);
  lcd.print("sd init done.       ");

  delay(100);
  if (!SD.exists("IMAGE.GCO")) {
    Serial.println("File doesn't exist");
  }else{
//    lcd.setCursor(0,2);
//    lcd.print("IMAGE.GCO exist");
    while (Serial.available() <= 0 || Serial.read()!='s'){
      delay(500);
//      lcd.setCursor(0,3);
//      lcd.print("waiting 's'");
    }
//      lcd.setCursor(0,3);
//      lcd.print("s received");

    int linecount = 0;
    char chr;
    File myFile = SD.open("IMAGE.GCO");
    if (!myFile) {
      Serial.println("File open error");
    }else{
      while (myFile.available()) {
        chr = myFile.read();
        
        if(char_count && (chr == '\n')){
          linecount++;
          Serial.print("Sended line ");
          Serial.print(linecount);
          Serial.print(": ");
          command[char_count]='\0';
          Serial.println(command);

          process_string(command, char_count);
          char_count=0;
        }
        else{
          if(char_count<(COMMAND_SIZE-1)){
            command[char_count++] = chr;
          }
        }
      }
      myFile.close();
      Serial.println("File finished.");
    }
  }
  disable_steppers();
}


////////////////
//  FILE
////////////////

void loopSerial(){
  lcd.setCursor(0,0);
  lcd.print("Serial commands mode");

  Serial.println("waiting for commands");
  int no_data = 0;
  char c;
  while(true){
    //read in characters if we got them.
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
    //mark no data.
    else
    {
      no_data++;
      delay(10);
    }

    //if theres a pause or we got a real command, do it
    if (char_count && (c == '\n' || no_data > 100))
    {
      //process our command!
      process_string(command, char_count);

      //clear command.
      init_process_string();
      char_count=0;
    }

    //no data?  turn off steppers
    //    if (no_data > 100000){
    //      disable_steppers();
    //      Serial.println("disabling steppers on serial comm...");
    //      return;
    //    }
  }
}












