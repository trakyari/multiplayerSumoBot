#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
int COUNTER_VAL = 0;
const int TIME_INTERVAL = 100;
const int SENSOR_VAL = 0;


char auth[] = "e5yOCCWJVL03BLqcvbA3_PqylWbppkJR";

char ssid[] = "Bleem";
char pass[] = "blerim123";

Servo servo;

void setup()
{
  Serial.begin(19200);
  pinMode(15, INPUT);
  //servo.attach(15); //nodeMCU port D8
}

void loop()
{ 
  int SENSOR_VAL = digitalRead(15);
  if(SENSOR_VAL == 1) { 
    //readLine();
    delay(1000);
    if(SENSOR_VAL == 0)
      COUNTER_VAL++;
  }
  
  Serial.print(COUNTER_VAL);
  Serial.println(digitalRead(15));
  
}

void readLine(){
  delay(100);
  if(SENSOR_VAL == 0) {
    COUNTER_VAL++;
  }
  
}
