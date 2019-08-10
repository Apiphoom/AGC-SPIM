#include <ARDUINO.h>
#include <SPI.h>
#include <RFID.h>

#define SS_PIN  22
#define RST_PIN 23

#define GOX     0
#define BACKX   1
#define GOY     2
#define BACKY   3

#define PWM1    3
#define PWM2    2
#define INA1    32
#define INB1    33
#define INA2    34
#define INB2    35

#define LIFT_DIRA   31
#define LIFT_DIRB   30
#define LIFT_PWM    4 

#define S1  analogRead(A0)
#define S2  analogRead(A1)
#define S3  analogRead(A2)
#define S4  analogRead(A3)

#define UP      138
#define DOWN    -90

#define STOP {  motorControl(0,0);  }

int t_x,t_y,t_z;
boolean stringComplete = false;
String inputString;
String dataRead;
char action;
int currentX = 1;
int currentY = 0;
int moveAction;
bool runningMode = false;

RFID rfid(SS_PIN, RST_PIN); 


void intiSystem(void);
void track(int trackSpeed, int diffSpeed);
void trackCrossX(int crossCount,int trackSpeed, int diffSpeed, int timeToTurn);
void trackCross2(int crossCount,int trackSpeed, int diffSpeed);
void trackTime(unsigned long timer);

void gotoXY(int x,int y);
void inComming(int x, int y);
void outting(int x,int y);


void setup() {
  intiSystem();
  SPI.begin(); 
  rfid.init();
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  Serial.println("READY");
  sendPos();
}

void loop() {
  serialEvent();
  if (rfid.isCard() && runningMode == false){
    delay(300);
    rfidRead();
    dataRead += "z";
    if (dataRead != "rz"){
      runningMode = true;
      delay(300);
      Serial2.print(dataRead);
      Serial.println(dataRead);
    }else {
      Serial.println("Read Error");
    }
  }
  if (stringComplete){
    if (action == 'i') Serial.print("Incoming goods to ");
    if (action == 'o') Serial.print("outting goods to ");
    Serial.print("\t"); 
    Serial.print("X : ");  Serial.print(t_x); Serial.print("\t"); 
    Serial.print("Y : ");  Serial.println(t_y);
    inputString = "";
    stringComplete= false;
    if (action == 'i'){
      inComming(t_x,t_y);
      action = 0;
      runningMode = false;
    }
    else if (action == 'o'){
      outting(t_x,t_y);
      action = 0;
      runningMode = false;
    }
  }


}

void intiSystem(void){
    for (int pins = 30; pins <=35; pins++){
    pinMode(pins, OUTPUT);
  }
  Serial.begin(9600);
  Serial2.begin(9600);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  //readSensor();
}

void track(int trackSpeed, int diffSpeed){
  if (S2 > 250 && S3 > 290) motorControl(trackSpeed, trackSpeed);
  else if (S2 < 250 && S3 > 290) motorControl(trackSpeed - diffSpeed, trackSpeed);
  else if (S2 > 250 && S3 < 290) motorControl(trackSpeed, trackSpeed - diffSpeed);
  else if(S2 > 250 && S3 < 290) motorControl(trackSpeed, trackSpeed);
}

void trackCrossX(int crossCount,int trackSpeed, int diffSpeed,int timeToTurn){
  for (int i = 0 ; i < crossCount; i ++){
    while(S1 > 250 || S4 > 250){
      track(trackSpeed, diffSpeed);
    }
    while(S1 < 250 || S4 < 250){
      track(trackSpeed, diffSpeed);
    }
    if (moveAction == GOY) currentY++;
    if (moveAction == BACKY) currentY--;
    if (moveAction == GOX) currentX--;
    if (moveAction == BACKX) currentX++;
    sendPos();
  }
  trackTime(timeToTurn);
  STOP
  
}

void trackCross2(int crossCount,int trackSpeed, int diffSpeed){
  for (int i = 0 ; i < crossCount; i ++){
    while(S1 > 250 || S4 > 250){
      track(trackSpeed, diffSpeed);
    }
    if (moveAction == GOY) currentY++;
    if (moveAction == BACKY) currentY--;
    if (moveAction == GOX) currentX--;
    if (moveAction == BACKX) currentX++;
    sendPos();
  }
  STOP
}

void trackTime(unsigned long timer){
  unsigned long Start;
  Start = millis();
  while(millis() - Start < timer){
    track(100,90);
  }
  STOP
}

void serialEvent() {
  while (Serial2.available()) {
    char inChar = (char)Serial2.read();
    inputString += inChar;
    if (inChar == 'z') {
      action = inputString[0];
      t_x = (int)inputString[1]-48;
      t_y = (int)inputString[2]-48;
      stringComplete = true;
    }
  }
}

void motor2(int m1Power){
  bool DIR;
  DIR = (m1Power < 0 ? true : false);
  digitalWrite(INA1, !DIR);
  digitalWrite(INB1, DIR);
  analogWrite(PWM1, abs(m1Power));
  
}

void motor1(int m1Power){
  bool DIR;
  DIR = (m1Power < 0 ? true : false);
  digitalWrite(INA2, DIR);
  digitalWrite(INB2, !DIR);
  analogWrite(PWM2, abs(m1Power));
  
}

void liftControl(int speedLift){
  bool DIR;
  DIR = (speedLift < 0 ? true : false);
  digitalWrite(LIFT_DIRA, DIR);
  digitalWrite(LIFT_DIRB, !DIR);
  analogWrite(LIFT_PWM, abs(speedLift));
}

void motorControl(int speedLeft, int speedRight){
  motor1(speedLeft);
  motor2(speedRight);
}

void readSensor(){
  while(true){
    Serial.print(analogRead(A0)); Serial.print("\t");
    Serial.print(analogRead(A1)); Serial.print("\t");
    Serial.print(analogRead(A2)); Serial.print("\t");
    Serial.println(analogRead(A3));
    delay(200);
  }
}

void turnLeft(int turnSpeed){
  motorControl(-turnSpeed,turnSpeed);
  delay(80);
  while(S2 > 250);
  while(S2 < 250);
  STOP
}

void turnRight(int turnSpeed){
  motorControl(turnSpeed,-turnSpeed);
  delay(80);
  while(S3 > 250);
  while(S3 < 250);
  STOP
}

void inComming(int x, int y){
  // LIFT UP
  delay(400);
  
  moveAction = GOY;
  liftControl(UP); delay(1100); liftControl(0);
  motorControl(-120,-120); delay(600); STOP 
  turnRight(100);  //turnRight(100);
  trackCrossX(y,100,100,200);
  turnLeft(100);
  moveAction = GOX;
  trackCross2(1,80,160);
  // LIFT DOWN
  liftControl(DOWN); delay(1050); liftControl(0);
  motorControl(-105,-105); delay(700);
  STOP
  turnLeft(100);
  moveAction = BACKX;
  trackCrossX(1,100,100,200);
  turnRight(100);
  moveAction = BACKY;
  trackCrossX(y,100,100,200);

}

void outting(int x, int y){
  delay(400);
   motorControl(-120,-120); delay(600); STOP 
  moveAction = GOY;
  turnRight(100);   //turnRight(100);
  trackCrossX(y,100,100,200);
  turnLeft(100);
  moveAction = GOX;
  trackCross2(1,80,160);
  // LIFT DOWN
  liftControl(UP); delay(1100); liftControl(0);
  

  motorControl(-105,-105); delay(700);
  STOP
  turnLeft(100);
  moveAction = BACKX;
  trackCrossX(1,100,100,200);
  turnRight(100);
  moveAction = BACKY;
  trackCrossX(y,100,100,200);
  liftControl(DOWN); delay(1050); liftControl(0);
  delay(3000);

}

void sendPos(){
  String positionSend;
  positionSend += "p";
  positionSend += currentX;
  positionSend += currentY;
  positionSend += "z";
  Serial2.print(positionSend);
}

void rfidRead()
{
  int serNum0;
  int serNum1;
  int serNum2;
  int serNum3;
  int serNum4;
  boolean readState = false;
  dataRead = "r";
  if (rfid.readCardSerial()){
    serNum0 = rfid.serNum[0];
    serNum1 = rfid.serNum[1];
    serNum2 = rfid.serNum[2];
    serNum3 = rfid.serNum[3];
    serNum4 = rfid.serNum[4];
    dataRead += String(serNum0);
    dataRead += String(serNum1);
    dataRead += String(serNum2);
    dataRead += String(serNum3);
    dataRead += String(serNum4);
              /*  Debug Code decimal data */
//                Serial.print ("Data DEC : ");
//                Serial.print(rfid.serNum[0], DEC);Serial.print(", ");
//                Serial.print(rfid.serNum[1], HEX);Serial.print(", ");
//                Serial.print(rfid.serNum[2], HEX);Serial.print(", ");
//                Serial.print(rfid.serNum[3], HEX);Serial.print(", ");
//                Serial.println(rfid.serNum[4], HEX);
              /* end debug */
    readState = true;
    }
    rfid.halt();
    Serial.print("RFID : ");
    Serial.println(dataRead);
}
