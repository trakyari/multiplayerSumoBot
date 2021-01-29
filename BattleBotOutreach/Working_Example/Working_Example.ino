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
const char* ssid = "Abdullai";
const char* password = "babush7.";

bool ledState = 0;
const int ledPin = 2;

// Create AsyncWebServer object on port 80
AsyncWebServer server(2000);
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
    color: white;
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
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  .card {
    background-color: #F8F7F9;;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }
  .button {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #0f8b8d;
    border: none;
    border-radius: 5px;
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    -webkit-tap-highlight-color: rgba(0,0,0,0);
   }
   /*.button:hover {background-color: #0f8b8d}*/
   .button:active {
     background-color: #0f8b8d;
     box-shadow: 2 2px #CDCDCD;
     transform: translateY(2px);
   }
   .state {
     font-size: 1.5rem;
     color:#8c8c8c;
     font-weight: bold;
   }
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1>ESP WebSocket Sumobot 1 Server</h1>
  </div>
  <div class="content">
    <div class="card">
      <h2>Output - GPIO 2</h2>
      <p class="state">state: <span id="state">%STATE%</span></p>
      <p><button id="button" class="button">Toggle</button></p>
    </div>
  </div>
<script>
  var gateway = `ws://${location.hostname}:8080/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    var state;
    if (event.data == "1"){
      state = "ON";
    }
    else{
      state = "OFF";
    }
    document.getElementById('state').innerHTML = state;
  }
  function onLoad(event) {
    initWebSocket();
    initButton();
  }
  function initButton() {
    document.getElementById('button').addEventListener('click', forward);
    document.addEventListener('keydown', keyDown);
    document.addEventListener('keyup', keyUp);
  }
  function keyDown(){
    if (event.key == 'ArrowUp'){
      forward();
    } else if (event.key == 'ArrowDown'){
      backward();
    } else if (event.key == 'ArrowLeft'){
      left();
    } else if (event.key == 'ArrowRight'){
      right();
    }
    else if(event.key == ' '){
      halt();
    }
    else if(event.key == 'f'){
      finder();
    }
    else if(event.key == 'l'){
      lift();
    }
    else if(event.key == 'd'){
      lower();
    }
  }
  
  function keyUp(){
    if (event.key == 'ArrowUp'){
      halt();
    } else if (event.key == 'ArrowDown'){
      halt();
    } else if (event.key == 'ArrowLeft'){
      halt();
    } else if (event.key == 'ArrowRight'){
      halt();
    }
  }
  
  function forward(){
    websocket.send('forward');
  }
  function backward(){
    websocket.send('backward');
  }
  function left(){
    websocket.send('left');
  }
  function right(){
    websocket.send('right');
  }
  function halt(){
    websocket.send('halt');
  }
  function finder(){
    websocket.send('find');
  }
  function lift(){
    websocket.send('lift');
  }
  function lower(){
    websocket.send('lower');
  }
</script>
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
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
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
  /*
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  */
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
