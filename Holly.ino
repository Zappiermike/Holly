#include <LiquidCrystal.h>
#include <TimeLib.h>
#include <SD.h>
#include <SPI.h>

LiquidCrystal lcd(3,5,6,7,8,9);
File myFile;

int numCols = 16;
int numRows = 2;
#define numCols 16
#define numRows 2
#define jan1 1609459200

char holidays[500];
int setdate = -1;
//const unsigned long jan1 = 1609459200;

void setup() {
  setTime(23, 59, 45, 10, 10, 2021);
  lcd.begin(numCols, numRows);

  // Establish a good serial connection
  Serial.begin(230400);
  while (!Serial) {
    ; // wait for serial port to connect
  }
  Serial.print("Initializing SD card...");
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
}

void loop() {
  // calculate the number of days since Jan 1 of this year
  int date = round((now()-jan1)/86400) % 365;

  // First time running program, assign date
  if (setdate == -1){
    Serial.println("First run...");
    setdate = date;
    determine_holidays(date);
  }
  
  // A new day has started!
  // Open SD Card, jump to newdate, read line into holidays array, 
  // close the file, print on lcd, change setdate to date
  else if (setdate != date){
    Serial.println("A new day has occurred!");
    determine_holidays(date);
    setdate = date;
  }

  // Day hasn't changed. Print the date, then read whatever was last stored in holidays
  else{
    if (holidays[0] == '\0'){
      return;
    }

    Serial.println(holidays);
    char delimiter = ',';
    char single_holiday[90];  // longest holiday is 86 chars

    int counter = 0;
    for (int i = 0; i < 499; i++){
      if (holidays[i+1] == '\0' || holidays[i] == ';'){
          display_text(single_holiday);
          memset(single_holiday, 0, sizeof(single_holiday));
          delay(10);
          break;
      }
      else if (holidays[i] == delimiter){
        counter = i+1;
        display_text(single_holiday);
        memset(single_holiday, 0, sizeof(single_holiday));
        delay(10);
      }
      else{
        single_holiday[i-counter] = holidays[i]; // Appends each char to the single_holiday array until we reach a comma to separate it
        delay(20);
      }
    }
    Serial.println("");
    Serial.println("Day has not changed...");
    delay(3000);
  }
}

void display_text(char *single_holiday){
  lcd.clear();
  delay(50);
  lcd.setCursor(0,0);
  lcd.print(monthShortStr(month()));
  lcd.print(" ");
  lcd.print(day());
  lcd.print(", ");
  lcd.print(year());
  
  Serial.print(single_holiday);
  lcd.setCursor(0,1);
  marquee(single_holiday);
  delay(500);
}

void determine_holidays(unsigned int date){
  byte index = 0;
  memset(holidays, 0, sizeof(holidays));
  myFile = SD.open("HOLLY.TXT");
    delay(500);
    
    // Success accessing SD Card
    if (myFile){
      unsigned int longcount = 0;
      char nextChar;
      while (myFile.available() > 0){
        // Read until we get to the correct line
        while (longcount != date){
          nextChar = myFile.read();
          if (nextChar == '\n'){
            longcount++;
          }
        }
        // Seek to line associated with how many days have passed since Jan 1
        myFile.seek(myFile.position());
        nextChar = myFile.read();        
        
        // Store the whole line into an array
        while(nextChar != '\n'){
          holidays[index] = nextChar;
          if (nextChar == ';')
            break;
          index++;
          nextChar = myFile.read();
          delay(10);
        }
        
        holidays[index] = '\0';
        break;
      }
      
      delay(100);
      myFile.close();
      delay(100);
    }

    // Failure accessing SD Card
    else{
      Serial.print("FAILED to open SD Card");
      lcd.clear();
      lcd.print("SD CARD FAIL");
      delay(30000);
    }
    // Finally, assign the setdate to the new date
//    setdate = date;
//    return;
}

void marquee(char *text)
{
  int length = strlen(text)+2;
  if(length < numCols){
    lcd.print(text);
    delay(1000);
  }
  else { //if (length > 40){
    bool end_of_text = false;
    int spot = 0;
    String str((char*)text);
    str += "  ";
    
    lcd.print(str.substring(spot, spot+16));
    delay(1000);
    while (end_of_text == false){
      lcd.setCursor(0,1);
      lcd.print(str.substring(spot, spot+16));
      spot = spot + 2;
      if (spot > (length-16)){
        spot = 0;
        end_of_text = true;
      }
      delay(300);
    }
  }
//  else
//  {
//    int pos;
//    for(pos = 0; pos < numCols; pos++)
//      lcd.print(text[pos]);
//    delay(1000);
//    while(pos < length)
//    {
//      lcd.scrollDisplayLeft();
//      lcd.print(text[pos]);
//      pos = pos + 1;
//      delay(275);  
//    }  
//  }
}
