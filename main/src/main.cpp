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
#define I2C_SDA1 21
#define I2C_SCL1 16
#define I2C_SDA2 17
#define I2C_SCL2 18

TwoWire I2C2 = TwoWire(1);
DFRobot_MLX90614_I2C sensor;

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
    processAuthenticationInterfaceState();
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

  // 1ST I2C BUS (NUMPAD)
  Wire.begin(I2C_SDA1, I2C_SCL1);

  // 2ND I2C BUS (TEMPSSENSOR)
  I2C2.begin(I2C_SDA2, I2C_SCL2);
  sensor.begin();

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
