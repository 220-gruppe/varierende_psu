#include <Arduino.h>

#define PWM 10
#define POT 0
#define DIR 12

void setup()
{
  Serial.begin(115200);
  pinMode(PWM, OUTPUT);
  pinMode(DIR, OUTPUT);
}

void loop()
{
  float rawPot = analogRead(0);
  int speed = 0;

  if (rawPot > 530)
  {
    digitalWrite(DIR, LOW);
    speed = map(rawPot, 533, 1023, 0, 255);
  }

  else if (rawPot < 490)
  {
    digitalWrite(DIR, HIGH);
    speed = map(rawPot, 0, 490, 255, 0);
  }

  else
  {
    speed = 0;
  }

  analogWrite(PWM, speed);

  Serial.print("Pot: ");
  Serial.print(rawPot);
  Serial.print(" | Speed: ");
  Serial.print(speed);

  

  int noget = digitalRead(2);
  Serial.print(" PIN 2: ");
  Serial.print(noget);
  int noget2 = digitalRead(9);
  Serial.print(" PIN 9: ");
  Serial.println(noget2);

  // float duty = rawPot / 1021.0;

  // int totalTid = 1000;
  // int onTime = duty * totalTid;
  // int offTime = totalTid - onTime;

  // if (duty >= 0.50)
  // {
  //   digitalWrite(DIR, HIGH);
  // }
  // else
  // {
  //   digitalWrite(DIR, LOW);
  // }

  // if (onTime > 0)
  // {
  //   digitalWrite(PWM, HIGH);
  //   delayMicroseconds(onTime);
  // }

  // if (offTime > 0)
  // {
  //   digitalWrite(PWM, LOW);
  //   delayMicroseconds(offTime);
  // }

  // Serial.print("DUTY | ");
  // Serial.println(duty);
}