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
bool flip;

// Replace with your network credentials
const char* ssid = "GRDTuned";
const char* password = "aaudirs4";

bool ledState = 0;
const int ledPin = 2;

// Create AsyncWebServer object on port 80
AsyncWebServer server(2003);
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
  .card {
    background-color: #ecf0f4;
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
  <div>
    <img src ="data:image/svg+xml;base64,PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0idXRmLTgiPz4KPCEtLSBHZW5lcmF0b3I6IEFkb2JlIElsbHVzdHJhdG9yIDI1LjEuMCwgU1ZHIEV4cG9ydCBQbHVnLUluIC4gU1ZHIFZlcnNpb246IDYuMDAgQnVpbGQgMCkgIC0tPgo8c3ZnIHZlcnNpb249IjEuMSIgaWQ9IkxheWVyXzEiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgeG1sbnM6eGxpbms9Imh0dHA6Ly93d3cudzMub3JnLzE5OTkveGxpbmsiIHg9IjBweCIgeT0iMHB4IgoJIHZpZXdCb3g9IjAgMCA1NzYgNDMyIiBzdHlsZT0iZW5hYmxlLWJhY2tncm91bmQ6bmV3IDAgMCA1NzYgNDMyOyIgeG1sOnNwYWNlPSJwcmVzZXJ2ZSI+CjxzdHlsZSB0eXBlPSJ0ZXh0L2NzcyI+Cgkuc3Qwe2ZpbGw6I0VDRjBGNDt9Cgkuc3Qxe2NsaXAtcGF0aDp1cmwoI1NWR0lEXzJfKTtmaWxsOiM0MUFENDk7fQoJLnN0MntjbGlwLXBhdGg6dXJsKCNTVkdJRF80Xyk7ZmlsbDojRkZGRkZGO30KCS5zdDN7Y2xpcC1wYXRoOnVybCgjU1ZHSURfNl8pO2ZpbGw6IzIzMUYyMDt9Cgkuc3Q0e2NsaXAtcGF0aDp1cmwoI1NWR0lEXzZfKTtmaWxsOiM0MUFENDk7fQoJLnN0NXtjbGlwLXBhdGg6dXJsKCNTVkdJRF84Xyk7ZmlsbDojRkZGRkZGO30KCS5zdDZ7Y2xpcC1wYXRoOnVybCgjU1ZHSURfMTBfKTtmaWxsOiM0MUFENDk7fQoJLnN0N3tjbGlwLXBhdGg6dXJsKCNTVkdJRF8xMl8pO2ZpbGw6I0ZGRkZGRjt9Cgkuc3Q4e2NsaXAtcGF0aDp1cmwoI1NWR0lEXzE0Xyk7ZmlsbDojNDFBRDQ5O30KCS5zdDl7Y2xpcC1wYXRoOnVybCgjU1ZHSURfMTZfKTtmaWxsOiNGRkZGRkY7fQoJLnN0MTB7Y2xpcC1wYXRoOnVybCgjU1ZHSURfMThfKTtmaWxsOiMyMzFGMjA7fQoJLnN0MTF7ZmlsbDojMjMxRjIwO30KCS5zdDEye2NsaXAtcGF0aDp1cmwoI1NWR0lEXzIwXyk7ZmlsbDojMjMxRjIwO30KCS5zdDEze2NsaXAtcGF0aDp1cmwoI1NWR0lEXzIwXyk7ZmlsbDojNDFBRDQ5O30KCS5zdDE0e2ZpbGw6I0ZGRkZGRjt9Cjwvc3R5bGU+CjxyZWN0IGNsYXNzPSJzdDAiIHdpZHRoPSI1NzYiIGhlaWdodD0iNDMyIi8+CjxnPgoJPGRlZnM+CgkJPHJlY3QgaWQ9IlNWR0lEXzFfIiB3aWR0aD0iNTc2IiBoZWlnaHQ9IjQzMiIvPgoJPC9kZWZzPgoJPGNsaXBQYXRoIGlkPSJTVkdJRF8yXyI+CgkJPHVzZSB4bGluazpocmVmPSIjU1ZHSURfMV8iICBzdHlsZT0ib3ZlcmZsb3c6dmlzaWJsZTsiLz4KCTwvY2xpcFBhdGg+Cgk8cGF0aCBjbGFzcz0ic3QxIiBkPSJNNDA3LjIsNzEuNGM2LjYsMCwxMS42LTMuNCwxMS42LTkuOGMwLTUuOC0zLjQtOC40LTkuMy0xMWwtMS44LTAuOGMtMy0xLjMtNC4zLTIuMS00LjMtNC4yYzAtMS43LDEuMy0zLDMuNC0zCgkJYzIsMCwzLjMsMC45LDQuNSwzbDUuNC0zLjVjLTIuMy00LTUuNS01LjYtOS45LTUuNmMtNi4yLDAtMTAuMiw0LTEwLjIsOS4yYzAsNS43LDMuMyw4LjQsOC40LDEwLjVsMS44LDAuOAoJCWMzLjIsMS40LDUuMSwyLjIsNS4xLDQuNmMwLDItMS45LDMuNS00LjgsMy41Yy0zLjUsMC01LjQtMS44LTYuOS00LjJsLTUuNywzLjNDMzk2LjUsNjguMyw0MDAuNyw3MS40LDQwNy4yLDcxLjQgTTM2OS40LDcxaDIyLjQKCQl2LTYuMWgtMTUuNFYzN2gtN1Y3MXogTTM0NS45LDY1LjJjLTYuMiwwLTEwLjQtNC44LTEwLjQtMTEuMnM0LjItMTEuMiwxMC40LTExLjJzMTAuNCw0LjksMTAuNCwxMS4yUzM1Mi4xLDY1LjIsMzQ1LjksNjUuMgoJCSBNMzQ1LjksNzEuNGM5LjgsMCwxNy42LTcuNCwxNy42LTE3LjRjMC0xMC4xLTcuOC0xNy41LTE3LjYtMTcuNXMtMTcuNiw3LjQtMTcuNiwxNy41QzMyOC4zLDY0LDMzNi4xLDcxLjQsMzQ1LjksNzEuNCBNMzA1LjcsNTYuNAoJCVY0My4xaDIuMmM1LjYsMCw4LjIsMS45LDguMiw2LjdjMCw0LjgtMi41LDYuNi04LjIsNi42SDMwNS43eiBNMzE2LjgsNjAuNGM0LjItMS45LDYuNi01LjIsNi42LTEwLjVjMC04LjMtNS43LTEyLjktMTUuMi0xMi45CgkJaC05LjR2MzRoN3YtOC41aDIuNGMwLjYsMCwxLjQsMCwyLTAuMWw1LjYsOC42aDguMUwzMTYuOCw2MC40eiBNMjc2LjksNzFoN1Y0My4xaDkuN1YzN2gtMjYuNHY2LjFoOS43VjcxeiBNMjYxLjMsNzEuM2gwLjdWMzcKCQloLTYuNXYxNC4yYzAsMi40LDAuNCw2LjIsMC40LDYuMnMtMi4zLTMuMS00LTQuNmwtMTcuMS0xNi4xaC0wLjdWNzFoNi42VjU2LjdjMC0yLjUtMC41LTYuMi0wLjUtNi4yczIuNCwzLjEsNC4xLDQuNkwyNjEuMyw3MS4zegoJCSBNMjEwLjUsNjUuMmMtNi4yLDAtMTAuNC00LjgtMTAuNC0xMS4yczQuMi0xMS4yLDEwLjQtMTEuMnMxMC40LDQuOSwxMC40LDExLjJTMjE2LjcsNjUuMiwyMTAuNSw2NS4yIE0yMTAuNSw3MS40CgkJYzkuOCwwLDE3LjYtNy40LDE3LjYtMTcuNGMwLTEwLjEtNy44LTE3LjUtMTcuNi0xNy41cy0xNy42LDcuNC0xNy42LDE3LjVDMTkyLjksNjQsMjAwLjcsNzEuNCwyMTAuNSw3MS40IE0xNzUuMiw3MS40CgkJYzYuNSwwLDExLjItMi45LDE0LjItNy41bC01LjQtMy44Yy0xLjksMi45LTQuNCw1LTguOCw1Yy02LjIsMC0xMC4zLTQuOC0xMC4zLTExLjJzNC4xLTExLjEsMTAuMy0xMS4xYzMuOSwwLDYuNiwxLjksOC4yLDQuNQoJCWw1LjMtMy44Yy0zLjEtNC44LTcuNC02LjktMTMuNi02LjljLTkuOCwwLTE3LjUsNy4zLTE3LjUsMTcuNEMxNTcuNyw2NCwxNjUuNCw3MS40LDE3NS4yLDcxLjQiLz4KCTxwYXRoIGNsYXNzPSJzdDEiIGQ9Ik0xNjIuNSwxMzVjLTUuNCwwLTkuNyw0LjMtOS43LDkuN3Y2Ny45YzAsNS40LDQuMyw5LjcsOS43LDkuN2g2Ny45YzUuNCwwLDkuNy00LjMsOS43LTkuN3YtNjcuOQoJCWMwLTUuNC00LjMtOS43LTkuNy05LjdIMTYyLjV6Ii8+CjwvZz4KPGc+Cgk8ZGVmcz4KCQk8cmVjdCBpZD0iU1ZHSURfM18iIHg9IjE3MyIgeT0iMTUxLjYiIHdpZHRoPSI0Ni44IiBoZWlnaHQ9IjU0Ii8+Cgk8L2RlZnM+Cgk8Y2xpcFBhdGggaWQ9IlNWR0lEXzRfIj4KCQk8dXNlIHhsaW5rOmhyZWY9IiNTVkdJRF8zXyIgIHN0eWxlPSJvdmVyZmxvdzp2aXNpYmxlOyIvPgoJPC9jbGlwUGF0aD4KCTxwYXRoIGNsYXNzPSJzdDIiIGQ9Ik0xODYuNywxNzcuMWgtMC43Yy0zLjUsMC03LDAtMTAuNSwwYy0wLjIsMC0wLjUsMC0wLjcsMGMtMC44LTAuMS0xLjQtMC41LTEuNi0xLjNjLTAuMy0wLjgtMC4xLTEuNCwwLjUtMgoJCWMxLTEsMi0yLDMtM2M2LTYsMTItMTIsMTgtMThjMS4yLTEuMiwyLjEtMS4yLDMuMywwYzYuOSw3LDEzLjksMTMuOSwyMC44LDIwLjhjMC42LDAuNiwwLjksMS40LDAuNiwyLjJjLTAuNCwwLjktMS4xLDEuMi0yLDEuMgoJCWMtMy42LDAtNy4yLDAtMTAuOSwwSDIwNnYwLjdjMCw4LjYsMCwxNy4xLDAsMjUuN2MwLDEuNC0wLjcsMi4xLTIuMiwyLjFjLTQuOSwwLTkuOSwwLTE0LjgsMGMtMS41LDAtMi4yLTAuNy0yLjItMi4yCgkJYzAtOC40LDAtMTYuOSwwLTI1LjNWMTc3LjF6Ii8+CjwvZz4KPGc+Cgk8ZGVmcz4KCQk8cmVjdCBpZD0iU1ZHSURfNV8iIHdpZHRoPSI1NzYiIGhlaWdodD0iNDMyIi8+Cgk8L2RlZnM+Cgk8Y2xpcFBhdGggaWQ9IlNWR0lEXzZfIj4KCQk8dXNlIHhsaW5rOmhyZWY9IiNTVkdJRF81XyIgIHN0eWxlPSJvdmVyZmxvdzp2aXNpYmxlOyIvPgoJPC9jbGlwUGF0aD4KCTxwYXRoIGNsYXNzPSJzdDMiIGQ9Ik0yMDIuNywxMTl2LTdoMS4xYzIuOCwwLDQuMSwxLjEsNC4xLDMuNWMwLDIuMy0xLjIsMy41LTQuMSwzLjVIMjAyLjd6IE0xOTkuMiwxMjZoMy41di0zLjloMS4yCgkJYzQuNywwLDcuNi0yLjUsNy42LTYuNnMtMi45LTYuNi03LjYtNi42aC00LjdWMTI2eiBNMTg4LjksMTI2LjJjMy45LDAsNi44LTIuMSw2LjgtNi40VjEwOWgtMy41djEwLjZjMCwyLjUtMS4xLDMuNC0zLjMsMy40CgkJYy0yLjMsMC0zLjQtMC45LTMuNC0zLjRWMTA5SDE4MnYxMC44QzE4MiwxMjQuMSwxODUsMTI2LjIsMTg4LjksMTI2LjIiLz4KCTxwYXRoIGNsYXNzPSJzdDQiIGQ9Ik0xNjIuNSwyMzUuOGMtNS40LDAtOS43LDQuMy05LjcsOS43djY3LjljMCw1LjQsNC4zLDkuNyw5LjcsOS43aDY3LjljNS40LDAsOS43LTQuMyw5LjctOS43di02Ny45CgkJYzAtNS40LTQuMy05LjctOS43LTkuN0gxNjIuNXoiLz4KPC9nPgo8Zz4KCTxkZWZzPgoJCTxyZWN0IGlkPSJTVkdJRF83XyIgeD0iMTczIiB5PSIyNTIuNCIgd2lkdGg9IjQ2LjgiIGhlaWdodD0iNTQiLz4KCTwvZGVmcz4KCTxjbGlwUGF0aCBpZD0iU1ZHSURfOF8iPgoJCTx1c2UgeGxpbms6aHJlZj0iI1NWR0lEXzdfIiAgc3R5bGU9Im92ZXJmbG93OnZpc2libGU7Ii8+Cgk8L2NsaXBQYXRoPgoJPHBhdGggY2xhc3M9InN0NSIgZD0iTTIwNi4xLDI4MC45aDAuN2MzLjUsMCw3LDAsMTAuNSwwYzAuMiwwLDAuNSwwLDAuNywwYzAuOCwwLjEsMS40LDAuNSwxLjYsMS4zYzAuMywwLjgsMC4xLDEuNC0wLjUsMgoJCWMtMSwxLTIsMi0zLDNjLTYsNi0xMiwxMi0xOCwxOGMtMS4yLDEuMi0yLjEsMS4yLTMuMywwYy02LjktNy0xMy45LTEzLjktMjAuOC0yMC44Yy0wLjYtMC42LTAuOS0xLjQtMC42LTIuMgoJCWMwLjQtMC45LDEuMS0xLjIsMi0xLjJjMy42LDAsNy4yLDAsMTAuOSwwaDAuN3YtMC43YzAtOC42LDAtMTcuMSwwLTI1LjdjMC0xLjQsMC43LTIuMSwyLjItMi4xYzQuOSwwLDkuOSwwLDE0LjgsMAoJCWMxLjUsMCwyLjIsMC43LDIuMiwyLjJjMCw4LjQsMCwxNi45LDAsMjUuM1YyODAuOXoiLz4KPC9nPgo8Zz4KCTxkZWZzPgoJCTxyZWN0IGlkPSJTVkdJRF85XyIgd2lkdGg9IjU3NiIgaGVpZ2h0PSI0MzIiLz4KCTwvZGVmcz4KCTxjbGlwUGF0aCBpZD0iU1ZHSURfMTBfIj4KCQk8dXNlIHhsaW5rOmhyZWY9IiNTVkdJRF85XyIgIHN0eWxlPSJvdmVyZmxvdzp2aXNpYmxlOyIvPgoJPC9jbGlwUGF0aD4KCTxwYXRoIGNsYXNzPSJzdDYiIGQ9Ik0yNjMuMiwyMzUuOGMtNS40LDAtOS43LDQuMy05LjcsOS43djY3LjljMCw1LjQsNC4zLDkuNyw5LjcsOS43aDY3LjljNS40LDAsOS43LTQuMyw5LjctOS43di02Ny45CgkJYzAtNS40LTQuMy05LjctOS43LTkuN0gyNjMuMnoiLz4KPC9nPgo8Zz4KCTxkZWZzPgoJCTxyZWN0IGlkPSJTVkdJRF8xMV8iIHg9IjI3MC4yIiB5PSIyNTYiIHdpZHRoPSI1NCIgaGVpZ2h0PSI0Ni44Ii8+Cgk8L2RlZnM+Cgk8Y2xpcFBhdGggaWQ9IlNWR0lEXzEyXyI+CgkJPHVzZSB4bGluazpocmVmPSIjU1ZHSURfMTFfIiAgc3R5bGU9Im92ZXJmbG93OnZpc2libGU7Ii8+Cgk8L2NsaXBQYXRoPgoJPHBhdGggY2xhc3M9InN0NyIgZD0iTTI5OC43LDI2OS43VjI2OWMwLTMuNSwwLTcsMC0xMC41YzAtMC4yLDAtMC41LDAtMC43YzAuMS0wLjgsMC41LTEuNCwxLjMtMS42YzAuOC0wLjMsMS40LTAuMSwyLDAuNQoJCWMxLDEsMiwyLDMsM2M2LDYsMTIsMTIsMTgsMThjMS4yLDEuMiwxLjIsMi4xLDAsMy4zYy03LDYuOS0xMy45LDEzLjktMjAuOCwyMC44Yy0wLjYsMC42LTEuNCwwLjktMi4yLDAuNmMtMC45LTAuNC0xLjItMS4xLTEuMi0yCgkJYzAtMy42LDAtNy4yLDAtMTAuOXYtMC43SDI5OGMtOC42LDAtMTcuMSwwLTI1LjcsMGMtMS40LDAtMi4xLTAuNy0yLjEtMi4yYzAtNC45LDAtOS45LDAtMTQuOGMwLTEuNSwwLjctMi4yLDIuMi0yLjIKCQljOC40LDAsMTYuOSwwLDI1LjMsMEgyOTguN3oiLz4KPC9nPgo8Zz4KCTxkZWZzPgoJCTxyZWN0IGlkPSJTVkdJRF8xM18iIHdpZHRoPSI1NzYiIGhlaWdodD0iNDMyIi8+Cgk8L2RlZnM+Cgk8Y2xpcFBhdGggaWQ9IlNWR0lEXzE0XyI+CgkJPHVzZSB4bGluazpocmVmPSIjU1ZHSURfMTNfIiAgc3R5bGU9Im92ZXJmbG93OnZpc2libGU7Ii8+Cgk8L2NsaXBQYXRoPgoJPHBhdGggY2xhc3M9InN0OCIgZD0iTTYxLjcsMjM1LjhjLTUuNCwwLTkuNyw0LjMtOS43LDkuN3Y2Ny45YzAsNS40LDQuMyw5LjcsOS43LDkuN2g2Ny45YzUuNCwwLDkuNy00LjMsOS43LTkuN3YtNjcuOQoJCWMwLTUuNC00LjMtOS43LTkuNy05LjdINjEuN3oiLz4KPC9nPgo8Zz4KCTxkZWZzPgoJCTxyZWN0IGlkPSJTVkdJRF8xNV8iIHg9IjY4LjYiIHk9IjI1NiIgd2lkdGg9IjU0IiBoZWlnaHQ9IjQ2LjgiLz4KCTwvZGVmcz4KCTxjbGlwUGF0aCBpZD0iU1ZHSURfMTZfIj4KCQk8dXNlIHhsaW5rOmhyZWY9IiNTVkdJRF8xNV8iICBzdHlsZT0ib3ZlcmZsb3c6dmlzaWJsZTsiLz4KCTwvY2xpcFBhdGg+Cgk8cGF0aCBjbGFzcz0ic3Q5IiBkPSJNOTQuMSwyODl2MC43YzAsMy41LDAsNywwLDEwLjVjMCwwLjIsMCwwLjUsMCwwLjdjLTAuMSwwLjgtMC41LDEuNC0xLjMsMS42Yy0wLjgsMC4zLTEuNCwwLjEtMi0wLjUKCQljLTEtMS0yLTItMy0zYy02LTYtMTItMTItMTgtMThjLTEuMi0xLjItMS4yLTIuMSwwLTMuM2M3LTYuOSwxMy45LTEzLjksMjAuOC0yMC44YzAuNi0wLjYsMS40LTAuOSwyLjItMC42YzAuOSwwLjQsMS4yLDEuMSwxLjIsMgoJCWMwLDMuNiwwLDcuMiwwLDEwLjl2MC43aDAuN2M4LjYsMCwxNy4xLDAsMjUuNywwYzEuNCwwLDIuMSwwLjcsMi4xLDIuMmMwLDQuOSwwLDkuOSwwLDE0LjhjMCwxLjUtMC43LDIuMi0yLjIsMi4yCgkJYy04LjQsMC0xNi45LDAtMjUuMywwSDk0LjF6Ii8+CjwvZz4KPGc+Cgk8ZGVmcz4KCQk8cmVjdCBpZD0iU1ZHSURfMTdfIiB3aWR0aD0iNTc2IiBoZWlnaHQ9IjQzMiIvPgoJPC9kZWZzPgoJPGNsaXBQYXRoIGlkPSJTVkdJRF8xOF8iPgoJCTx1c2UgeGxpbms6aHJlZj0iI1NWR0lEXzE3XyIgIHN0eWxlPSJvdmVyZmxvdzp2aXNpYmxlOyIvPgoJPC9jbGlwUGF0aD4KCTxwYXRoIGNsYXNzPSJzdDEwIiBkPSJNMzI0LjgsMzQ5LjVoMy41di0xMy45aDQuOXYtMy4xSDMyMHYzLjFoNC45VjM0OS41eiBNMzAzLjEsMzQ5LjVoMy41di01LjJoNy4zdjUuMmgzLjV2LTE3aC0zLjV2OC43aC03LjMKCQl2LTguN2gtMy41VjM0OS41eiBNMjkzLjMsMzQ5LjdjMy4xLDAsNS41LTEuMiw2LjUtMnYtNy40aC03LjN2My4xaDQuMXYyLjVjLTAuOSwwLjQtMi4yLDAuNi0zLjMsMC42Yy0zLjYsMC01LjYtMi40LTUuNi01LjYKCQljMC0zLjIsMi01LjYsNS4xLTUuNmMyLDAsMywwLjcsMy44LDEuNmwyLjQtMi40Yy0xLjQtMS42LTMuNC0yLjQtNi4yLTIuNGMtNS4xLDAtOC43LDMuNy04LjcsOC43CgkJQzI4NC4xLDM0NiwyODcuNywzNDkuNywyOTMuMywzNDkuNyBNMjc3LjUsMzQ5LjVoMy41di0xN2gtMy41VjM0OS41eiBNMjY1LjYsMzQyLjJ2LTYuNmgxLjFjMi44LDAsNC4xLDEsNC4xLDMuNHMtMS4yLDMuMy00LjEsMy4zCgkJSDI2NS42eiBNMjcxLjEsMzQ0LjJjMi4xLTAuOSwzLjMtMi42LDMuMy01LjNjMC00LjEtMi45LTYuNC03LjYtNi40aC00Ljd2MTdoMy41di00LjJoMS4yYzAuMywwLDAuNywwLDEtMC4xbDIuOCw0LjNoNC4xCgkJTDI3MS4xLDM0NC4yeiIvPgoJPHBhdGggY2xhc3M9InN0MTAiIGQ9Ik0yMzMsMzQ5LjdoMC4zdi0xNy4xaC0zLjN2Ny4xYzAsMS4yLDAuMiwzLjEsMC4yLDMuMXMtMS4yLTEuNS0yLTIuM2wtOC42LTguMWgtMC4zdjE3LjJoMy4zdi03LjEKCQljMC0xLjItMC4yLTMuMS0wLjItMy4xczEuMiwxLjUsMiwyLjNMMjMzLDM0OS43eiBNMjAwLjUsMzQ5LjdoMC40bDQtOC4yYzAuNC0wLjksMS0yLjUsMS0yLjVzMC41LDEuNiwwLjksMi41bDMuOCw4LjJoMC4zCgkJbDYuMi0xNy4xaC0zLjRsLTIuNSw2LjhjLTAuNCwxLjItMC45LDMuMS0wLjksMy4xcy0wLjUtMS45LTEuMS0zLjFsLTMuMy03aC0wLjRsLTMuMyw3Yy0wLjYsMS4xLTEuMSwzLjEtMS4xLDMuMXMtMC41LTEuOS0wLjktMy4xCgkJbC0yLjUtNi44aC0zLjdMMjAwLjUsMzQ5Ljd6IE0xODQuOCwzNDYuNmMtMy4xLDAtNS4yLTIuNC01LjItNS42YzAtMy4yLDIuMS01LjYsNS4yLTUuNmMzLjEsMCw1LjIsMi40LDUuMiw1LjYKCQlDMTkwLDM0NC4yLDE4Ny45LDM0Ni42LDE4NC44LDM0Ni42IE0xODQuOCwzNDkuN2M0LjksMCw4LjgtMy43LDguOC04LjdjMC01LTMuOS04LjctOC44LTguN2MtNC45LDAtOC44LDMuNy04LjgsOC43CgkJQzE3NiwzNDYsMTc5LjksMzQ5LjcsMTg0LjgsMzQ5LjcgTTE2Mi45LDM0Ni40di0xMC45aDEuNWMzLjgsMCw1LjUsMi4yLDUuNSw1LjRjMCwzLjItMS43LDUuNC01LjUsNS40SDE2Mi45eiBNMTU5LjQsMzQ5LjVoNS4yCgkJYzUuMiwwLDguOS0zLjUsOC45LTguNWMwLTUtMy43LTguNS04LjktOC41aC01LjJWMzQ5LjV6Ii8+CjwvZz4KPHBhdGggY2xhc3M9InN0MTEiIGQ9Ik0xMTUuMywzNDkuNWgzLjV2LTEzLjloNC44di0zLjFoLTEzLjJ2My4xaDQuOVYzNDkuNXogTTk3LjUsMzQ5LjVoMy41di01LjJoNi44di0zLjFIMTAxdi01LjZoNy40di0zLjFIOTcuNQoJVjM0OS41eiBNODIuNiwzNDkuNWgxMS42di0zLjFoLTguMVYzNDRIOTN2LTNoLTYuOHYtNS40aDcuOHYtMy4xSDgyLjZWMzQ5LjV6IE02OC42LDM0OS41aDExLjJ2LTMuMWgtNy43di0xMy45aC0zLjVWMzQ5LjV6Ii8+CjxnPgoJPGRlZnM+CgkJPHJlY3QgaWQ9IlNWR0lEXzE5XyIgd2lkdGg9IjU3NiIgaGVpZ2h0PSI0MzIiLz4KCTwvZGVmcz4KCTxjbGlwUGF0aCBpZD0iU1ZHSURfMjBfIj4KCQk8dXNlIHhsaW5rOmhyZWY9IiNTVkdJRF8xOV8iICBzdHlsZT0ib3ZlcmZsb3c6dmlzaWJsZTsiLz4KCTwvY2xpcFBhdGg+Cgk8cGF0aCBjbGFzcz0ic3QxMiIgZD0iTTQ5NS43LDI3OC45di03aDEuMWMyLjgsMCw0LjEsMS4xLDQuMSwzLjVjMCwyLjMtMS4yLDMuNS00LjEsMy41SDQ5NS43eiBNNDkyLjIsMjg1LjloMy41VjI4MmgxLjIKCQljNC43LDAsNy42LTIuNSw3LjYtNi41YzAtNC4xLTIuOS02LjYtNy42LTYuNmgtNC43VjI4NS45eiBNNDg1LjEsMjg1LjloMy41di0xN2gtMy41VjI4NS45eiBNNDcxLDI4NS45aDExLjJ2LTMuMWgtNy43di0xMy45SDQ3MQoJCVYyODUuOXogTTQ1Ny4xLDI4NS45aDMuNXYtNS4yaDYuOHYtMy4xaC02LjhWMjcyaDcuNHYtMy4xaC0xMC45VjI4NS45eiIvPgoJPHBhdGggY2xhc3M9InN0MTMiIGQ9Ik00NDYuNCwxNzIuMWMtNS40LDAtOS43LDQuMy05LjcsOS43djY3LjljMCw1LjQsNC4zLDkuNyw5LjcsOS43aDY3LjljNS40LDAsOS43LTQuMyw5LjctOS43di02Ny45CgkJYzAtNS40LTQuMy05LjctOS43LTkuN0g0NDYuNHoiLz4KPC9nPgo8cG9seWdvbiBjbGFzcz0ic3QxNCIgcG9pbnRzPSI0NjYuNCwyMzguMiA0NzUuOSwyMzguMiA0NzUuOSwyMjQuMiA0OTQuMSwyMjQuMiA0OTQuMSwyMTUuOCA0NzUuOSwyMTUuOCA0NzUuOSwyMDAuNiA0OTYsMjAwLjYgCgk0OTYsMTkyLjMgNDY2LjQsMTkyLjMgIi8+Cjwvc3ZnPgo=" width = 25%/>
  </div>
<script>
  var gateway = `ws://${location.hostname}:8081/ws`;
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
    else if (strcmp((char*)data, "flip") == 0) {
      flip = 1;
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
  
  int time_now = millis();
  int period = 500;
  while(flip){
    leftFlip.write(30);
    rightFlip.write(123);
    while(millis() < time_now + period){
    }  
    leftFlip.write(91);
    rightFlip.write(64);
    flip = 0;
  }
    
  
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
