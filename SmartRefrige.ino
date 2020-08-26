/*************************************************** 
  This is an example sketch for our optical Fingerprint sensor

  Designed specifically to work with the Adafruit BMP085 Breakout 
  ----> http://www.adafruit.com/products/751

  These displays use TTL Serial to communicate, 2 pins are required to 
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/
// On Leonardo/Micro or others with hardware serial, use those! #0 is green wire, #1 is white
// uncomment this line:
// #define mySerial Serial1

// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire) //Orange 
// pin #3 is OUT from arduino  (WHITE wire) //White
// comment these two lines if using hardware serial

//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
//////////////////////////////////// DOC README //////////////////////////////////////////////
/*
  0-9 is number key
  * is cancel key
  # is enter key
  ABCD is ... key

  Press 'A' to enter admin mode
  ESL Password : 603603

  Press '*' to cancel everything back to waiting state
  Press '#' to confirm password 

*/
//////////////////////////////////// LIBRARY //////////////////////////////////////////////
#include <Adafruit_Fingerprint.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

/////////////////////////////// DEFINE VARIABLES ///////////////////////////////////////////
#define green_Light 13
#define red_Light 12
#define ring A0

///////////// KEYPAD /////////////////
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {11 ,10, 9, 8}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 6, 5, 4}; //connect to the column pinouts of the keypad


//////////////////////////////////// MAPPING /////////////////////////////////////////////
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

/////////////////////////////////// VARIABLES ///////////////////////////////////////////

unsigned long now = 0;
unsigned long sec = 0; 
unsigned long presec = 0;

String ESL_password = "603603"; 
String blank_Box = "";
bool waitState = true;

char pressKey = NO_KEY;

uint8_t mode = 2; // 0 = main, 1 = enroll 2 = scan finger 

//////////////////////////////////// SET UP //////////////////////////////////////////////

void setup()  
{
  lcd.init();            // Turn off light, initial of LCD 
  lcd.backlight();       // Turn on light
  
  Print("Hello, Pop!",2,0);
  Print("FingerPrint Scan",0,1);

  //init pinmode ++
  pinMode(green_Light,OUTPUT);
  pinMode(red_Light,OUTPUT);
  pinMode(ring,OUTPUT);
  
  Serial.begin(9600);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(500);
  Serial.println("");
  Serial.println("Welcome to Pop finger print");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  while(1){ //Check sensor until Found Sensor 
    if (finger.verifyPassword()) {
      ClearPrint("Sensor found!",1,0);
      Serial.println("Found fingerprint sensor!");
      delay(1500);
      break;
    } else {
      ClearPrint("Did't found",2,0);
      Print("sensor!",4,1);
      Serial.println("Did not find fingerprint sensor :(");
      delay(100);
    }
  }
    

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please, 'enroll' first.");
  } 
  else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains "); 
    Serial.print(finger.templateCount); 
    Serial.println(" templates");
  }
}


//////////////////////////////////// MAIN LOOP //////////////////////////////////////////////
void loop()                     
{
  ClockSystem();
  
  CheckFingerSystem();
  
  SelectMode();
}
/////////////////////////////////////// MODE //////////////////////////////////////////////
void SelectMode(){
  pressKey = keypad.getKey(); //getKey will get NO_KEY as default

  switch(mode){
    case 0: DefaultMode();       //normalMode
            break;
    case 1: EnterPassMode();
            break;
    case 2: AdminMode();
            break;
  } 
}

//////////////////////////////////// CASE BY CASE //////////////////////////////////////////////
void DefaultMode(){
  waitState = true; 
  if (pressKey != NO_KEY){ //Check if something press or not... 
    if(pressKey == 'A'){          //normalMode   
      mode = 1;

    //Nextstate
      InitModel_1();
      return;
    }
  }
}
void EnterPassMode(){

  if (pressKey != NO_KEY){  
    if(pressKey == '#' && blank_Box == ESL_password){   //Password ESL: 603603
      mode = 2;
    
    //Nextstate
      InitModel_2();
      return;
    }
    else if(pressKey == '*'){
      BackToFirst();
      return;
    }
      
    if(pressKey == 'B' || pressKey == 'C' || pressKey == 'D'){        
      Reset();
      InitModel_1();
      
    }else{
      if(pressKey != 'A') 
        blank_Box += pressKey;
        
      if(blank_Box.length() > ESL_password.length()|| (pressKey == '#' && blank_Box != ESL_password)){
        Wrong();
        InitModel_1();
      }else{
        Print(blank_Box,5,1);
        Serial.println(blank_Box);
      }
    }
     
  }
}
void AdminMode(){
  if (pressKey != NO_KEY){ //Check if something press or not... 
    switch(pressKey){
      case '1': Add();
                break;
      case '2': Delete();
                break;
      case '3': Unlock();
                break;
      case '*': BackToFirst();
                break;
    }
  }

}
/////////////Mainfuction//////////////
void Add(){
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  pressKey = NO_KEY;
  ClearPrint("Type ID (1-127):",0,0);
  
  String temp = "";
  
  while(pressKey != '#'){
    pressKey = keypad.getKey();
    if(pressKey != NO_KEY && pressKey != '#' && pressKey != 'A' && pressKey != 'B' && pressKey != 'C' && pressKey != 'D' && pressKey != '*'){
       temp += pressKey;
       Print(temp,7,1);
       Serial.println(temp);
    }
    else if (pressKey == '*'){
       temp.remove(temp.length()-1);
       ClearPrint("Type ID (1-127):",0,0);
       Print(temp,7,1);
       Serial.println(temp);
    }
  }
  
  uint8_t id = temp.toInt();
  Serial.println(id);
  if (id == 0) {// ID #0 not allowed, try again!
     ClearPrint("ID #0 Not allow",0,0);
     Print("Try again",4,1);
     Serial.println("Error: ID #0 not allowed, try again");
     return;
  }
  while (!getFingerprintEnroll(id));
}
void Delete(){
  
}
///////////////Subfunction////////////
void InitModel_1(){
  waitState = false;             
  ClearPrint("ESL Password : ",1,0);
}
void InitModel_2(){
  waitState = false;      
  ClearPrint("ESL Member ^^ ",2,0);  
  delay(1000);     
  ClearPrint("1 Add 2 Delete ",1,0);
  Print("3 Unlock: ",1,1);
}

//////////////////////////////////// ALL SYSTEM //////////////////////////////////////////////

void ClockSystem(){
  now = millis(); 
  sec = now/1000;
}

void CheckFingerSystem(){
  int checkFinger = getFingerprintIDez();
  
  if(checkFinger == 1){             //PASS
    Unlock();
  }else if(checkFinger == -1){      //FAIL
    Fail();
  }else if(sec!= presec && waitState == true){    //WAIT
    Waiting();
    presec = sec;
  }
}

////////////////////////////////// FINGERPRINT SYSTEM ////////////////////////////////////////
///////////FINGER PRINT////////////
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -3; //NO Detection

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -2; //NO Convert

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1; //Invalid
  
  return 1; //Pass
}
///////////////ENROLL////////////

uint8_t getFingerprintEnroll(uint8_t id) {

  int p = -1;
  Serial.print(F("Waiting for valid finger to enroll as #")); 
  Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println(F("Image taken"));
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(F("."));
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println(F("Communication error"));
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println(F("Imaging error"));
      break;
    default:
      Serial.println(F("Unknown error"));
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println(F("Image converted"));
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println(F("Image too messy"));
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println(F("Communication error"));
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println(F("Could not find fingerprint features"));
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println(F("Could not find fingerprint features"));
      return p;
    default:
      Serial.println(F("Unknown error"));
      return p;
  }
  
  Serial.println(F("Remove finger"));
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print(F("ID ")); 
  Serial.println(id);



  
  p = -1;
  Serial.println(F("Place same finger again"));
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println(F("Image taken"));
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(F("."));
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println(F("Communication error"));
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println(F("Imaging error"));
      break;
    default:
      Serial.println(F("Unknown error"));
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println(F("Image converted"));
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println(F("Image too messy"));
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println(F("Communication error"));
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println(F("Could not find fingerprint features"));
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println(F("Could not find fingerprint features"));
      return p;
    default:
      Serial.println(F("Unknown error"));
      return p;
  }
  
  // OK converted!
  Serial.print(F("Creating model for #"));  
  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println(F("Prints matched!"));
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println(F("Communication error"));
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println(F("Fingerprints did not match"));
    return p;
  } else {
    Serial.println(F("Unknown error"));
    return p;
  }   
  
  Serial.print(F("ID ")); 
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println(F("Stored!"));
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println(F("Communication error"));
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println(F("Could not store in that location"));
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println(F("Error writing to flash"));
    return p;
  } else {
    Serial.println(F("Unknown error"));
    return p;
  }   
}

////////////////////////////////// LOCK SYSTEM ////////////////////////////////////////
void Unlock(){
  waitState = true;
  ClearPrint("Access Granted!",1,0);
  Print("Need a drink?",2,1);
  Serial.println("Need a drink?");
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  GreenBlink();
}
void Fail(){
  ClearPrint("Invalid!",4,0);
  Print("Please Try again",0,1);
  FailBeep();
  Serial.println("Invalid! Please try again.");
}


////////////////////////////////// KEYPAD SYSTEM ////////////////////////////////////////
void Waiting(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("ESL Refrigerator");
  if(sec%3 == 0){
    lcd.setCursor(1,1);
    lcd.print("  .Waiting.    ");
  }
  else if(sec%3 == 1){
    lcd.setCursor(1,1);
    lcd.print(" ..Waiting..   ");
  }
  else if(sec%3 == 2){
    lcd.setCursor(1,1);
    lcd.print("...Waiting...  ");
  }  
}
void Reset(){
  blank_Box = "";
  ClearPrint("Clear!",5,0);
  Serial.println("Clear");
  delay(500);
}
void Wrong(){
  blank_Box = "";
  ClearPrint("Wrong! Try again",0,0);
  Serial.println("Wrong! Try again");
  delay(1000);
}
void BackToFirst(){
  mode = 0;
  blank_Box = "";
  delay(500);
}

////////////////////////////// KEYPAD FUCNTION ////////////////////////////////////
void ClearPrint(String text,int pos,int line){
  lcd.clear();
  lcd.setCursor(pos,line);
  lcd.print(text);
}
void Print(String text, int pos,int line){
  lcd.setCursor(pos,line);
  lcd.print(text);
}

////////////////////////////////// LIGHT ////////////////////////////////////////
void GreenBlink(){
  Serial.println("GREEN ON"); 
  digitalWrite(red_Light,LOW);
  digitalWrite(green_Light,HIGH);

  PassBeep();
  //delay(4000); //DON'T Forget
  digitalWrite(green_Light,LOW);
  digitalWrite(red_Light,HIGH); 
}

/////////////////////////////// SOUND ALERT ////////////////////////////////////////
void PassBeep(){
  digitalWrite(ring,HIGH);
  delay(265);
  digitalWrite(ring,LOW); 
  delay(250);
  
  digitalWrite(ring,HIGH);
  delay(150);
  digitalWrite(ring,LOW); 
  delay(150);
  digitalWrite(ring,HIGH);
  delay(150);
  digitalWrite(ring,LOW); 
  delay(200);
  
  digitalWrite(ring,HIGH);
  delay(200);
  digitalWrite(ring,LOW); 
  delay(200);
  digitalWrite(ring,HIGH);
  delay(200);
  digitalWrite(ring,LOW);   //150
}

void FailBeep(){
  digitalWrite(ring,HIGH);
  delay(180);
  digitalWrite(ring,LOW); 
  delay(180);
  digitalWrite(ring,HIGH);
  delay(180);
  digitalWrite(ring,LOW); 
}
































/*
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  Serial.println(p);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  // found a match!
  Serial.print("Found ID #"); 
  Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); 
  Serial.println(finger.confidence); 

  return finger.fingerID;
}
*/
















//if (pressKey != NO_KEY){ //Check if something press or not... 


/*
  digitalWrite(red_Light,HIGH); 
  Serial.print("RETURN : ");
  Serial.print(checkFinger);
  Serial.print("   Found ID # "); 
  Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); 
  Serial.println(finger.confidence);
*/
