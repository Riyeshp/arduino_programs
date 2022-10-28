#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <math.h>

#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))

template< typename T, size_t N > size_t ArraySize (T (&) [N]){ return N; }

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// Variables will change:

int menu = 0; // menu value to prevent screen refresh
String screens[]= {"->Settings","->Test"};
String settingScreens[]={"< counter Limit    >", "< Interval     >"};
String testScreens[]={"Start", "pause", "Stop"};
int solenoidSwitch = 0;//?
bool manualSwitchStatus = false;//ON OFF button for manually operating the solenoid

int pinOut = 10;
int upwardSwitch = 0;
//EEPROM Variables
int interval;
int intervalAddress = 5;//Address location on EEPROM memory
int totalCount;
int totalCountAddress = 10;//Address location on EEPROM memory


#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

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
//EEPROM Files
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
  lcd.createChar(2, arrowdown);
  interval = EEPROMReadlong(intervalAddress);
  totalCount = EEPROMReadlong(totalCountAddress);
  pinMode(10, OUTPUT);
  Serial.print("interval:");
  Serial.println(interval);
  Serial.print("totalCount:");
  Serial.println(totalCount);

}

void loop() {
  uint8_t buttons = lcd.readButtons();


  //EEPROMWritelong(foglevelAddress,0);
  //EEPROMWritelong(incrementAddress,10);
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
       // sensorSimulator(487, 344);//only use for simulation with out sensors ******
        displayMenu(0);
        delay(25);
    }
          }
    // delay(500);
}

void displayMenu(int x) {//obselete function of displayView  but is used to display landing screen
     switch (x) {
      default:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print ("Default");
     
      case 0:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print (screens[x]);
        lcd.setCursor(0,1);
        lcd.print(x);
        // menu =100;
        break;
      case 1:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print (screens[x]);
        lcd.setCursor(0,1);
        lcd.print(x);
        menu =100;
        // calibration();
        break;
      case 2:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print (screens[x]);    
        lcd.setCursor(0,1);
        lcd.print(x);
        menu =100;
        break;
      case 3:
        //fogTest(interval,fogLevel, range);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print ("->Fog test");      
        lcd.setCursor(0,1);
        lcd.print(x);
        lcd.setCursor(2,1);
        lcd.print("Fog Level:10%");
        menu =100;
        break;
    }
}

void sensorValueReading(){
  // reads the input on analog pin A0 (value between 0 and 1023)
  int analogValue = analogRead(A0);
  int analogValue1 = analogRead(A2);
  // sensorReading = analogValue;

  //dummy data reading for testing============================
  // sensorReading = dummyData[dummyCounter];
  // dummyCounter++;
  // int array_length = sizeof(dummyData)/sizeof(int);

  // if(dummyCounter >= array_length){
  //   sensorReading = dummyData[array_length-1];
  // }
  //============================================================
  delay(100);

}



void menuView( String menu[], int menulength){
  int menuloc = 0;
  int x =0;
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
      if(menu == settingScreens ){
        menuView(screens,ArraySize(screens));
      }
      // menu = 0;
      x=1;
    }
    // else if (buttons & BUTTON_DOWN){
    //   // menu = 0;
    //   x=1;
    // }
    else if(buttons & BUTTON_SELECT){
      if(menu == screens){
        switch(menuloc){
        case 0:
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(menu[0]);
          menuView(settingScreens,ArraySize(settingScreens));
          break;
        }          
      } else if(menu == settingScreens){
        switch(menuloc){
          case 0:
            lcd.setCursor(0, 1);
            lcd.print("<FogLevel ");
            lcd.setCursor(10, 1);
            // lcd.print(fogLevel);
            lcd.setCursor(13, 1);
            lcd.print("% >");
            // fogLevelSetup(1);
            break;
          case 1:
            // intervalSetup();
            Serial.println("Freuency setup");
            break;

        }
    }
  }

}
}

void displayView(String menu[],int menulength, int menuloc){
  if(menu == screens){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(menu[menuloc]);
  }else if (menu == settingScreens){
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(menu[menuloc]);
  }
}

void manualSolenoidSwitch(){

  manualSwitchStatus = !manualSwitchStatus;

  if(manualSwitchStatus){
    lcd.setCursor(0,1);
    lcd.println("Solenoid ON     ");
    digitalWrite(pinOut, HIGH);
  }else{
    lcd.setCursor(0,1);
    lcd.println("Solenoid OFF    ");
    digitalWrite(pinOut, LOW);

  }
  delay(1000);
 

}

void intervalSetup(){
  int tempInterval;
  int timeinmilis;
  int tempIncrement =5;
  tempInterval = EEPROMReadlong(intervalAddress) /1000;
  int y = 1 ;
  int z = 2;
  while(y == 1){
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("<Interval ");
    lcd.setCursor(10, 1);
    lcd.print(tempInterval);
    lcd.setCursor(13, 1);
    lcd.print("s");
    lcd.setCursor(14, 1);
    lcd.print(" >");
    // lcd.setCursor(14, 1);
    // lcd.write(z);
    delay(300);
    uint8_t buttons = lcd.readButtons();
    Serial.println("test 680");
    if(buttons & BUTTON_LEFT && tempInterval >0){
      tempInterval= tempInterval-tempIncrement;
      Serial.println("test 684");
    }else if(buttons & BUTTON_RIGHT && tempInterval < 100){
      tempInterval= tempInterval+tempIncrement;
      Serial.println("test 687");
    }else if(buttons & BUTTON_SELECT){
      Serial.println(tempInterval);
      timeinmilis = tempInterval*1000;
      EEPROMWritelong(intervalAddress,timeinmilis);
      // interval = EEPROMReadlong(intervalAddress);
      Serial.println("test 691");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Interval:");
      lcd.print(tempInterval);
      lcd.println("  sec  ");
      lcd.setCursor(0, 1);
      lcd.print("Press ");
      lcd.setCursor(6,1);
      lcd.write(2);
      lcd.setCursor(7, 1);
      lcd.println(" to exit  ");
      z=3;
      y = 0;      
    }else if(buttons & BUTTON_DOWN){
     
      Serial.println("test 696");
    }
  }
}

 



  