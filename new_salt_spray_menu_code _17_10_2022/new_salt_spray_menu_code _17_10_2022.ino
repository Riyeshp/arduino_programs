#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <math.h>
#include <movingAvg.h> 

//array length calculation
#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))
template< typename T, size_t N > size_t ArraySize (T (&) [N]){ return N; }

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// Variables
int menu = 0; // menu value to prevent screen refresh
String screens[]= {"->Settings","->Calibration", "->Test Status"};
String settingScreens[]={"< FogLevel     >", "< Interval     >", "< Increment    >", "< Test Duration>"};
String calibrationScreens[]={"< Start        >","< old results  >"};
String testScreens[]={"< Test start >","< Manual Start >"};
int sensorReading;//Actual sensor Reading data
int toggleSwitch;// it control the landing page up and down arrows.****
bool manualSwitchStatus = false;//ON OFF button for manually operating the solenoid
int fogTestStatus = 0; // variable to drive loop function inside  fogtest function.
int pinOut = 10; //sensor pin
movingAvg sensorValues(15);//moving average data point set. any value more than 20 is breaking the code.
//EEPROM Variables
int interval;
int intervalAddress=5;//Address location on EEPROM memory
int fogLevel;
int foglevelAddress=10;//Address location on EEPROM memory
int maxReading;
int maxReadingAddress=15;//Address location on EEPROM memory
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
int testDuration;
int testDurationAddress=45;
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
  sensorValues.begin();
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
  testDuration =EEPROMReadlong(testDurationAddress);
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
  Serial.print("testSeconds:");
  Serial.println(testSeconds);
  Serial.print("range:");
  Serial.println(range);
  Serial.print("increment:");
  Serial.println(increment);
  Serial.print("Test Duration:");
  Serial.println(testDuration);
}
void loop() {
  uint8_t buttons = lcd.readButtons();
  // below lines of codes are used  whenever values had to be written into the EEPROM addresses DONT delete!!
  // EEPROMWritelong(foglevelAddress,0);
  // EEPROMWritelong(incrementAddress,1);
  // EEPROMWritelong(maxReadingAddress, 470);
  // EEPROMWritelong(minReadingAddress, 399);
  // EEPROMWritelong(rangeAddress, 71);
  // EEPROMWritelong(testMinutesAddress, 30);
  // EEPROMWritelong(testSecondsAddress, 23);
  // EEPROMWritelong(testDurationAddress, 60);
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
      lcd.print ("Range:");
      lcd.setCursor(6, 0);
      lcd.print (range);
      lcd.setCursor(11, 0);
      lcd.print (testMinutes);
      lcd.print(":");
      lcd.print(testSeconds);        
      lcd.setCursor(0,1);
      lcd.print("Max:");
      lcd.setCursor(4,1);
      lcd.print(maxReading);
      lcd.setCursor(9,1);
      lcd.print("Min:");
      lcd.print(minReading);
      break;     
    case 99:
      lcd.setCursor(0,0);
      lcd.print ("-> Fog Reading   ");//extra space to clear screen without lcd.clear()
      lcd.setCursor(0,1);
      lcd.write(toggleSwitch);
      lcd.setCursor(2,1);
      lcd.print (sensorReading);// sensorReading2 is used for sensorSimulator /sensorReading is used for data from sensors****    
      lcd.setCursor(5,1);
      lcd.print (" lumen      ");//extra space to clear screen without lcd.clear()
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
void calibration(int interval, int calibProcess){//calibration cycle function
  int maxValue;
  int minValue;
  int counter =0;
  int start;
  int end;
  long startTime;
  int previousValue;
  int currentValue;
  long currentTime;
  long previousTime;
  int tempvalue=0;
  int tempValue2 =0;
  int counter2 = 0;
  int fogMovAvg;
  int movAvgStartCounter=0; 
  lcd.setBacklight(TEAL);
  sensorValueReading();
  startTime = millis();
  start = sensorReading;// change this to sensorReading for actual sensor readings
  end  = start -10;// hardcoded number for 10 less than the final value . end value not relevent with new movingAvg calibration cycle
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
  Serial.print("\t");
  Serial.print("MovingAverage");
  Serial.print("\t");
  Serial.print("Difference");
  Serial.print("\t");
  Serial.print("Counter");
  Serial.println("\t");
  while(calibProcess == 1){
    sensorValueReading();
    //sensorReading = simulatorValues[newcounter];
    currentValue = sensorReading;// change this to sensorReading for actual sensor readings
    fogMovAvg = sensorValues.reading(currentValue);
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
    Serial.print("\t");
    Serial.print(fogMovAvg);
    Serial.print("\t");
    Serial.print(abs(currentValue - fogMovAvg));
    Serial.print("\t");
    Serial.print(movAvgStartCounter);
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
    movAvgStartCounter++;  
    previousValue = currentValue;
    if(movAvgStartCounter >= 100){
      if (abs(currentValue - fogMovAvg)  < 2){                   
        Serial.print("Moving average:");
        Serial.println(fogMovAvg);
        Serial.print("diff:");
        Serial.println(fogMovAvg -currentValue);
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
}
void fogTest(int interval , int foglevel, int range){// fogtest function.
  //percentage logic here
  // int analogValue = analogRead(A0);
  // int analogValue1 = analogRead(A2);
  testDuration =EEPROMReadlong(testDurationAddress);
  int limit;
  int upperlimit;
  int lowerlimit;
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
    currentTime = millis();
    tempTime = (currentTime - startTime)/60000;
    // Serial.println(startTime);
    // Serial.println(tempTime);
    // Serial.println(currentTime);
    // Serial.println(tempTime);
    if(tempTime > testDuration){
      fogTestStatus = 0;
      Serial.println("Test Finished");
    }
  }
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
            Serial.println("Interval setup");
            break;
          case 2:            
            //lcd.clear();
            incrementSetup();
            break;
          case 3:            
            //lcd.clear();
            testDurationSetup();
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
void displayView(String menu[],int menulength, int menuloc){// function to display various Menuview option
  // int line1x;
  // int line1y;
  // int line2x;
  // int line2y;
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
void manualSolenoidSwitch(){//function to manual control Solenoid 
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
void intervalSetup(){// function to set the interval value for the tests.
  int tempInterval;
  int timeinmilis;
  int tempIncrement = 1; //increment;// change that to increment if you want to drive the interval increment by increment variable
  tempInterval = EEPROMReadlong(intervalAddress) /1000;
  int y = 1 ;// Variable to drive the while loop listening to button presses.
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
    // Serial.println("test 680");
    if(buttons & BUTTON_LEFT && tempInterval >0){
      tempInterval= tempInterval-tempIncrement;
      // Serial.println("test 684");
    }else if(buttons & BUTTON_RIGHT && tempInterval < 100){
      tempInterval= tempInterval+tempIncrement;
      // Serial.println("test 687");
    }else if(buttons & BUTTON_SELECT){
      Serial.println(tempInterval);
      timeinmilis = tempInterval*1000;
      EEPROMWritelong(intervalAddress,timeinmilis);
      // interval = EEPROMReadlong(intervalAddress);
      // Serial.println("test 691");
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
      // Serial.println("test 696");
    }
  }
}
void fogLevelSetup(int x ){// function to set the fog level.
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
    // Serial.println("test 588");
    delay(300);
    if(buttons & BUTTON_LEFT && tempFogLevel >0){
      tempFogLevel= tempFogLevel-10;
      // Serial.println("test 592");
    }else if(buttons & BUTTON_RIGHT && tempFogLevel < 100){
      tempFogLevel= tempFogLevel+10;
      // Serial.println("test 595");
    }else if(buttons & BUTTON_SELECT){
      Serial.println(tempFogLevel);
      EEPROMWritelong(foglevelAddress,tempFogLevel);
      fogLevel = EEPROMReadlong(foglevelAddress);
      // Serial.println("test 600");
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
void incrementSetup(){// function to set the increment values. 
  int tempIncrement;
  tempIncrement = EEPROMReadlong(incrementAddress);
  int y = 1 ;// Variable to drive the while loop listening to button presses.
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
    // Serial.println("test 763");
     if(buttons & BUTTON_LEFT && tempIncrement >0){
      tempIncrement= tempIncrement-1;
      // Serial.println("test 767");
    }else if(buttons & BUTTON_RIGHT && tempIncrement < 150){
      tempIncrement= tempIncrement+1;
      // Serial.println("test 770");
    }else if(buttons & BUTTON_SELECT){
      Serial.println(tempIncrement);
      EEPROMWritelong(incrementAddress,tempIncrement);
      // Serial.println("test 774");
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
      // Serial.println("test 779");
    }
  }
}
void testDurationSetup(){//function to set the test duration.
  int tempDuration = EEPROMReadlong(testDurationAddress);
  // int tempDuration = tempValue;
  int x = 1;// Variable to drive the while loop listening to button presses.
  lcd.clear();
  while(x == 1){
    lcd.setCursor(0, 1);
    lcd.print("<Duration: ");
    lcd.setCursor(10, 1);
    lcd.print(tempDuration);
    lcd.setCursor(13, 1);
    lcd.print("min>");
    delay(300);
    uint8_t buttons = lcd.readButtons();
    // Serial.println("test 857");
  
    if(buttons & BUTTON_LEFT && tempDuration > 0){
      tempDuration= tempDuration-5;
      // Serial.println("test 861");
        // if(tempDuration < 0){
        //   tempDuration = 0;
        //   lcd.clear();
        // }
    }else if(buttons & BUTTON_RIGHT && tempDuration < 300 ){ //the Duration is limited to max of 5hrs i.e 300 mins
      tempDuration= tempDuration+5;
      // Serial.println("test 770");
    }else if(buttons & BUTTON_SELECT){
      Serial.println(tempDuration);
      // tempValue = tempDuration*1000;
      // Serial.println(tempValue);
      EEPROMWritelong(testDurationAddress,tempDuration);
      // Serial.println("test 774");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Duration: ");
      lcd.print(tempDuration);
      lcd.println("    ");
      lcd.setCursor(0, 1);
      lcd.print("Press ");
      lcd.setCursor(6,1);
      lcd.write(2);
      lcd.setCursor(7, 1);
      lcd.println(" to exit  ");
      x = 0;
    }else if(buttons & BUTTON_DOWN){
     
      // Serial.println("test 779");
    }
  }
}












