#include "pwm.h"
#include "programs.h"

namespace
{
float currentDuty = 255.0f;
unsigned long lastPrint = 0;
float currentMA = 0.0f;
float lastTargetCurrentMA = 0.0f;
float shuntVoltageMV = 0.0f;

// Joule måling variabler
float totalJoule = 0.0;
bool reachedTarget = false;
unsigned long lastEnergyUpdate = 0;
}

void setupPwm()
{
  pinMode(PWM_GPIO, OUTPUT);
  pinMode(SHUTDOWN_PIN, OUTPUT);
  digitalWrite(SHUTDOWN_PIN, LOW);  // Shutdown pin LOW ved startup (gate driver disabled)
  
  pinMode(SHUNT_PIN, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(SHUNT_PIN, ADC_0db);
  analogSetPinAttenuation(VOLTAGE_ADC, ADC_11db);


  ledc_timer_config_t timer_cfg = {};
  timer_cfg.speed_mode = PWM_MODE;
  timer_cfg.duty_resolution = PWM_RESOLUTION;
  timer_cfg.timer_num = PWM_TIMER;
  timer_cfg.freq_hz = PWM_FREQ_HZ;
  timer_cfg.clk_cfg = LEDC_AUTO_CLK;
  ESP_ERROR_CHECK(ledc_timer_config(&timer_cfg));

  ledc_channel_config_t channel_cfg = {};
  channel_cfg.gpio_num = PWM_GPIO;
  channel_cfg.speed_mode = PWM_MODE;
  channel_cfg.channel = PWM_CHANNEL;
  channel_cfg.intr_type = LEDC_INTR_DISABLE;
  channel_cfg.timer_sel = PWM_TIMER;
  channel_cfg.duty = 0;
  channel_cfg.hpoint = 0;
  ESP_ERROR_CHECK(ledc_channel_config(&channel_cfg));
  
  // Start PWM med duty 0 (sikker tilstand)
  ledc_set_duty(PWM_MODE, PWM_CHANNEL, 0);
  ledc_update_duty(PWM_MODE, PWM_CHANNEL);
}

void pwmControlStep(float targetCurrentMA, float kp)
{
  lastTargetCurrentMA = targetCurrentMA;
  const uint32_t pwmMaxDuty = (1UL << PWM_RESOLUTION) - 1;
  uint32_t sumMV = 0;

  for (int i = 0; i < 50; i++)
  {
    sumMV += analogReadMilliVolts(SHUNT_PIN);
  }

  shuntVoltageMV = sumMV / 50.0f;
  currentMA = shuntVoltageMV / SHUNT_RESISTOR_OHM;

  if (svejseAktiv)
  {
    float voltageV = readVoltage();
    float voltageBUS = voltageV * 8.203;

    float watts = voltageBUS * (currentMA / 1000.0f);
    totalJoule += watts * 0.02; // 20ms = 0.02s

    float currentTarget = getTargetJoule();
    if (totalJoule >= currentTarget && currentTarget > 0.0f) {
      reachedTarget = true;
      Serial.println("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      Serial.print("TARGET NÅET: "); Serial.print(totalJoule); Serial.print(" J (target: "); Serial.print(currentTarget); Serial.println(" J)");
      Serial.println("SYSTEM SLUKKET.");
      Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    }

    if (millis() - lastPrint > 200)
    {
      lastPrint = millis();
      float voltageV_print = readVoltage();
      float pwmPct = (currentDuty / pwmMaxDuty) * 100;

      Serial.print("V_load: "); Serial.print(voltageBUS, 2);
      Serial.print("V | mA: "); Serial.print(currentMA, 0);
      Serial.print(" | PWM: "); Serial.print(pwmPct, 1);
      Serial.print("% | Joule: "); Serial.println(totalJoule, 2);
    }
  }

  float fejl = targetCurrentMA - currentMA;

  currentDuty += fejl * kp;

  if (currentDuty > 460) currentDuty = 460; // 90%
  if (currentDuty < 77) currentDuty = 77;   // 15%

  ledc_set_duty(PWM_MODE, PWM_CHANNEL, static_cast<uint32_t>(currentDuty));
  ledc_update_duty(PWM_MODE, PWM_CHANNEL);
}

float getCurrentMA()
{
  return currentMA;
} 

float getTargetCurrentMA()
{
  return lastTargetCurrentMA;
}

float getShuntVoltageMV()
{
  return shuntVoltageMV;
}

float readVoltage (){
  uint32_t sumMV = 0;
  for (int i = 0; i < 10; i++) {
    sumMV += analogReadMilliVolts(VOLTAGE_ADC);
  }
  float voltageMV = sumMV / 10.0f;
  return voltageMV / 1000.0f;
}

float calculatePowerW() {
  float currentA = getCurrentMA() / 1000.0f; 
  float voltageV = readVoltage();
  return currentA * voltageV;
}

void resetEnergy() {
  totalJoule = 0.0;
  reachedTarget = false;
  lastEnergyUpdate = millis();
}

float getTotalJoule() {
  return totalJoule;
}

bool hasReachedTarget() {
  return reachedTarget;
}

void enableSvejsning() {
  digitalWrite(SHUTDOWN_PIN, HIGH);  
  Serial.println("Svejsning ENABLED (Shutdown pin HIGH)");
}

void disableSvejsning() {
  digitalWrite(SHUTDOWN_PIN, LOW);   
  ledc_set_duty(PWM_MODE, PWM_CHANNEL, 0); 
  ledc_update_duty(PWM_MODE, PWM_CHANNEL);
  Serial.println("Svejsning DISABLED (Shutdown pin LOW, PWM = 0)");
}

