// Arduino G-code Interpreter
// v1.0 by Mike Ellery - initial software (mellery@gmail.com)
// v1.1 by Zach Hoeken - cleaned up and did lots of tweaks (hoeken@gmail.com)
// v1.2 by Chris Meighan - cleanup / G2&G3 support (cmeighan@gmail.com)
// v1.3 by Zach Hoeken - added thermocouple support and multi-sample temp readings. (hoeken@gmail.com)
//#include <HardwareSerial.h>
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
  lcd.begin(20, 4);

  lcd.setCursor(0,0);
  lcd.print("starting");

  //other initialization.
  init_process_string();
  init_steppers();

}
const int FILE_MODE=1;
const int SERIAL_MODE=2;

int option=1;
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

void loopFile(){
  lcd.setCursor(0,0);
  lcd.print("Initializing SD card");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(SD_SELECT, OUTPUT);

  int _try=0;
  while (!SD.begin(SD_SELECT)) {
    lcd.setCursor(0,1);
    lcd.print("init failed! " );
    lcd.print(++_try);
    Serial.println("sd failed");
    delay(1000);
  }
  Serial.println("sd ok");
  lcd.setCursor(0,1);
  lcd.print("init done.      ");

  delay(100);
  if (SD.exists("AMIGUS.GCO")) {
    lcd.setCursor(0,2);
    lcd.print("AMIGUS.GCO exist");
  }
  while (Serial.available() <= 0 || Serial.read()!='s'){
    delay(500);
    lcd.setCursor(0,3);
    lcd.print("waiting por 's' signal");
  }
  File myFile = SD.open("AMIGUS.GCO");
  char_count=0;
  int linecount = 0;
  int totalcount=0;
  char chr;
  if (myFile) {
    while (myFile.available()) {
      //  c='\0';
      chr = myFile.peek();
      while(chr<=0){
        Serial.print("peek failed, re-reading. response: "); 
        Serial.println((int)chr);
        delay(1000);
        chr = myFile.peek();
      }
      myFile.read();
      totalcount++;
      //  myFile.seek(myFile.position()-1);
      //  char c1 = myFile.read();
      //  char c1 = c;
      //    Serial.print(c);
      //    Serial.print("  Re-read :");
      //    Serial.println(c1);

      // Serial.println((int)chr);

      if((int)chr <0){
        Serial.print("Leido incorrecto en el caracter ");
        Serial.println(totalcount);
        delay(3000);
      }


      if(char_count && (chr == '\n')){
        linecount++;
        Serial.print("Sended line ");
        Serial.print(linecount);
        //Serial.print("  total char ");
        //Serial.print(totalcount);
        Serial.print(": ");
        command[char_count]='\0';
        Serial.println(command);

        //process our command!
        process_string(command, char_count);
        //clear command.
        //   init_process_string();

        char_count=0;
      }
      else{
        if(char_count<COMMAND_SIZE){
          command[char_count++] = chr;
        }
      }
    }
    myFile.close();
  }

  //  }
  //  else {
  //    Serial.println("AMIGUS.GCO doesn't exist.");
  //    // open a new file and immediately close it:
  //  Serial.println("Creating AMIGUS.GCO...");
  //  File myFile = SD.open("AMIGUS.GCO", FILE_WRITE);
  //  myFile.close();
  //
  //   myFile = SD.open("AMIGUS1.GCO", FILE_WRITE);
  //  myFile.close();
  //
  //    return;
  //  }
  Serial.println("disabling steppers of file print...");
  disable_steppers();
}

void loopSerial(){
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
      delayMicroseconds(10000);
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










