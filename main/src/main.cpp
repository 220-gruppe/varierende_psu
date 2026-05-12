// freeRTOS
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <SPI.h>
#include <Wire.h>

// Include
#include "database.h"
#include "interface_state.h"
#include "server.h"
#include "pwm.h"
#include "auth.h"
#include "rfid.h"
#include "numpad.h"
#include "screen.h"
#include "tempsensor.h"
#include "programs.h"

// SPI
#define SPI_MISO 11
#define SPI_MOSI 13
#define SPI_SCK 12

// I2C
#define I2C_SDA 16
#define I2C_SCL 17

float heatInput = 70000; // aendres til noget fra sensor
float targetCurrentMA = 5.0;
float Kp = 1.1;

TaskHandle_t auth_interfaceT = nullptr;
// TaskHandle_t user_interfaceT = nullptr;
TaskHandle_t serverT = nullptr;
TaskHandle_t pwmT = nullptr;

void pwmTask(void *pvParameters)
{
  TickType_t lastWake = xTaskGetTickCount();

  for (;;)
  {
    pwmControlStep(targetCurrentMA, Kp);
    xTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10));
  }
}

void auth_interfaceTask(void *pvParameters)
{
  TickType_t lastWake = xTaskGetTickCount();

  for (;;)
  {
    processAuthenticationInterfaceState();
    processUserInterfaceState();
    drawScreen();
    xTaskDelayUntil(&lastWake, pdMS_TO_TICKS(100));
  }
}

/*
void user_interfaceTask(void *pvParameters)
{
  TickType_t lastWake = xTaskGetTickCount();

  for (;;)
  {
    processUserInterfaceState();
    xTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10));
  }
}
*/

void serverTask(void *pvParameters)
{
  TickType_t lastWake = xTaskGetTickCount();

  for (;;)
  {
    processServer();
    xTaskDelayUntil(&lastWake, pdMS_TO_TICKS(100));
  }
}

void setup()
{
  Serial.begin(115200);
  delay(3000);
  Serial.println("Boot startet");

  // Start SPI
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  // Start I2C
  Wire.setPins(I2C_SDA, I2C_SCL);
  Wire.begin(I2C_SDA, I2C_SCL);

  setupDatabase();
  setupAuth();
  setupPwm();
  setupRFID();
  setupTempSensor();
  setupNumpad();
  setupScreen();
  setupServer();

  Serial.println("Starter tasks");

  // xTaskCreatePinnedToCore(pwmTask, "pwm", 2048, NULL, 3, &pwmT, 1);
  xTaskCreatePinnedToCore(auth_interfaceTask, "auth_interface", 4096, NULL, 2, &auth_interfaceT, 1);
  // xTaskCreatePinnedToCore(user_interfaceTask, "user_interface", 8192, NULL, 1, &user_interfaceT, 1);
  xTaskCreatePinnedToCore(serverTask, "server", 4096, NULL, 1, &serverT, 0);

  /*==============================================================*/
  // DEBUGGING
  if (auth_interfaceT == nullptr)
    Serial.println("auth_interfaceT FEJL - kunne ikke oprettes");
  else
    Serial.println("auth_interfaceT OK");

  // if (user_interfaceT == nullptr)
  //   Serial.println("user_interfaceT FEJL - kunne ikke oprettes");
  // else
  //   Serial.println("user_interfaceT OK");

  if (serverT == nullptr)
    Serial.println("serverTask FEJL - kunne ikke oprettes");
  else
    Serial.println("serverTask OK");
  /*==============================================================*/

  DB("users", "UID,USER,PASSWORD");
  databaseRead();
  DB("svejse_logs","STATUS, CALCULATED_ENERGY, TARGET_ENERGY, AVG_TEMP, SVEJSNING_TIME");
  databaseRead(); 
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
}
