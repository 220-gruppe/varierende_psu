// freeRTOS
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/ledc.h"

// PWM
#include <driver/ledc.h>

// Include
#include "logo.h"
#include "server.h"
#include "pwm.h"
#include "numpad.h"
#include "auth.h"
#include "rfid.h"
#include "screen.h"
#include ""

// SPI
#define SPI_MISO 11
#define SPI_MOSI 13
#define SPI_SCK 12

// I2C
#define I2C_SDA 21
#define I2C_SCL 16

// BUTTONS
#define BUTTON_PIN 3

String workerID;

String scannedUID = "";
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

void inaktivitetTjek() // LOG UD PGA INAKTIVITET
{
    if (millis() - tidStart > TIMER)
    {
        isLoggedIn = false;
        indtastet = "";
        tastet = "";
        tft.fillRect(0, 120, 320, 50, SPIDER_BG);
        tft.setTextColor(TFT_RED, SPIDER_BG);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("INAKTIV: LOGGET UD", 160, 110, 1);
        delay(2000);
        nuStatus = "KLAR TIL SCAN";
        Serial.println("logget ud pga inaktivitet");
    }
}

void pwmTask(void *pvParameters)
{
  const uint32_t pwmMaxDuty = (1UL << PWM_RESOLUTION) - 1;
  uint32_t lastPrint = 0;
  TickType_t lastWake = xTaskGetTickCount();

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

    xTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10));
  }
}

void interfaceTask(void *pvParameters)
{
  TickType_t lastWake = xTaskGetTickCount();

  for (;;)
  {
    opdaterScreen();
    kortScan();

    if (manglerPin)
      numpadLogik();

    if (isLoggedIn)
      inaktivitetTjek();

    xTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10));
  }
}

void serverTask(void *pvParameters)
{
  TickType_t lastWake = xTaskGetTickCount();

  for (;;)
  {
    server.handleClient();
    xTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10));
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1500);
  Serial.println("Boot startet");

  // Start SPI
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  // START I2C SDA, SCL
  Wire.begin(I2C_SDA, I2C_SCL);

  setupDatabase();

  Serial.println("Starter tasks");

  xTaskCreatePinnedToCore(pwmTask, "pwm", 2048, NULL, 5, &pwmT, 1);
  xTaskCreatePinnedToCore(interfaceTask, "interface", 6144, NULL, 1, &interfaceT, 1);
  xTaskCreatePinnedToCore(serverTask, "server", 2048, NULL, 1, &serverT, 0);
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
}
