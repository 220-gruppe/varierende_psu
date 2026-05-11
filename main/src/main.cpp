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

TaskHandle_t interfaceT = nullptr;
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

void interfaceTask(void *pvParameters)
{
  TickType_t lastWake = xTaskGetTickCount();

  for (;;)
  {
    processInterfaceState();
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

  //xTaskCreatePinnedToCore(pwmTask, "pwm", 2048, NULL, 3, &pwmT, 1);
  xTaskCreatePinnedToCore(interfaceTask, "interface", 6144, NULL, 1, &interfaceT, 1);
  xTaskCreatePinnedToCore(serverTask, "server", 6144, NULL, 1, &serverT, 0);
  
  DB("users", "UID,USER,PASSWORD");
  databaseRead();
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
}
