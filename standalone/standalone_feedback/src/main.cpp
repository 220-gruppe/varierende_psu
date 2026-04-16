#include <Arduino.h>

const int ledPin = 13;
const int shuntPin = 12;
const int freq = 50000;
const int ledChannel = 0;
const int resolution = 8;

// Regulator variabler
float targetCurrentMA = 5.0; // ønsket strøm
float currentDuty = 127.0;
float Kp = 1.5; // styrke

void setup()
{
  Serial.begin(115200);
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(ledPin, ledChannel);
  analogSetAttenuation(ADC_0db);
}

void loop()
{
  long sum = 0;
  for (int i = 0; i < 50; i++)
  { // gennemsnit for adc måling
    sum += analogRead(shuntPin);
  }
  float adcAvg = sum / 50.0;
  float voltage = (adcAvg / 4095.0) * 1.1;
  float currentMA = (voltage / 50.0) * 1000.0; // strøm

  float fejl = targetCurrentMA - currentMA; // hvor stor er fejlen

  currentDuty += fejl * Kp;
  int currentdutyPct = round((currentDuty / 255.0) * 100);

  // grænseværdier for 0-100%
  if (currentDuty > 255)
    currentDuty = 255;
  if (currentDuty < 0)
    currentDuty = 0;


  ledcWrite(ledChannel, (int)currentDuty); //opdater PWM duty

  Serial.print("Target: ");
  Serial.print(targetCurrentMA);
  Serial.print("mA | Current: ");
  Serial.print(currentMA);
  Serial.print("mA | PWM Duty: ");
  Serial.print(currentdutyPct);
  Serial.println("%");

  delay(10); 
  //Denne kommentar er super fucking vigtig, ellers kompiler lortet ikke
}