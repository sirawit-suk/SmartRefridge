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

#include <Adafruit_Fingerprint.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#define green_Light 13
#define red_Light 12
#define ring A0

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

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

//int mode = 0; // 0 = main, 1 = enroll 2 = scan finger 
unsigned long now = 0;
unsigned long sec = 0; 
unsigned long presec = 0;

String id_Student = "62010948"; 
String id_Box = "";
int checkPass = 0;
bool waitState = true;

void setup()  
{
  lcd.init();            // Turn off light, initial of LCD 
  lcd.backlight();       // Turn on light
  
  Print("Hello, Pop!",2,0);
  Print("FingerPrint Scan",0,1);
  
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

void loop()                     
{
  now = millis(); 
  sec = now/1000;
  digitalWrite(red_Light,HIGH); 
  if(getFingerprintIDez() == 1){ 
    Unlock();
  }else if(getFingerprintIDez() == -1){
    Serial.println(getFingerprintIDez());
    Fail();
  }else if(sec!= presec && waitState == true){
    Waiting();
    presec = sec;
  }
  
  char pressKey = keypad.getKey(); //getKey will get NO_KEY as default

  if (pressKey != NO_KEY){ //Check if something press or not...
    Serial.println(pressKey);
    waitState = false;
    if(pressKey == '#' || pressKey == 'A' || pressKey == 'B' || pressKey == 'C' || pressKey == 'D'){//define invalid keys //whichKey == '*'        
      Reset();
      
    }else{
      id_Box += pressKey;
      if(id_Student.length() > 8){
        Reset();
      }else{
        ClearPrint(id_Box,1,0);
        Serial.println(id_Box);
      }
    }  
    if(pressKey == id_Student[checkPass]){
      checkPass++;
    }
    if(checkPass == 8){
      Unlock();  //checkPass = 0 already in function
    }


//    switch(pressKey){
//      case '0': Serial.println("mode default");
//                break;
//      case '1': Serial.println("mode enroll");
//                break;
//      case '2': Serial.println("mode scan finger");
//                break;
//      default: Serial.println("mode default");
//    }
  }

  
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -3; //NO Detection

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -2; //NO Convert

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1; //Invalid
  return 1; //Pass
}

void GreenBlink(){
  Serial.println("GREEN ON"); 
  digitalWrite(red_Light,LOW);
  digitalWrite(green_Light,HIGH);

  PassBeep();
  delay(4000); // old is 1200 millisec
  digitalWrite(green_Light,LOW);
  digitalWrite(red_Light,HIGH); 
}

void Unlock(){
  waitState = true;
  checkPass = 0;
  ClearPrint("Access Granted!",1,0);
  Print("Welcome,62010948",0,1);
  Serial.println("Welcome! 62010948.");
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
void Reset(){
  waitState = true;
  checkPass = 0;
  id_Box = "";
  ClearPrint("Clear!",1,0);
  Serial.println("Clear");
  delay(500);
}

void ClearPrint(String text,int pos,int line){
  lcd.clear();
  lcd.setCursor(pos,line);
  lcd.print(text);
}
void Print(String text, int pos,int line){
  lcd.setCursor(pos,line);
  lcd.print(text);
}
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
