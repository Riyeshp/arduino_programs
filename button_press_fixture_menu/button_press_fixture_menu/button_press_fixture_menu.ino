#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <math.h>
#include <AccelStepper.h>

//array length calculation
#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))
template< typename T, size_t N > size_t ArraySize (T (&) [N]){ return N; }

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

//AccelStepper myStepper(AccelStepper::DRIVER, 9, 8);
AccelStepper myStepper(1,8,9);

// Variables
int menu = 0; // menu value to prevent screen refresh
String screens[]= {"->Settings", "->Test Status"};
String settingScreens[]={"<SolenoidSelect>", "<Delay         >", "<Limit         >","<Limit switch  >","<Counter reset >"};
String testScreens[]={"<Limit Cycle   >","<Manual Cycle  >"};
String solenoidList[]={"A","B","A&B","A/B" };
int sensorReading;//Actual sensor Reading data
int toggleSwitch;// it control the landing page up and down arrows.****
bool manualSwitchStatus = false;//ON OFF button for manually operating the solenoid
int fogTestStatus = 0; // variable to drive loop function inside  fogtest function.
int pinOut1 = 6; //sensor pin
int pinOut2 = 7; //sensor pin
//EEPROM Variables
int delayTime;
int delayTimeAddress=5;//Address location on EEPROM memory
int solenoidSelection;
int solenoidSelectionAddress = 15;//Address location on EEPROM memory
// int step;
// int stepAddress=25;
long limit;
long limitAddress=45;//Address location on EEPROM memory
// long counter;
long count=0;
long countAddress=55;//Address location on EEPROM memory
bool counterSwitch;
bool counterSwitchAddress =35;//Address location on EEPROM memory





//variables for start function
int counter=0;
int hundred=0;//counter
int Time=0;
int timer=0; 
int Reset=1;
long address=0;
long testvalue =0;


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
  // myStepper.setMaxSpeed(30000);//maxspeed is hardcoded
  // myStepper.setSpeed(speed);
  // myStepper.setAcceleration(3500);//acceleration is hardcoded
  lcd.begin(16, 2);
  lcd.setBacklight(WHITE);
  lcd.createChar(0, customChar1);
  lcd.createChar(1, customChar2);
  lcd.createChar(2, arrowdown);
  delayTime = EEPROMReadlong(delayTimeAddress);
  // step = EEPROMReadlong(stepAddress);
  limit =EEPROMReadlong(limitAddress);
  counterSwitch = EEPROMReadlong(counterSwitchAddress);
  count = EEPROMReadlong(countAddress);
  solenoidSelection = EEPROMReadlong(solenoidSelectionAddress);
  pinMode(pinOut1, OUTPUT);
  digitalWrite(pinOut1, LOW);
  pinMode(pinOut2, OUTPUT);
  digitalWrite(pinOut2, LOW);
  Serial.print("Solenoid Selected: ");
  Serial.println(solenoidList[solenoidSelection]);
  Serial.print("Delay: ");
  Serial.print(delayTime);
  Serial.println("ms");
  Serial.print("Counter Limit: ");
  Serial.println(limit);
  Serial.print("Counter Switch: ");
  Serial.println(counterSwitch);
  Serial.print("Count: ");
  Serial.println(count);
  // start();//if startup is required uncomment this line
}
void loop() {
  uint8_t buttons = lcd.readButtons();
  // below lines of codes are used  whenever values had to be written into the EEPROM addresses manually DONT delete!!
  // EEPROMWritelong(delayTimeAddress,5000);
  //EEPROMWritelong(stepAddress,175);
    // EEPROMWritelong(limitAddress, 100000);
  //  EEPROMWritelong(counterSwitchAddress, true);
  // EEPROMWritelong(countAddress, 5);
  // EEPROMWritelong(solenoidSelectionAddress, 2);

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
      lcd.print (count);// sensorReading2 is used for sensorSimulator /sensorReading is used for data from sensors****    
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
void limitCycle(){
  counterSwitch= EEPROMReadlong(counterSwitchAddress);
  int tempCount=EEPROMReadlong(countAddress);
  lcd.setBacklight(BLUE);
  lcd.clear();
  while(counterSwitch ==1){
    uint8_t buttons = lcd.readButtons();
    switch(solenoidSelection){
      case 0: //A solenoid only.
        digitalWrite(6, HIGH);
        lcd.setCursor(12, 0);
        lcd.print("  6H");
        delay(delayTime);
        digitalWrite(6, LOW);
        lcd.setCursor(12, 0);
        lcd.print("  6L");
        delay(delayTime);
        break;
      case 1://B solenoid only.
        digitalWrite(7, HIGH);
        lcd.setCursor(12, 0);
        lcd.print("7H  ");
        delay(delayTime);
        digitalWrite(7, LOW);
        lcd.setCursor(12, 0);
        lcd.print("7L  ");
        delay(delayTime);
        break;
      case 2://A and B solenoid together based on the delay time.
        digitalWrite(6, HIGH);
        digitalWrite(7, HIGH);
        lcd.setCursor(12, 0);
        lcd.print("7H6H");
        delay(delayTime);
        digitalWrite(6, LOW);
        digitalWrite(7, LOW);
        lcd.setCursor(12, 0);
        lcd.print("7L6L");
        delay(delayTime);   
        break;
      case 3://A and B solenoid alternatively based on the delay time.
        digitalWrite(6, HIGH);
        lcd.setCursor(12, 0);
        lcd.print("  6H");
        delay(delayTime);
        digitalWrite(6, LOW);
        digitalWrite(7, HIGH);
        lcd.setCursor(12, 0);
        lcd.print("7H6L");
        delay(delayTime);
        digitalWrite(7, LOW);
        lcd.setCursor(12, 0);
        lcd.print("7L  ");
        break;
    }
    if(hundred > 100){
     count=EEPROMReadlong(countAddress);
     count = count+100;
     EEPROMWritelong(countAddress, count);
     hundred=0;
     lcd.println(" Count written to memory ");
     lcd.clear();
    }
    Serial.print("count:");
    Serial.print(tempCount);
    Serial.println("");
    Serial.print("Limit:");
    Serial.print(limit);
    Serial.println("");
    if(tempCount > limit){
      counterSwitch = 0;
      EEPROMWritelong(counterSwitchAddress, counterSwitch);
      Serial.println("Counter reached set limit");
      Serial.println("Limit switch turned OFF");
    }
    if (buttons & BUTTON_DOWN){
      counterSwitch = 0;
      EEPROMWritelong(counterSwitchAddress, counterSwitch);
      Serial.println("Cycle Stopped manually");
      lcd.setBacklight(YELLOW);
    }
    lcd.clear();
    hundred=hundred+1;
    lcd.setCursor(0, 0);
    lcd.print("Cycle Count:");
    lcd.setCursor(0, 1);
    // lcd.print(">");
    // lcd.setCursor(0, 2);
    lcd.print((count+hundred)); 
    tempCount++;   
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
      if(menu == settingScreens || menu == testScreens){
        lcd.setBacklight(VIOLET);
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
            solenoidSelectionSetup();
            Serial.println("Solenoid setup");
            break;
          case 1:
            delayTimeSetup();
            Serial.println("Delay time setup");
            break;
          case 2:            
            limitSetup();
            Serial.println("Limit setup");
            break;
          case 3:            
            limitSwitchSetup();
            Serial.println("Limit switch setup");
            break;
          case 4:
            counterReset();
            Serial.println("Counter reset setup");
            break;

            
        }
      }else if (menu == testScreens){
        // Serial.println("test 410");
        switch(menuloc){
          case 0:
            // fogTest(interval,fogLevel, range);
            lcd.setCursor(0,1);
            if(counterSwitch == 0){
              Serial.println("Counter Switch turned OFF");
              lcd.setCursor(0,0);
              lcd.println("Limit switch OFF      ");
              lcd.setCursor(1, 0);
              lcd.print(">Limit:");
              lcd.println(limit);
              break;
            }
            limitCycle();
            break;
          case 1:
            manualCycle();
            // lcd.setCursor(0,1);
            // lcd.println("testing check     ");
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
void delayTimeSetup(){// function to set the interval value for the tests.
  int tempDelayTime;
  int timeinmilis;
  int tempIncrement = 1; //increment;// change that to increment if you want to drive the interval increment by increment variable
  tempDelayTime = EEPROMReadlong(delayTimeAddress)/1000;
  int y = 1 ;// Variable to drive the while loop listening to button presses.
  int z = 2;
  lcd.clear();
  while(y == 1){
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("<Delay:");
    lcd.setCursor(7, 1);
    lcd.print(tempDelayTime);
    lcd.setCursor(9, 1);
    lcd.print("sec   >");
    delay(500);
    uint8_t buttons = lcd.readButtons();
    // Serial.println("test 680");
    if(buttons & BUTTON_LEFT && tempDelayTime >0){
      tempDelayTime= tempDelayTime-1;
      // Serial.println("test 684");
    }else if(buttons & BUTTON_RIGHT && tempDelayTime < 60){
      tempDelayTime= tempDelayTime+1;
      // Serial.println("test 687");
    }else if(buttons & BUTTON_SELECT){
      Serial.println(tempDelayTime);
      EEPROMWritelong(delayTimeAddress,tempDelayTime*1000);
      // interval = EEPROMReadlong(intervalAddress);
      // Serial.println("test 691");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Delay:");
      lcd.print(tempDelayTime); 
      lcd.setCursor(8, 0);
      lcd.println("sec     ");
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
void solenoidSelectionSetup(){// function to set the fog level.
  int tempSolenoidList;
  tempSolenoidList = solenoidSelection;
  lcd.clear();
  int y = 1 ;
  while(y == 1){
    lcd.setCursor(0, 1);
    lcd.print("<Selection:");
    lcd.setCursor(11, 1);
    lcd.print(tempSolenoidList + 1);
    lcd.setCursor(14, 1);
    lcd.print(" >");
        uint8_t buttons = lcd.readButtons();
    // Serial.println("test 588");
    delay(500);
    if(buttons & BUTTON_LEFT && tempSolenoidList > 0){
      tempSolenoidList= tempSolenoidList-1;
      // Serial.println("test 592");
    }else if(buttons & BUTTON_RIGHT && tempSolenoidList < 3){
      tempSolenoidList= tempSolenoidList+1;
      // Serial.println("test 595");
    }else if(buttons & BUTTON_SELECT){
      Serial.println(tempSolenoidList);
      EEPROMWritelong(solenoidSelectionAddress,tempSolenoidList);
      solenoidSelection = EEPROMReadlong(solenoidSelectionAddress);
      // Serial.println("test 600");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Selection:");
          lcd.setCursor(10, 0);
          lcd.print(solenoidList[tempSolenoidList]);
          lcd.setCursor(0, 1);
          lcd.print("Press ");
          lcd.setCursor(6,1);
          lcd.write(2);
          lcd.setCursor(7, 1);
          lcd.println(" to exit  ");
          break;
      y = 0;
      delay(500);
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
      lcd.print("Limit:");
      lcd.setCursor(6, 0);
      lcd.print(tempLimit);
      lcd.setCursor(13, 0);
      lcd.print("   ");
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
void start(){//function for startup timer
  while (timer<10){
    lcd.clear();
    lcd.print("Reset? :");
    if (Reset==1){lcd.print("no");}
    if (Reset==0){lcd.print("yes");}
    uint8_t buttons = lcd.readButtons();
    if (buttons & BUTTON_UP){Reset=0;}
    if (buttons & BUTTON_DOWN){Reset=1;}
    timer=((micros()-Time)/1000000);
    lcd.print("    ");
    lcd.print((10-timer));
    //lcd.println(Reset);
    delay(150);
    if (Reset==0){
      count=0;
      EEPROMWritelong(address, count);
    }
    if(Reset==1){count=EEPROMReadlong(0);}
  } 
}
void limitSwitchSetup(){
    counterSwitch = !counterSwitch;
  if(counterSwitch){
    lcd.setCursor(0,0);
    lcd.println("<Limit switch  >");
    lcd.setCursor(0,1);
    lcd.println("ON              ");
    EEPROMWritelong(counterSwitchAddress, counterSwitch);
  }else{
    lcd.setCursor(0,0);
    lcd.println("<Limit switch  >");
    lcd.setCursor(0,1);
    lcd.println("OFF             ");
    EEPROMWritelong(counterSwitchAddress, counterSwitch);

  }
  delay(1000);

}
void counterReset(){
  EEPROMWritelong(countAddress, 0);
  Serial.println("Counter reset Done");
  lcd.setCursor(0,0);
  lcd.print("Reset done");
}
void manualCycle(){
  
  EEPROMWritelong(counterSwitchAddress, true);
  counterSwitch= EEPROMReadlong(counterSwitchAddress);
  int tempCount=EEPROMReadlong(countAddress);
  lcd.setBacklight(BLUE);


  while(counterSwitch ==1){
    uint8_t buttons = lcd.readButtons();
    switch(solenoidSelection){
      case 0: //A solenoid only.
        digitalWrite(6, HIGH);
        lcd.setCursor(12, 0);
        lcd.print("  6H");
        delay(delayTime);
        digitalWrite(6, LOW);
        lcd.setCursor(12, 0);
        lcd.print("  6L");
        delay(delayTime);
        break;
      case 1://B solenoid only.
        digitalWrite(7, HIGH);        
        lcd.setCursor(12, 0);
        lcd.print("7H  ");
        delay(delayTime);
        digitalWrite(7, LOW);
        lcd.setCursor(12, 0);
        lcd.print("7L  ");
        delay(delayTime);
        break;
      case 2://A and B solenoid together based on the delay time.
        digitalWrite(6, HIGH);
        digitalWrite(7, HIGH);
        lcd.setCursor(12, 0);
        lcd.print("7H6H");
        delay(delayTime);
        digitalWrite(6, LOW);
        digitalWrite(7, LOW);
        lcd.setCursor(12, 0);
        lcd.print("7L6L");
        delay(delayTime);
        break;
      case 3://A and B solenoid alternatively based on the delay time.
        digitalWrite(6, HIGH);
        lcd.setCursor(12, 0);
        lcd.print("  6H");
        delay(delayTime);
        digitalWrite(6, LOW);
        digitalWrite(7, HIGH);
        lcd.setCursor(12, 0);
        lcd.print("7H6L ");
        delay(delayTime);
        digitalWrite(7, LOW);
        lcd.setCursor(12, 0);
        lcd.print("7L  ");
        delay(delayTime);
        break;
    }

   
    if(hundred > 100){
     count=EEPROMReadlong(countAddress);
     count = count+100;
     EEPROMWritelong(countAddress, count);
     hundred=0;
     lcd.println(" Count written to memory ");
     Serial.println("Count written to memory ");
     lcd.clear();
    }
    Serial.print("count:");
    Serial.print(tempCount);
    Serial.println("");
    Serial.print("Limit:");
    Serial.print(limit);
    Serial.println("");

    lcd.clear();
    hundred=hundred+1;
    lcd.setCursor(0, 0);
    lcd.print("Cycle Count:");
    lcd.setCursor(0, 1);
    // lcd.print(">");
    lcd.print((count+hundred)); 
    tempCount++;  
    // delay(800);
    if (buttons & BUTTON_DOWN){
      counterSwitch = 0;
      EEPROMWritelong(counterSwitchAddress, counterSwitch);
      Serial.println("Cycle Stopped manually");
      lcd.setBacklight(YELLOW);
    }
  }  
}


