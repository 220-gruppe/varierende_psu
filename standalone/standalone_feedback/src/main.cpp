#include <Arduino.h>

const int pwmPin = 12;
const int shutdown = 11;
const int shuntPin = 13;
const int freq = 50000;
const int ledChannel = 0;
const int resolution = 9;

// Regulator variabler
float targetCurrentMA = 1500.0; // ønsket strøm
float currentDuty = 255.0;
float Kp = 0.1; // styrke

void setup()
{
  Serial.begin(115200);
  pinMode(shutdown, OUTPUT);
  delay(10);
  digitalWrite(shutdown, HIGH);
  
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(pwmPin, ledChannel);
  
  analogSetAttenuation(ADC_0db);
}

void loop()
{
  long sum = 0;
  for (int i = 0; i < 50; i++)
  {
    sum += analogRead(shuntPin);
  }
  float adcAvg = sum / 50.0;

  float voltage = (adcAvg / 4095.0) * 1.1;
  float currentMA = (voltage / 0.1055) * 1000.0; 

  float fejl = targetCurrentMA - currentMA;
  currentDuty += fejl * Kp;

  if (currentDuty > 511) currentDuty = 511;
  if (currentDuty < 0) currentDuty = 0;

  ledcWrite(ledChannel, (int)currentDuty);


  Serial.print("ADC: ");
  Serial.print(adcAvg, 0);
  Serial.print(" | Current: ");
  Serial.print(currentMA);
  Serial.print("mA | PWM Duty: ");
  Serial.print((currentDuty / 511.0) * 100);
  Serial.println("%");

  delay(20);
}