#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <math.h>

//array length calculation
#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))
template< typename T, size_t N > size_t ArraySize (T (&) [N]){ return N; }

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// Variables
int menu = 0; // menu value to prevent screen refresh
String screens[]= {"->Settings", "->Test Status"};
String settingScreens[]={"<Angle         >", "<Speed         >", "<Limit         >"};
String testScreens[]={"<Test start    >","<Manual control>"};
int sensorReading;//Actual sensor Reading data
int toggleSwitch;// it control the landing page up and down arrows.****
bool manualSwitchStatus = false;//ON OFF button for manually operating the solenoid
int fogTestStatus = 0; // variable to drive loop function inside  fogtest function.
int pinOut1 = 6; //sensor pin
int pinOut2 = 7; //sensor pin
//EEPROM Variables
int speed;
int speedAddress=5;//Address location on EEPROM memory
int angle;
int angleAddress=25;//Address location on EEPROM memory
long limit;
long limitAddress=45;
long counter;
long counterAddress=55;


//LCD color assignment
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7
//Custom character assignment
byte customChar1[] = {
  B00100,
  B01110,
  B11111,
  B11111,
  B01110,
  B01110,
  B01110,
  B01110
};
byte customChar2[] = {
  B01110,
  B01110,
  B01110,
  B01110,
  B11111,
  B11111,
  B01110,
  B00100
};
byte arrowdown[] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100
};
//EEPROM functions
long EEPROMReadlong(long address)
      {
      //Read the 4 bytes from the eeprom memory.
      long four = EEPROM.read(address);
      long three = EEPROM.read(address + 1);
      long two = EEPROM.read(address + 2);
      long one = EEPROM.read(address + 3);
      //Return the recomposed long by using bitshift.
      return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
      }
void EEPROMWritelong(int address, long value)
      {
      //Decomposition from a long to 4 bytes by using bitshift.
      //One = Most significant -> Four = Least significant byte
      byte four = (value & 0xFF);
      byte three = ((value >> 8) & 0xFF);
      byte two = ((value >> 16) & 0xFF);
      byte one = ((value >> 24) & 0xFF);
      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(address, four);
      EEPROM.write(address + 1, three);
      EEPROM.write(address + 2, two);
      EEPROM.write(address + 3, one);
      }
//EEPROM Files done
void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setBacklight(WHITE);
  lcd.createChar(0, customChar1);
  lcd.createChar(1, customChar2);
  speed = EEPROMReadlong(speedAddress);
  angle = EEPROMReadlong(angleAddress);
  limit =EEPROMReadlong(limitAddress);
  pinMode(10, OUTPUT);
  Serial.print("Speed:");
  Serial.println(speed);
  Serial.print("Angle:");
  Serial.println(angle);
  Serial.print("Limit:");
  Serial.println(limit);
}
void loop() {
  uint8_t buttons = lcd.readButtons();
  // below lines of codes are used  whenever values had to be written into the EEPROM addresses DONT delete!!
  // EEPROMWritelong(speedAddress,8500);
  // EEPROMWritelong(angleAddress,175);
  // EEPROMWritelong(limitAddress, 85000);

  //button selected
  if (buttons) {
      lcd.clear();
      lcd.setCursor(0,0);
      if (buttons & BUTTON_UP) {
        lcd.setBacklight(YELLOW);
        displayMenu(99);
      }
      if (buttons & BUTTON_DOWN){
        lcd.setBacklight(YELLOW);  
        displayMenu(99);
      }      
      if (buttons & BUTTON_SELECT){
        lcd.setBacklight(VIOLET);
        menuView(screens,ArraySize(screens));    
      }
    }
  //button not selected landing screen
  if(!buttons){
    if(menu == 0){//menu value is just to avoid lcd refresh
      lcd.setBacklight(YELLOW);
      sensorValueReading();  
      displayMenu(99);
    }
    delay(1000);    
  }
}
void displayMenu(int x) {//function to display landing screen.
  switch (x) {
    default:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print ("Default");
    case 98:
      lcd.clear();
      lcd.setCursor(0,0);
      // lcd.print ("Range:");
      // lcd.setCursor(6, 0);
      // lcd.print (range);
      lcd.setCursor(11, 0);
     // lcd.print (testMinutes);
      lcd.print(":");
     // lcd.print(testSeconds);        
      lcd.setCursor(0,1);
      lcd.print("Max:");
      lcd.setCursor(4,1);
    //  lcd.print(maxReading);
      lcd.setCursor(9,1);
      lcd.print("Min:");
     // lcd.print(minReading);
      break;     
    case 99:
      lcd.setCursor(0,0);
      lcd.print ("->Cycle Count  ");//extra space to clear screen without lcd.clear()
      lcd.setCursor(0,1);
      lcd.print (sensorReading);// sensorReading2 is used for sensorSimulator /sensorReading is used for data from sensors****    
      lcd.setCursor(5,1);
      //lcd.print (" lumen      ");//extra space to clear screen without lcd.clear()
      break;
  }
}
void sensorValueReading(){//function to read the sensor at analog pin A0(Big LDR sensor). (A1(small LDR sensor) is not yet used)
  // reads the input on analog pin A0 (value between 0 and 1023)
  int analogValue = analogRead(A0);
  int analogValue1 = analogRead(A2);
  sensorReading = analogValue;
  delay(1000);
}
void testcycle(int interval , long limit, int range){// fogtest function.
  //percentage logic here
  // int analogValue = analogRead(A0);
  // int analogValue1 = analogRead(A2);
  // testDuration =EEPROMReadlong(testDurationAddress);

  int previousValue;
  int currentValue;
  int tolerance = 2;
  fogTestStatus = 1;
  int startTime = millis();
  float currentTime;
  float tempTime;
  sensorValueReading();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reading :");
  lcd.print(sensorReading);
  lcd.setCursor(0, 1);
  lcd.print("Fog level :");
  //lcd.print(foglevel);
}
void menuView( String menu[], int menulength){// menu view for arduino LCD
  int menuloc = 0;
  int x =0;// Variable to drive the while loop listening to button presses.
  delay(500);
  Serial.println(menu[0]);
  displayView(menu,menulength, menuloc);
  while (x == 0){
    uint8_t buttons = lcd.readButtons();
    if(buttons & BUTTON_RIGHT){        
      if(menuloc < menulength-1){
        menuloc++;
        displayView(menu,menulength, menuloc);
        delay(500);
      }
    }else if(buttons & BUTTON_LEFT){        
      if(menuloc > 0){
        menuloc--;
        displayView(menu,menulength, menuloc);
        delay(500);
      }
    }else if (buttons & BUTTON_UP || buttons & BUTTON_DOWN ){
      if(menu == settingScreens || menu == testScreens){
        menuView(screens,ArraySize(screens));
      }
      x=1;
    }else if(buttons & BUTTON_SELECT){
      if(menu == screens){
        switch(menuloc){
        case 0:
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(menu[0]);
          menuView(settingScreens,ArraySize(settingScreens));
          break;
        case 1:
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(menu[1]);
          menuView(testScreens,ArraySize(testScreens));
          break;
        }          
      }else if(menu == settingScreens){
        switch(menuloc){
          case 0:
            AngleSetup(1);
            Serial.println("Angle setup");
            break;
          case 1:
            speedSetup();
            Serial.println("Speed setup");
            break;
          case 2:            
            limitSetup();
            Serial.println("Limit setup");
            break;
        }
      }else if (menu == testScreens){
        Serial.println("test 410");
        switch(menuloc){
          case 0:
            // fogTest(interval,fogLevel, range);
            lcd.setCursor(0,1);
            lcd.println("Test in progress ");
            break;
          case 1:
            lcd.setCursor(0,1);
            lcd.println("testing check     ");
            //digitalWrite(pinOut, HIGH);
            break;
        }
       
      }
    }
  }

}
void displayView(String menu[],int menulength, int menuloc){// function to display various Menuview option

  if(menu == screens){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(menu[menuloc]);
  }else if (menu == settingScreens ||menu == testScreens  ){
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(menu[menuloc]);
  }
 

}
void speedSetup(){// function to set the interval value for the tests.
  int tempSpeed;
  int timeinmilis;
  int tempIncrement = 1; //increment;// change that to increment if you want to drive the interval increment by increment variable
  tempSpeed = EEPROMReadlong(speedAddress);
  int y = 1 ;// Variable to drive the while loop listening to button presses.
  int z = 2;
  lcd.clear();
  while(y == 1){
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("<Speed:");
    lcd.setCursor(7, 1);
    lcd.print(tempSpeed);
    lcd.setCursor(13, 1);
    lcd.print("rpm");
    delay(300);
    uint8_t buttons = lcd.readButtons();
    // Serial.println("test 680");
    if(buttons & BUTTON_LEFT && tempSpeed >0){
      tempSpeed= tempSpeed-100;
      // Serial.println("test 684");
    }else if(buttons & BUTTON_RIGHT && tempSpeed < 100000){
      tempSpeed= tempSpeed+100;
      // Serial.println("test 687");
    }else if(buttons & BUTTON_SELECT){
      Serial.println(tempSpeed);
      EEPROMWritelong(speedAddress,tempSpeed);
      // interval = EEPROMReadlong(intervalAddress);
      // Serial.println("test 691");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Speed:");
      lcd.print(tempSpeed); 
      lcd.setCursor(12, 0);
      lcd.println("rpm  ");
      lcd.setCursor(0, 1);
      lcd.print("Press ");
      lcd.setCursor(6,1);
      lcd.write(2);
      lcd.setCursor(7, 1);
      lcd.println(" to exit  ");
      z=3;
      y = 0;      
    }else if(buttons & BUTTON_DOWN){
      // Serial.println("test 696");
    }
  }
}
void AngleSetup(int x ){// function to set the fog level.
  int tempAngle;
  tempAngle = angle;
  lcd.clear();
  int y = 1 ;
  while(y == 1){
    lcd.setCursor(0, 1);
    lcd.print("<Angle ");
    lcd.setCursor(9, 1);
    lcd.print(tempAngle);
    lcd.setCursor(12, 1);
    lcd.print("deg>");
    uint8_t buttons = lcd.readButtons();
    // Serial.println("test 588");
    delay(500);
    if(buttons & BUTTON_LEFT && tempAngle >0){
      tempAngle= tempAngle-5;
      // Serial.println("test 592");
    }else if(buttons & BUTTON_RIGHT && tempAngle < 360){
      tempAngle= tempAngle+5;
      // Serial.println("test 595");
    }else if(buttons & BUTTON_SELECT){
      Serial.println(tempAngle);
      EEPROMWritelong(angleAddress,tempAngle);
      angle = EEPROMReadlong(angleAddress);
      // Serial.println("test 600");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Angle:");
      lcd.setCursor(6, 0);
      lcd.print(angle);
      lcd.setCursor(9, 0);
      lcd.println("deg    ");
      lcd.setCursor(0, 1);
      lcd.print("Press ");
      lcd.setCursor(6,1);
      lcd.write(2);
      lcd.setCursor(7, 1);
      lcd.println(" to exit  ");
      y = 0;
    }
  }
 

}
void limitSetup(){// function to set the increment values. 
  long tempLimit;
  tempLimit = EEPROMReadlong(limitAddress);
  int y = 1 ;// Variable to drive the while loop listening to button presses.
  int z = 2;
  while(y == 1){
    lcd.setCursor(0, 1);
    lcd.print("<Limit:        >");
    lcd.setCursor(7, 1);
    lcd.print(tempLimit);
    // lcd.setCursor(15, 1);
    // lcd.print(">");
    // lcd.setCursor(14, 1);
    // lcd.write(z);
    delay(300);
    uint8_t buttons = lcd.readButtons();
    // Serial.println("test 763");
     if(buttons & BUTTON_LEFT && tempLimit >0){
      tempLimit= tempLimit-1000;
      // Serial.println("test 767");
    }else if(buttons & BUTTON_RIGHT && tempLimit < 1000000){
      tempLimit= tempLimit+1000;
      // Serial.println("test 770");
    }else if(buttons & BUTTON_SELECT){
      Serial.println(tempLimit);
      EEPROMWritelong(limitAddress,tempLimit);
      // Serial.println("test 774");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Limit:        >");
      lcd.setCursor(6, 0);
      lcd.print(tempLimit);
      lcd.println("    ");
      lcd.setCursor(0, 1);
      lcd.print("Press ");
      lcd.setCursor(6,1);
      lcd.write(2);
      lcd.setCursor(7, 1);
      lcd.println(" to exit  ");
      z=3;
      y = 0;
    }else if(buttons & BUTTON_DOWN){     
      // Serial.println("test 779");
    }
  }
}













