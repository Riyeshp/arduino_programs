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
String screens[]= {"->Settings","->Calibration", "->Test Status"};
String settingScreens[]={"< FogLevel     >", "< Interval     >", "< Increment    >"};
String calibrationScreens[]={"< Start        >","< old results  >"};
String testScreens[]={"< Test start >","< Manual Start >"};

int dummyData[]= {480,479,478,477,476,475,474,473,472,471,470,469,468,467,466,466,466,465,463,462,461,460,459,458,457,456,455,455,454,453,452,451,450,450,449,448,447,446,445,444,443,442,441,440,439,438,437,436,435,434,433,432,431,430,429,428,427,426,425,424,423,422,421,420,419,418,417,416,415,414,413,412,411,410,409,408,407,406,405,404,403,402,401,400,399,398,397,396,395,394,393,392,391,390,389,388,387,386,385,384,383,382,381,380,380,380,380,380,381,382,383,384,385,386,387,388,389,390,391,392,393,394,395,396,397,398,399,400,401,402,403,404,405,406,407,408,409,410,411,412,413,414,415,416,417,418,419,420,421,422,423,424,425,426,427,428,429,430,431,432,433,434,435,436,437,438,439,440,441,442,443,444,445,446,447,448,449,450,451,452,453,454,455,456,457,458,459,460,461,462,463,465,466,467,468,469,470,471,472,472,472,472,472,473,474,475,476,477,478,479,480};
int dummyCounter = 0;
int sensorReading;//Actual sensor Reading data
int toggleSwitch;//sensorsimulator limit toggle switch. it also control the landing page up and down arrows.****
int solenoidSwitch = 0;//?
bool manualSwitchStatus = false;//ON OFF button for manually operating the solenoid
int fogTestStatus = 0; // loop control inside for test function
int startvalue = 340;//for sensorsimulator only
int default_max;//
int default_min;//
int pinOut = 10;
int upwardSwitch = 0;
//EEPROM Variables
int interval;
int intervalAddress = 5;//Address location on EEPROM memory
int fogLevel;
int foglevelAddress = 10;//Address location on EEPROM memory
int maxReading;
int maxReadingAddress =15;//Address location on EEPROM memory
int minReading;
int minReadingAddress=20;//Address location on EEPROM memory
int range;
int rangeAddress=25;//Address location on EEPROM memory
int testMinutes;
int testMinutesAddress=30;//Address location on EEPROM memory
int increment;
int incrementAddress=35;
int testSeconds;
int testSecondsAddress=40;//Address location on EEPROM memory

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
  fogLevel = EEPROMReadlong(foglevelAddress);
  maxReading =EEPROMReadlong(maxReadingAddress);
  minReading =EEPROMReadlong(minReadingAddress);
  testMinutes =EEPROMReadlong(testMinutesAddress);
  testSeconds =EEPROMReadlong(testSecondsAddress);
  range = EEPROMReadlong(rangeAddress);
  increment =EEPROMReadlong(incrementAddress);
  pinMode(10, OUTPUT);
  Serial.print("interval:");
  Serial.println(interval);
  Serial.print("fogLevel:");
  Serial.println(fogLevel);
  Serial.print("maxReading:");
  Serial.println(maxReading);
  Serial.print("minReading:");
  Serial.println(minReading);
  Serial.print("testMinutes:");
  Serial.println(testMinutes);
  Serial.print("range:");
  Serial.println(range);
  Serial.print("increment:");
  Serial.println(increment);
}

void loop() {
  uint8_t buttons = lcd.readButtons();


  // EEPROMWritelong(foglevelAddress,0);
  // EEPROMWritelong(incrementAddress,10);
  // EEPROMWritelong(maxReadingAddress, 470);
  //  EEPROMWritelong(minReadingAddress, 399);
  // EEPROMWritelong(rangeAddress, 91);
  // EEPROMWritelong(testMinutesAddress, 30);
  // EEPROMWritelong(testSecondsAddress, 23);
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
        displayMenu(99);
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
        fogTest(interval,fogLevel, range);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print ("->Fog test");      
        lcd.setCursor(0,1);
        lcd.print(x);
        lcd.setCursor(2,1);
        lcd.print("Fog Level:10%");
        menu =100;
        break;
      case 97:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print ("Calib Inprogress");
        lcd.setCursor(6, 0);
        lcd.print (range);
        lcd.setCursor(9, 0);
        lcd.print ("Time:");
        lcd.setCursor(14, 0);
        lcd.print (testMinutes);
        lcd.setCursor(0,1);
        lcd.print(sensorReading);
        break;
      case 98:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print ("Range:");
        lcd.setCursor(6, 0);
        lcd.print (range);
        lcd.setCursor(11, 0);
        lcd.print (testMinutes);
        lcd.print(":");
        lcd.print(testSeconds);        
        lcd.setCursor(0,1);
        lcd.print("Max:");
        // Serial.println(maxReading);
        lcd.setCursor(4,1);
        lcd.print(maxReading);
        lcd.setCursor(9,1);
        lcd.print("Min:");
        lcd.print(minReading);
        // lcd.setCursor(14,1);
        // lcd.print (sensorReading);
        // menu =100;
        break;
      case 99:
        // lcd.clear();
        lcd.setCursor(0,0);
        lcd.print ("-> Fog Reading   ");//extra space to clear screen without lcd.clear()
        lcd.setCursor(0,1);
        lcd.write(toggleSwitch);
        lcd.setCursor(2,1);
        lcd.print (sensorReading);// sensorReading2 is used for sensorSimulator /sensorReading is used for data from sensors****    
        lcd.setCursor(5,1);
        lcd.print (" lumen      ");//extra space to clear screen without lcd.clear()
        // menu =100;
        break;
    }
}

void sensorValueReading(){
  // reads the input on analog pin A0 (value between 0 and 1023)
  int analogValue = analogRead(A0);
  int analogValue1 = analogRead(A2);
  sensorReading = analogValue;

  //dummy data reading for testing============================
  // sensorReading = dummyData[dummyCounter];
  // sensorReading = 211;
  // dummyCounter++;
  // // int array_length = sizeof(dummyData)/sizeof(int);
  // // Serial.println(array_length);
  // if(dummyCounter >= 211){
  //   sensorReading = dummyData[210];
  // }
  //============================================================
  delay(100);

}

void calibration(int interval, int calibProcess){
  int maxValue;
  int minValue;
  // int newcounter =0; // simulatorValue variable
  int counter =0;
  int start;
  int end;
  long startTime;
  int previousValue;
  int currentValue;
  // int deltaValue;
  long currentTime;
  long previousTime;
  int tempvalue=0;
  int tempValue2 =0;
  int counter2 = 0;
  // int deltaTime;
  // Serial.println(interval);
  // Serial.println(calibProcess);
 
  sensorValueReading();
  startTime = millis();
  start = sensorReading;// change this to sensorReading for actual sensor readings
  end  = start -10;// hardcoded number for 10 less than the final value
  //log timestamp and start value to SD card
  previousTime = millis();
  previousValue = start;
  minValue = start;
  maxValue = start;
  digitalWrite(pinOut, HIGH); // started solenoid
  Serial.print("Sensor start reading:");
  Serial.println(start);
  Serial.print("Sensor approximate End reading:");
  Serial.println(end);
  Serial.print("Current");
  Serial.print("\t");
  Serial.print("Previous");
  Serial.print("\t");
  Serial.print("Min");
  Serial.print("\t");
  Serial.print("Max");
  Serial.println("\t");

  while(calibProcess == 1){
    sensorValueReading();
    //sensorReading = simulatorValues[newcounter];
    currentValue = sensorReading;// change this to sensorReading for actual sensor readings
    currentTime = millis();
    minValue = min(currentValue,minValue);
    maxValue = max(currentValue,maxValue);
    // Serial.print("currentValue:");
    Serial.print(currentValue);
    Serial.print("\t");
    // Serial.println('previousValue:');
    Serial.print(previousValue);
    Serial.print("\t");    
    //log timestamp and current value to SD card
    Serial.print(minValue);
    Serial.print("\t");
    Serial.print(maxValue);
    Serial.println(" ");
    lcd.setCursor(0,1);
    lcd.print("C");
    lcd.setCursor(1,1);
    lcd.print(currentValue);
    lcd.setCursor(5,1);
    lcd.print("Mi");
    lcd.setCursor(7,1);
    lcd.print(minValue);
    lcd.setCursor(11,1);
    lcd.print("Ma");
    lcd.setCursor(13,1);
    lcd.print(maxValue);    

    delay(interval);

    if (previousValue == currentValue){
      if (currentValue == minReading){
        if(tempValue2 ==  0){
        tempValue2 = currentValue;
        }else if (tempValue2 != 0 && tempValue2 != currentValue){
        counter2 =0;
        tempValue2 = currentValue;
        }else if (tempValue2 == currentValue){
        counter2++;
        tempValue2 = currentValue;
        }
      }
    }
    previousValue = currentValue;
    if (counter2 == 100){
      digitalWrite(pinOut, LOW);
      Serial.println("Solenoid stopped");
      long testduration = ((startTime - millis())/1000)/60;
      int fogRange = maxValue -minValue;
      long testseconds = (( millis() - startTime)/1000);
      long testminutes = testseconds/60;
      long remainseconds = testseconds%60;
      Serial.println("Calibartion completed");
      Serial.print("Sensor start:");
      Serial.println(start);
      Serial.print("Sensor approximate End reading:");
      Serial.println(end);
      Serial.print("Calibration end time:");
      Serial.println(millis());
      Serial.print("Current value:");
      Serial.println(currentValue);
      Serial.print("previous value:");
      Serial.println(previousValue);
      Serial.print("Max Reading:");
      Serial.println(maxValue);
      Serial.print("Min Reading:");
      Serial.println(minValue);
      Serial.print("Test Duration:");
      Serial.print(testminutes);
      Serial.print(":");
      Serial.println(remainseconds);
      Serial.print("Fog Range:");
      Serial.println(fogRange);
      EEPROMWritelong(maxReadingAddress,maxValue);
      EEPROMWritelong(minReadingAddress,minValue);
      EEPROMWritelong(testMinutesAddress,testminutes);
      EEPROMWritelong(testSecondsAddress,remainseconds);
      EEPROMWritelong(rangeAddress,fogRange);
      counter2 =0;
      calibProcess = 0;

    }        
  }
}

void fogTest(int interval , int foglevel, int range){
  //percentage logic here
  // int analogValue = analogRead(A0);
  // int analogValue1 = analogRead(A2);
  int limit;
  int upperlimit;
  int lowerlimit;
  int previousValue;
  int currentValue;
  int tolerance = 2;
  fogTestStatus = 1;
  sensorValueReading();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reading :");
  lcd.print(sensorReading);
  lcd.setCursor(0, 1);
  lcd.print("Fog level :");
  lcd.print(foglevel);
  // Serial.print("Analog reading : ");
  // Serial.print(sensorReading);
  // Serial.print("/n");
  previousValue = sensorReading;
  limit = round(range/10);

  if(fogLevel == 100){
    upperlimit = minReading + tolerance;
    lowerlimit = minReading;
    delay(1000);
  }else if(fogLevel == 0){
    upperlimit = maxReading ;
    lowerlimit = maxReading - tolerance;
    delay(1000);
  }else{
    upperlimit = (maxReading - (limit*(fogLevel/10)))+tolerance;
    lowerlimit = (maxReading - (limit*(fogLevel/10)))-tolerance;
    delay(1000);
  }
    Serial.print("Upperlimit:");
    Serial.print(upperlimit);
    Serial.println("\t");
    Serial.print("Lowerlimit:");
    Serial.print(lowerlimit);
    Serial.println("\t");
    Serial.print("previousValue");
    Serial.print("\t");
    Serial.print("currentValue");
    Serial.println("\t");

  while(fogTestStatus == 1){
      sensorValueReading();
      currentValue = sensorReading;
    if(sensorReading > upperlimit){
      digitalWrite(pinOut, HIGH);
      lcd.setBacklight(RED);
    }else if (sensorReading < lowerlimit){
      digitalWrite(pinOut, LOW);
      lcd.setBacklight(GREEN);
    }else{
      digitalWrite(pinOut, LOW);
      lcd.setBacklight(GREEN);
    }
    lcd.setCursor(9, 0);
    lcd.print(sensorReading);
    delay(interval);
    Serial.print(previousValue);
    Serial.print("\t");
    Serial.print(currentValue);
    Serial.println("\t");
   
    if(previousValue < currentValue){
        lcd.setCursor(14, 0);
        lcd.write(byte(0));
    }else if(previousValue > currentValue){
        lcd.setCursor(14, 0);
        lcd.write(byte(1));
    }
    previousValue = currentValue;
  }

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
      if(menu == settingScreens || menu == calibrationScreens || menu == testScreens){
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
        case 1:
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(menu[1]);
          menuView(calibrationScreens,ArraySize(calibrationScreens));
          break;
        case 2:
          // lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(menu[2]);
          menuView(testScreens,ArraySize(testScreens));
          break;
        }          
      } else if(menu == settingScreens){
        switch(menuloc){
          case 0:
            lcd.setCursor(0, 1);
            lcd.print("<FogLevel ");
            lcd.setCursor(10, 1);
            lcd.print(fogLevel);
            lcd.setCursor(13, 1);
            lcd.print("% >");
            fogLevelSetup(1);
            break;
          case 1:
            intervalSetup();
            Serial.println("Freuency setup");
            break;
          case 2:            
            //lcd.clear();
            incrementSetup();
            break;
        }
      }else if (menu == calibrationScreens){
          switch(menuloc){
          case 0:
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.println("Calib Inprogress");
            calibration(interval,1);
            displayMenu(98);
            break;
          case 1:
            lcd.clear();
            displayMenu(98);
            break;
        }
       
      }else if (menu == testScreens){
          switch(menuloc){
          case 0:
            fogTest(interval,fogLevel, range);
            // lcd.setCursor(0,1);
            // lcd.println("Test in progress ");
            break;
          case 1:
            manualSolenoidSwitch();
            // lcd.setCursor(0,1);
            // lcd.println("Solenoid ON     ");
            // digitalWrite(pinOut, HIGH);
            break;
        }
       
      }

    }
  }

}

void displayView(String menu[],int menulength, int menuloc){
  int line1x;
  int line1y;
  int line2x;
  int line2y;

  if(menu == screens){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(menu[menuloc]);

  }else if (menu == settingScreens || menu == calibrationScreens ||menu == testScreens  ){
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(menu[menuloc]);
    // lcd.setCursor(0, 1);
    // lcd.print(menu[menuloc]);

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

void arrowticker(int x, int y, int previousValue, int currentValue){
    if(previousValue < currentValue){
        lcd.setCursor(x, y);
        lcd.write(byte(0));
    }else if(previousValue > currentValue){
        lcd.setCursor(x, y);
        lcd.write(byte(1));

    }
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
void fogLevelSetup(int x ){
  int tempFogLevel;
  tempFogLevel = fogLevel;
  int y = 1 ;
  while(y == 1){
    lcd.setCursor(0, 1);
    lcd.print("<FogLevel ");
    lcd.setCursor(10, 1);
    lcd.print(tempFogLevel);
    lcd.setCursor(13, 1);
    lcd.print("% >");
    uint8_t buttons = lcd.readButtons();
    Serial.println("test 588");
    delay(300);
    if(buttons & BUTTON_LEFT && tempFogLevel >0){
      tempFogLevel= tempFogLevel-10;
      Serial.println("test 592");
    }else if(buttons & BUTTON_RIGHT && tempFogLevel < 100){
      tempFogLevel= tempFogLevel+10;
      Serial.println("test 595");
    }else if(buttons & BUTTON_SELECT){
      Serial.println(tempFogLevel);
      EEPROMWritelong(foglevelAddress,tempFogLevel);
      fogLevel = EEPROMReadlong(foglevelAddress);
      Serial.println("test 600");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Fog level:");
      lcd.print(fogLevel);
      lcd.println("  %  ");
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
void incrementSetup(){
  int tempIncrement;
  tempIncrement = EEPROMReadlong(incrementAddress);
  int y = 1 ;
  int z = 2;
  while(y == 1){
    lcd.setCursor(0, 1);
    lcd.print("<Increment ");
    lcd.setCursor(11, 1);
    lcd.print(tempIncrement);
    lcd.setCursor(15, 1);
    lcd.print(">");
    // lcd.setCursor(14, 1);
    // lcd.write(z);
    delay(300);
    uint8_t buttons = lcd.readButtons();
    Serial.println("test 763");
 
    if(buttons & BUTTON_LEFT && tempIncrement >0){
      tempIncrement= tempIncrement-1;
      Serial.println("test 767");
    }else if(buttons & BUTTON_RIGHT && tempIncrement < 150){
      tempIncrement= tempIncrement+1;
      Serial.println("test 770");
    }else if(buttons & BUTTON_SELECT){
      Serial.println(tempIncrement);
      EEPROMWritelong(incrementAddress,tempIncrement);
      Serial.println("test 774");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Increment: ");
      lcd.print(tempIncrement);
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
     
      Serial.println("test 779");
    }
    }
  }


