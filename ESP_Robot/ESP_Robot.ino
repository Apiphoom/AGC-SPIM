#include <ESP8266WiFi.h>
#include <MicroGear.h>

const char* ssid     = "Jcubuntu";
const char* password = "12345678";

#define APPID   "SPIM"
#define KEY     "lPwMXOUWIpwdRQo"
#define SECRET  "4LObnhEGXFsE3LG6kW3QvEB6q"

#define ALIAS   "robot01"
#define STATETOPIC "/RobotState/" ALIAS
#define POSTOPIC "/RobotPos/" ALIAS

WiFiClient client;
MicroGear microgear(client);

boolean statusLED = true;
unsigned long lastUpdate = 0;
boolean stringComplete = false;
String inputString;
char action;
int t_x, t_y;

void onMsgComing(char *topic, uint8_t* msg, unsigned int msglen) {
  //Serial.print("Incoming message --> ");
  msg[msglen] = '\0';
  String dataSend = "";
  dataSend += msg[0];
  dataSend += msg[1];
  dataSend += msg[2];
  dataSend += "z";
  Serial.print((char *)msg);

}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  //Serial.println("Connected to NETPIE...");
  microgear.setAlias(ALIAS);
}

void setup() {
  microgear.on(MESSAGE, onMsgComing);
  microgear.on(CONNECTED, onConnected);
  Serial.begin(9600);
  pinMode(16, OUTPUT);
  if (WiFi.begin(ssid, password)) {
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      statusLED = !statusLED;
      digitalWrite(16, statusLED);
    }
  }
  microgear.init(KEY, SECRET, ALIAS); // กำหนดค่าตันแปรเริ่มต้นให้กับ microgear
  microgear.connect(APPID);           // ฟังก์ชั่นสำหรับเชื่อมต่อ NETPIE
}

void loop() {
  serialEvent();
  if (microgear.connected()) {
    digitalWrite(16, LOW);
    if (stringComplete) {
      if (inputString[0] == 'p') {
        String sendDataToWeb;
        sendDataToWeb += inputString[0];
        sendDataToWeb += " ";
        sendDataToWeb += inputString[1];
        sendDataToWeb += " ";
        sendDataToWeb += inputString[2];
        microgear.chat("website", sendDataToWeb);
       
        inputString = "";
        stringComplete = false;
      }
      else if (inputString[0] == 'r') {
        String sendDataToWeb;
        unsigned int textLen = inputString.length();
        for (int i = 0 ; i < textLen - 1; i++){
          sendDataToWeb += inputString[i];
        }
        microgear.chat("website",sendDataToWeb);
        inputString = "";
        stringComplete = false;
      }

    }
    microgear.loop();
    if (millis()  - lastUpdate > 2000) {
      microgear.publish(STATETOPIC, "1");
      lastUpdate = millis();
    }
  }
  else {
    digitalWrite(16, HIGH);
    microgear.connect(APPID);
  }
}


void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == 'z') {
      action = inputString[0];
      t_x = (int)inputString[1] - 48;
      t_y = (int)inputString[2] - 48;
      stringComplete = true;
    }
  }
}
