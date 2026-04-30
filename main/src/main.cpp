// freeRTOS
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/ledc.h"
#include <SPI.h>
#include <Wire.h>

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

// SPI
#define SPI_MISO 11
#define SPI_MOSI 13
#define SPI_SCK 12

// I2C
#define I2C_SDA 21
#define I2C_SCL 16

float heatInput = 70000;   // aendres til noget fra sensor
float targetCurrentMA = 5.0;
float Kp = 1.1;
unsigned long tidStart = 0;

namespace
{
constexpr unsigned long INACTIVITY_TIMEOUT_MS = 30000;
}

TaskHandle_t interfaceT = nullptr;
TaskHandle_t serverT = nullptr;
TaskHandle_t pwmT = nullptr;

void inaktivitetTjek() // LOG UD PGA INAKTIVITET
{
    if (authStatus() && millis() - tidStart > INACTIVITY_TIMEOUT_MS)
    {
        logout();
        resetNumpad();
        screenReady();
        screenInactiveLogout();
        Serial.println("logget ud pga inaktivitet");
    }
}

void pwmTask(void *pvParameters)
{
  TickType_t lastWake = xTaskGetTickCount();

  for (;;)
  {
    pwmControlStep(targetCurrentMA, Kp);

    xTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10));
  }
}

void interfaceTask(void *pvParameters)
{
  TickType_t lastWake = xTaskGetTickCount();

  for (;;)
  {
    String scannedUID = scanUID();
    AuthState state = authState();

    if (state == AuthState::WaitingForChip)
    {
      screenScanNewChip();

      if (scannedUID.length() > 0)
      {
        if (completePendingUserCreation(scannedUID))
        {
          resetNumpad();
          screenReady();
          Serial.println("Ny bruger oprettet");
        }
        else
        {
          screenReady();
          screenUnknownChip();
        }
      }
    }
    else if (state == AuthState::WaitingForPin)
    {
      screenEnterPin();
      numpadLogik();

      if (isUserDone())
      {
        if (authUser(getTyped()))
        {
          screenLoggedIn(currentUserName());
          resetNumpad();
          Serial.println("ADGANG GODKENDT");
        }
        else
        {
          screenEnterPin();
          screenWrongPin();
          clearTyped();
          resetUserDone();
          Serial.println("FORKERT KODE!");
        }
      }
    }
    else if (authStatus())
    {
      screenLoggedIn(currentUserName());
      inaktivitetTjek();
    }
    else
    {
      if (scannedUID.length() > 0)
      {
        if (loadUserByUID(scannedUID))
        {
          resetNumpad();
          screenEnterPin();
        }
        else
        {
          screenReady();
          screenUnknownChip();
        }
      }
      else
      {
        screenReady();
      }
    }

    drawScreen();

    xTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10));
  }
}

void serverTask(void *pvParameters)
{
  TickType_t lastWake = xTaskGetTickCount();

  for (;;)
  {
    processServer();
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
  setupAuth();
  setupPwm();
  setupRFID();
  setupNumpad();
  setupScreen();
  setupServer();

  Serial.println("Starter tasks");

  xTaskCreatePinnedToCore(pwmTask, "pwm", 2048, NULL, 5, &pwmT, 1);
  xTaskCreatePinnedToCore(interfaceTask, "interface", 6144, NULL, 1, &interfaceT, 1);
  xTaskCreatePinnedToCore(serverTask, "server", 2048, NULL, 1, &serverT, 0);
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
}
