/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-websocket-server-arduino/
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include <ESP8266mDNS.h>
#include <Servo.h>

// Motor Setup
Servo leftMotor;
Servo rightMotor;
Servo actuator;
Servo leftFlip;
Servo rightFlip;


int leftFlipPin = 14;
int rightFlipPin = 12;
int sensorPin = 12;
int potPin = 14;
float leftSpeed = 90;
float rightSpeed = 90;
float actuatorSpeed = 90;
bool finderState = 0;
bool liftState = 0;
bool lowerState = 0;
bool ledState = 0;
bool flip;

// Replace with your network credentials
int userCount = 0;
const char* ssid =  "SSID";
const char* password = "password";


// Create AsyncWebServer object on port 80
AsyncWebServer server(2002);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: #ecf0f4;
  }
  h2{
    font-size: 1.5rem;
    font-weight: bold;
    color: #143642;
  }
  .topnav {
    overflow: hidden;
    background-color: #143642;
  }
  body {
    margin: 0;
    background-color: #ecf0f4;
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1>ESP WebSocket Sumobot 3 ESP Test Server</h1>
  </div>
 </body>
</html>
)rawliteral";

void notifyClients() {
  ws.textAll(String(ledState));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "forward") == 0) {
      leftSpeed = 30;
      rightSpeed = 150;
      Serial.println("Up");
      notifyClients();
    }
    else if (strcmp((char*)data, "backward") == 0) {
      leftSpeed = 150;
      rightSpeed = 30;
      Serial.println("down");
      notifyClients();
    }
    else if (strcmp((char*)data, "left") == 0) {
      leftSpeed = 30;
      rightSpeed = 30;
      Serial.println("left");
      notifyClients();
    }
    else if (strcmp((char*)data, "right") == 0) {
      leftSpeed = 150;
      rightSpeed = 150;
      Serial.println("right");
      notifyClients();
    }
    else if (strcmp((char*)data, "halt") == 0) {
      leftSpeed = 90;
      rightSpeed = 90;
      Serial.println("stop");
      notifyClients();
    }
    else if (strcmp((char*)data, "flip") == 0) {
      flip = !flip;
      notifyClients();
    }
    else if (strcmp((char*)data, "lift") == 0){
      liftState = 1;
      notifyClients();
    }
    else if (strcmp((char*)data, "lower") == 0){
      lowerState = 1;
      notifyClients();
    }
  }
}
      
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        userCount++;
        if(userCount >= 2){
          ws.enable(false);
        }
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        userCount--;
        if(userCount < 2){
          ws.enable(true);
        }
        leftMotor.write(90);
        rightMotor.write(90);
        break;
      case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
      return "CONNECTED";
  }
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  leftMotor.attach(13);
  rightMotor.attach(15);
  leftFlip.attach(leftFlipPin);
  rightFlip.attach(rightFlipPin);
  leftFlip.write(90);
  rightFlip.write(10);
  //pinMode(potPin, INPUT);

  //pinMode(sensorPin, INPUT);
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  
 
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });


  // Start server
  server.begin();
}

void loop() {
  ws.cleanupClients();

  if(flip)
    leftFlip.write(160);
  else
    leftFlip.write(90);

  // This is some flipper code for two servos facing opposite directions
  // Left Top Stop = leftTstop
  // Left Bottom Stop = leftBstop
  // This is the same for the right, used for setting the end positions
  // of the servos, this will be different for every servo and you need to find this manually
  // Another note is to not use delays because this will interfere with ASyncWebServer
  /*
  int time_now = millis();
  int period = 500;
  int servoDelay = 50;
  int time_nowS;
  
  int leftTstop = 30;
  int rightTstop = 130;
  
  int rightBstop = 10;
  int leftBstop = 143;

  leftFlip.write(leftBstop);
  rightFlip.write(rightBstop);

  
  
  while(flip){
    leftFlip.write(leftBstop);
    rightFlip.write(rightBstop);
    while(leftBstop > leftTstop && rightBstop < rightTstop){    
      leftBstop -=  10;
      rightBstop += 10;
      leftFlip.write(leftBstop);
      rightFlip.write(rightBstop);
      time_nowS = millis();
      while(millis() < time_nowS + servoDelay){}
    }
 
    while(leftBstop < 143 || rightBstop > 10){    
      leftBstop +=  10;
      rightBstop -= 10;
      leftFlip.write(leftBstop);
      rightFlip.write(rightBstop);
      time_nowS = millis();
      while(millis() < time_nowS + servoDelay){}
    }
    
    time_now = millis();
    while(millis() < time_nowS + period){}    
    leftFlip.write(10);
    rightFlip.write(143);
    flip = 0;
  }
  */
  
 
  leftMotor.write(leftSpeed);
  rightMotor.write(rightSpeed);

}
