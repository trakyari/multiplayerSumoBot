/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-websocket-server-arduino/
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

/* Front end is running on DigitalOcean server sumobot.ddns.net
 * This is the command server for Bot5 in COD sumobot livestream
 */

// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include <ESP8266mDNS.h>
#include <Servo.h>

// Motor Setup for drive motors and bot specific features
Servo leftMotor;
Servo rightMotor;
Servo flipper;


float leftSpeed = 90;
float rightSpeed = 90;

bool flip= 0;

// Replace with your network credentials of event location
const char* ssid = "SSID";
const char* password = "password";

bool ledState = 0;
const int ledPin = 2;

// Create AsyncWebServer object on port 2004, this is so tunnel can recoginize which bot 
AsyncWebServer server(2004);
AsyncWebSocket ws("/ws");


// Test front end for deubgging network connection
// need to change 
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
    <h1>ESP WebSocket Sumobot 5 ESP Landing</h1>
  </div>
 </body>
</html>
)rawliteral";

void notifyClients() {
  ws.textAll(String(ledState));
}

// Message handler to parse websocket messages
// Wiring on Bot 5 might be flipped for each motor
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
    else if (strcmp((char*)data, "flip") == 0) {
      flip = !flip;
      notifyClients();
    }
  }
}


/* Disable the websocket when new user is connected
 * Enable again when user leaves
 * Send user commands to string proccessor
 */
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        ws.enable(false);
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        ws.enable(true);
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
  flipper.attach(14);
  

  
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);


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

  // This is if you had a roller or spinner  
  if(flip){
    flipper.write(160);
  }
  else {
    flipper.write(90);
  }
    
 
  
  leftMotor.write(leftSpeed);
  rightMotor.write(rightSpeed);

}
