/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-websocket-server-arduino/
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Not in use

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

int sensorPin = 12;
int potPin = 14;
float leftSpeed = 90;
float rightSpeed = 90;
float actuatorSpeed = 90;
bool leftState = 0;
bool rightState = 0;
bool fowardState = 0;
bool backwardState = 0;
bool finderState = 0;
bool liftState = 0;
bool lowerState = 0;

// Replace with your network credentials
const char* ssid = "SSID";
const char* password = "password";

bool ledState = 0;
const int ledPin = 2;

// Create AsyncWebServer object on port 80
AsyncWebServer server(2001);
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
    <h1>ESP WebSocket Sumobot 6 ESP Landing</h1>
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
      leftSpeed = 150;
      rightSpeed = 150;
      notifyClients();
    }
    else if (strcmp((char*)data, "backward") == 0) {
      leftSpeed = 30;
      rightSpeed = 30;
      notifyClients();
    }
    else if (strcmp((char*)data, "left") == 0) {
      leftSpeed = 30;
      rightSpeed = 150;
      notifyClients();
    }
    else if (strcmp((char*)data, "right") == 0) {
      leftSpeed = 150;
      rightSpeed = 30;
      notifyClients();
    }
    else if (strcmp((char*)data, "halt") == 0) {
      leftSpeed = 90;
      rightSpeed = 90;
      notifyClients();
    }
    else if (strcmp((char*)data, "find") == 0) {
      finderState = 1;
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
        //ws.enable(false);
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        //ws.enable(true);
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
    if (ledState){
      return "FORWARD";
    }
    else{
      return "BACKWARDS";
    }
  }
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  leftMotor.attach(13);
  rightMotor.attach(15);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(potPin, INPUT);

  pinMode(sensorPin, INPUT);
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
  
  // Finder Code
  while(finderState == 1){
    Serial.println("In finder mode");
    Serial.println(digitalRead(sensorPin));
    if(digitalRead(sensorPin) == 0){
      leftMotor.write(0);
      rightMotor.write(0);
      finderState = 0;
      Serial.println("Exited finder mode");
    }
    leftMotor.write(180);
    rightMotor.write(180);

    delay(10);
  }

  while(liftState == 1){
    actuator.write(180);
    Serial.print("Lifting     ");
    Serial.println(analogRead(potPin));
    if(analogRead(potPin) >= 700){
      actuator.write(90);
      liftState = 0;
    }
  }
  
   while(lowerState == 1){
    actuator.write(0);
    Serial.print("Lifting     ");
    Serial.println(analogRead(potPin));
    if(analogRead(potPin) < 50){
      actuator.write(90);
      lowerState = 0;
    }
  }
  
  leftMotor.write(leftSpeed);
  rightMotor.write(rightSpeed);

}
