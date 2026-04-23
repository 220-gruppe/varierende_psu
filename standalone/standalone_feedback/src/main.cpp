#include <Arduino.h>

const int pwmPin = 10;
const int sdPin = 12;
const int shuntPin = 13;
const int freq = 50000;
const int ledChannel = 0;
const int resolution = 9;

// Regulator variabler
float targetCurrentMA = 1000.0; // ønsket strøm
float currentDuty = 255.0;
float Kp = 0.1; // styrke

// ===== ADC midlingsfilter =====
#define ADC_FILTER_SIZE 32
int adcBuffer[ADC_FILTER_SIZE];
int adcIndex = 0;
long adcSum = 0;
bool adcFilled = false;

void setup()
{
  Serial.begin(115200);
  pinMode(pwmPin, PULLDOWN);
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(pwmPin, ledChannel);
  analogSetAttenuation(ADC_0db);
}

void loop()
{
  // ---------- ADC glidende middelværdi ----------
  int adcRaw = analogRead(shuntPin);

  adcSum -= adcBuffer[adcIndex];
  adcBuffer[adcIndex] = adcRaw;
  adcSum += adcRaw;

  adcIndex++;
  if (adcIndex >= ADC_FILTER_SIZE)
  {
    adcIndex = 0;
    adcFilled = true;
  }

  float adcAvg;
  if (adcFilled)
    adcAvg = adcSum / (float)ADC_FILTER_SIZE;
  else
    adcAvg = adcSum / (float)(adcIndex + 1);
  // --------------------------------------------

  float voltage = (adcAvg / 4095.0) * 1.1;
  float currentMA = (voltage / 0.1055) * 1000.0; // strøm

  float fejl = targetCurrentMA - currentMA;

  currentDuty += fejl * Kp;

  if (currentDuty > 511)
    currentDuty = 511;
  if (currentDuty < 0)
    currentDuty = 0;

  int currentdutyPct = round((currentDuty / 511.0) * 100);

  ledcWrite(ledChannel, (int)255);//ændre dette tilbage hvis det skal regulere automatisk

  Serial.print("Target: ");
  Serial.print(targetCurrentMA);
  Serial.print("mA | Current: ");
  Serial.print(currentMA);
  Serial.print("mA | PWM Duty: ");
  Serial.print(currentdutyPct);
  Serial.println("%");

  delay(20);
}

//FORSØG 1
// #include <Arduino.h>

// const int pwmPin = 10;
// const int shuntPin = 13;
// const int freq = 50000;
// const int ledChannel = 0;
// const int resolution = 9;

// // Regulator variabler
// float targetCurrentMA = 2000.0; // ønsket strøm
// float currentDuty = 255.0;
// float Kp = 0.1; // styrke

// void setup()
// {
//   Serial.begin(115200);
//   ledcSetup(ledChannel, freq, resolution);
//   ledcAttachPin(pwmPin, ledChannel);
//   analogSetAttenuation(ADC_0db);
// }

// void loop()
// {
//   long sum = 0;
//   for (int i = 0; i < 50; i++)
//   { // gennemsnit for adc måling
//     sum += analogRead(shuntPin);
//   }
//   float adcAvg = sum / 50.0;
//   float voltage = (adcAvg / 4095) * 1.1;
//   float currentMA = (voltage / 0.1055) * 1000.0; //strøm

//   float fejl = targetCurrentMA - currentMA; // hvor stor er fejlen

//   currentDuty += fejl * Kp;
//   int currentdutyPct = round((currentDuty / 511.0) * 100);

//   // grænseværdier for 0-100%
//   if (currentDuty > 511)
//     currentDuty = 511;
//   if (currentDuty < 0)
//     currentDuty = 0;


//   ledcWrite(ledChannel, (int)currentDuty); //opdater PWM duty
//   //delay(10000);
//   //ledcWrite(ledChannel, 255);
//   // Serial.print(analogRead(shuntPin));
//   Serial.print("Target: ");
//   Serial.print(targetCurrentMA);
//   Serial.print("mA | Current: ");
//   Serial.print(currentMA);
//   Serial.print("mA | PWM Duty: ");
//   Serial.print(currentdutyPct);
//   Serial.println("%");
//   delay(20); 
// }