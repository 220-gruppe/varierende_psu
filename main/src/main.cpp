// freeRTOS
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>

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
#include "svejse_logs.h"

// ==============================================================
// KONFIGURATION - juster normale systemparametre her i main.cpp
// ==============================================================

// SD-kort
extern const int SD_MOUNT_ATTEMPTS = 3;
extern const unsigned long SD_MOUNT_RETRY_DELAY_MS = 300;

// Numpad
extern const uint8_t NUMPAD_PIN_LENGTH = 4;
extern const unsigned long NUMPAD_DEBOUNCE_MS = 250;

// Temperatursensor
extern const int TEMP_SENSOR_SAMPLES = 20;
extern const unsigned long TEMP_SENSOR_SAMPLE_DELAY_MS = 20;
extern const float TEMP_SENSOR_MAX_VALID_C = 200.0f;

// Svejseprogrammer: navn, svejsetid i ms, target stroem i mA
extern const WeldProgram WELD_PROGRAMS[] = {
  {"Program 1 ", 50000UL, 1000.0f},
  {"Program 2 ", 60000UL, 1500.0f},
  {"Program 3 ", 70000UL, 2000.0f},
  {"Program 4 ", 183000UL, 3000.0f},
};
extern const uint8_t WELD_PROGRAM_COUNT = sizeof(WELD_PROGRAMS) / sizeof(WELD_PROGRAMS[0]);

// Svejse/PWM tuning
float Kp = 0.1f;
extern const float SHUNT_RESISTOR_OHM = 0.1055f;
extern const float VOLTAGE_DIVIDER_RATIO = 8.203f;
extern const int SHUNT_ADC_SAMPLES = 50;
extern const int VOLTAGE_ADC_SAMPLES = 10;
extern const float ADC_RAW_MAX = 4095.0f;
extern const float SHUNT_ADC_FULL_SCALE_MV = 950.0f;
extern const float VOLTAGE_ADC_FULL_SCALE_MV = 3100.0f;
extern const float PWM_MIN_DUTY = 77.0f;
extern const float PWM_MAX_DUTY = 460.0f;
extern const unsigned long PWM_DEBUG_PRINT_INTERVAL_MS = 200;
extern const float WELD_MIN_VALID_VOLTAGE_V = 0.5f;
extern const float WELD_MIN_VALID_CURRENT_MA = 50.0f;
extern const unsigned long WELD_FAULT_GRACE_MS = 700;
extern const unsigned long WELD_FAULT_CONFIRM_MS = 300;

// Svejselog / energiberegning
extern const float OUTPUT_RESISTANCE_OHM = 0.3729f;
extern const unsigned long SVEJSNING_MEASUREMENT_INTERVAL_MS = 1000;

// UI timing
extern const unsigned long INACTIVITY_TIMEOUT_MS = 240000;
extern const unsigned long TEMP_DISPLAY_MS = 3000;

TaskHandle_t auth_interfaceT = nullptr;
TaskHandle_t serverT = nullptr;
TaskHandle_t pwmT = nullptr;

void pwmTask(void *pvParameters)
{
  TickType_t lastWake = xTaskGetTickCount();

  for (;;)
  {
    pwmControlStep(getSvejseTargetCurrentMA(), Kp);
    xTaskDelayUntil(&lastWake, pdMS_TO_TICKS(20));
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
  Serial.print("Reset reason: ");
  Serial.println((int)esp_reset_reason());

  setupDatabase();
  removeSvejsningTestLogs();
  printSdCardContents();

  setupAuth();
  setupPwm();
  setupRFID();
  setupTempSensor();
  setupNumpad();
  setupScreen();
  setupServer();

  Serial.println("Starter tasks");

  xTaskCreatePinnedToCore(pwmTask, "pwm", 6144, NULL, 3, &pwmT, 1);
  xTaskCreatePinnedToCore(auth_interfaceTask, "auth_interface", 12288, NULL, 2, &auth_interfaceT, 1);
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

}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
}
