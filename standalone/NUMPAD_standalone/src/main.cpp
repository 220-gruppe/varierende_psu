#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>

Adafruit_MPR121 numpad = Adafruit_MPR121();

void setup()
{
  Serial.begin(115200);

  // Wire.begin(43, 44);


}

void loop()
{
  Serial.println("HEj");
}