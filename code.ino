//Global for keypad
#include <Keypad.h>

const byte rows=4;
const byte cols=4;
char keymap[rows][cols]={
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[rows]={9,8,7,6};
byte colPins[cols]={5,4,3,2};
Keypad K = Keypad(makeKeymap(keymap), rowPins, colPins, rows, cols);

//-------------------------------------------------------
//Global for LCD
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // Wokwi default address
//--------------------------------------------------------

// Pin Definitions
#define STEP_PIN A0
#define DIR_PIN A1
#define ENABLE_PIN A2
#define MS1_PIN A3
#define MS2_PIN A4
#define MS3_PIN A5

// Wire Feeding Parameters
const float SPOOL_CIRCUMFERENCE = 0.1; // meters
const int STEPS_PER_REV = 200;
const int MICROSTEPS = 16;


//---------------
int operation=0; 
int rollLength=0;
String lenStr="";
String inputStr=""; // Added missing global declaration
 

//relay
 

void setup() {
  
  
   
  lcd.init();
  lcd.backlight();
  lcd.clear();
  initStepper();
}

void loop() {
  if(operation==2){
   lcd.clear();
   lcd.print("Enter cut len :");
   lcd.setCursor(0,1);
   lcd.print("max:");
   lcd.print(intToString(rollLength));
     
   byte ptr=0;
    while(ptr==0){
      char key = K.getKey();
      if (key) {
        if (key == '#') {
          lcd.clear();
          lcd.print("Processing Len");
          lcd.setCursor(0, 1);
          lcd.print(inputStr);
          lcd.print("m");
          delay(1000);
          byte response=1;
          if(stringToInt(inputStr)>=rollLength){response=0;}
 
          lcd.clear();
          if(response==0){
          lcd.clear();
          lcd.print("Not enough Len");
          operation=0;
          inputStr = "";
          
          ptr=1;
          delay(1000);
          }
          
          else{
          int check=driveMotor(stringToInt(inputStr));
          operation=0;
          inputStr = "";
          //lcd.clear();
          /*if(check==1)
          lcd.print("All Done.");
          else lcd.print("Failed. F..");*/
          ptr=1;
          delay(2000);
          }
        }
        else if (key == '*') {
          if (inputStr.length() > 0) {
            inputStr.remove(inputStr.length()-1);
          }
        }
        else {
          inputStr += key;
        }
    
        lcd.setCursor(0, 1);
        lcd.print("            ");
        lcd.setCursor(0,1);
        lcd.print(inputStr);
      }
    }
  }
  else if(operation==1){
    lcd.clear();
    lcd.print("Enter Roll Len:");
    byte ptr=0;
    while(ptr==0) {
      char key=K.getKey();
      if(key){
        if(key=='#'){
          rollLength=stringToInt(lenStr);
          lenStr="";
          lcd.clear();
          lcd.print("Lenght set ");
          lcd.setCursor(0,1);
          lcd.print(rollLength);
          operation=0;
          ptr=1;
          delay(1000);
        }
        else{
          lenStr+=key;
          lcd.setCursor(0,1);
          lcd.print(lenStr);
        }
      } 
    }
  }
  else{
    lcd.clear();
    lcd.print("A.New Roll Len");
    lcd.setCursor(0,1);
    lcd.print("B.Start Cutting");
    char keyO='k';
    while(keyO=='k'){
      char temp=K.getKey();
      if(temp)keyO=temp;
    }
    if(keyO=='A')operation=1;
    else if(keyO=='B'){operation=2;lcd.clear();}
  }
}

 
#define STEP_PIN     10
#define DIR_PIN      11
#define MS1_PIN      12
#define MS2_PIN      13
#define MS3_PIN      A0
#define ENABLE_PIN   A1

#define STEPS_PER_REV      200
#define MICROSTEPS         16
#define SPOOL_CIRCUMFERENCE 0.5

#define RELAY_PIN A3

#define TRIG_PIN A1
#define ECHO_PIN A2  
 
void initStepper() {

 pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
   // 
   pinMode(RELAY_PIN, OUTPUT);
   digitalWrite(RELAY_PIN, LOW);
  //
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(MS1_PIN, OUTPUT);
  pinMode(MS2_PIN, OUTPUT);
  pinMode(MS3_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);

  digitalWrite(MS1_PIN, HIGH);  // 1/8 microstepping total steps/rev=800, 
  digitalWrite(MS2_PIN, LOW);
  digitalWrite(MS3_PIN, LOW);

  digitalWrite(ENABLE_PIN, LOW);  // Enable driver
  digitalWrite(DIR_PIN, HIGH);   // Default direction
}

int driveMotor(float length) {
  if (length <= 0) return 0;
 // assuming circumference =400cm=4000mm. steps/rev is 800,  
 // so one step=4000/800=5mm, as there should be max error of +-5mm,
 //total steps=length in mm divide by 5mm.
  int total_steps =1000*length/5; //bz length is in meter,
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Rolling...");
  if(total_steps<0)total_steps*=-1;

  //as one step need to iterations so we are twice the steps here.
  for (int i = 0; i < total_steps*2; i++) {
    digitalWrite(STEP_PIN, HIGH);
    
    digitalWrite(STEP_PIN, LOW);
     
    lcd.setCursor(0,1);
    double temp=1.0*i/total_steps*100/2;//divide 2 is as for we need to twice the steps.
    lcd.print(temp);lcd.print("%");// Adjust speed here
  }

   
  delay(1000);
  lcd.clear();
  lcd.print("Cutting Start...");
  driveRelay();
  
  lcd.clear();
  lcd.print("Cutting done.");
  delay(500);
  lcd.clear();lcd.print("Cross checking.");
  delay(1000);
  int check=crossCheck();
  if(check){
    lcd.clear();
    lcd.print("Successfully");
    lcd.setCursor(0,1);
    lcd.print("Done. ezz");
    delay(3000);
  }
  else{
    
      lcd.clear();
  lcd.print("Cutting Start...");
  driveRelay();
  //IR Logic.
  lcd.clear();
  lcd.print("Cutting done.");
  delay(500);
  lcd.clear();
  lcd.print("Final check.");
  delay(1000);
  int temp1=crossCheck();
  if(temp1){lcd.clear();lcd.print("Done in 2nd.");delay(1000);}
  else{lcd.clear();
  lcd.print("Cut failed again.");
  delay(1000);
  lcd.setCursor(0,1);
  lcd.print("Do manually.");
  delay(3000);}
  rollLength-=length;
  return 0;
  }

  rollLength-=length;
  return 1;
}

void driveRelay(){
  
    
      digitalWrite(RELAY_PIN, HIGH);
      delay(2000);
      digitalWrite(RELAY_PIN, LOW);
      delay(1000);
      digitalWrite(RELAY_PIN, HIGH);
      delay(2000);
      digitalWrite(RELAY_PIN, LOW);
      
    
}
 
 
int crossCheck(){

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Measure echo duration (µs)
  long duration = pulseIn(ECHO_PIN, HIGH);
  
  // Convert to cm (speed of sound = 343m/s = 0.0343cm/µs)
  float distance=duration *0.0343 / 2;
   
  if(distance<200 && distance>180) return 0;
  else return 1;

}




//----------------------------------
String intToString(int number) {
  if (number == 0) return "0";
  String result = "";
  bool isNegative = false;
  if (number < 0) {
    isNegative = true;
    number = -number;
  }
  while (number > 0) {
    char digit = '0' + (number % 10);
    result = digit + result;
    number /= 10;
  }
  if (isNegative) {
    result = "-" + result;
  }
  return result;
}

int stringToInt(String str) {
  int result = 0;
  for (int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (c >= '0' && c <= '9') {
      result = result * 10 + (c - '0');
    }
    else {
      break;
    }
  }
  return result;
}
