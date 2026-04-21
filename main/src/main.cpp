// egne libraries
#include <logo.h>
#include <variabler.h>
#include <config.h>
#include <functions.h>
#include <server.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/ledc.h"

String scannedUID = "";
String workerID = "";
float heatInput = 70000;   // aendres til noget fra sensor
uint32_t mellemLog = 0;    // aendres til reelt tidspunkt
int counter = 0;           // blot en counter til antal
SvejseLog aktuelSvejsning; // instans af struct
File svejsningData;        // instans af sdkort
File logins;               // instans af sdkort

float targetCurrentMA = 5.0;
float currentDuty = 0.0;
float Kp = 1.1;

String tempNavn = "";
String tempPin = "";
String tempUID = "";
bool waitforChip = false;

bool manglerPin = false;
bool isLoggedIn = false;
String indtastet = "";
bool ikkeKodet = false;
String nuStatus = "";
String tastet = "";
String korrektPin = "";
String sidsteStatus = "";

unsigned long tidStart = 0;

TaskHandle_t interfaceT = nullptr;
TaskHandle_t serverT = nullptr;
TaskHandle_t pwmT = nullptr;

void pwmTask(void *pvParameters)
{
  const uint32_t pwmMaxDuty = (1UL << PWM_RESOLUTION) - 1;
  uint32_t lastPrint = 0;

  for (;;)
  {
    uint32_t sumMV = 0;

    for (int i = 0; i < 50; i++)
    {
      sumMV += analogReadMilliVolts(SHUNT_PIN);
    }

    float shuntVoltageMV = sumMV / 50.0;
    float currentMA = shuntVoltageMV / SHUNT_RESISTOR_OHM;
    float fejl = targetCurrentMA - currentMA;

    currentDuty += fejl * Kp;

    // PWM_RESOLUTION er 8-bit, saa duty skal ligge mellem 0 og 255.
    if (currentDuty > pwmMaxDuty)
      currentDuty = pwmMaxDuty;
    if (currentDuty < 0)
      currentDuty = 0;

    ledc_set_duty(PWM_MODE, PWM_CHANNEL, (uint32_t)currentDuty);
    ledc_update_duty(PWM_MODE, PWM_CHANNEL);

    if (millis() - lastPrint > 500)
    {
      lastPrint = millis();
      int currentDutyPct = round((currentDuty / pwmMaxDuty) * 100);

      Serial.print("Target: ");
      Serial.print(targetCurrentMA);
      Serial.print("mA | Current: ");
      Serial.print(currentMA);
      Serial.print("mA | PWM Duty: ");
      Serial.print(currentDutyPct);
      Serial.println("%");
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void interfaceTask(void *pvParameters)
{
  for (;;)
  {
    opdaterScreen();
    kortScan();

    if (manglerPin)
      numpadLogik();

    if (isLoggedIn)
      inaktivitetTjek();

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void serverTask(void *pvParameters)
{
  for (;;)
  {
    server.handleClient();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1500);
  Serial.println("Boot startet");

  setupSPI();
  createFile();
  setupPwm();

  Serial.println("Starter tasks");

  xTaskCreatePinnedToCore(pwmTask, "pwm", 2048, NULL, 5, &pwmT, 1);
  xTaskCreatePinnedToCore(interfaceTask, "interface", 6144, NULL, 1, &interfaceT, 1);
  xTaskCreatePinnedToCore(serverTask, "server", 2048, NULL, 1, &serverT, 0);
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
}
