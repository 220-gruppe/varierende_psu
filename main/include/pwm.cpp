#include "pwm.h"
#include "driver/adc.h"
#include "soc/sens_reg.h"
#include "soc/soc.h"

namespace
{
constexpr adc1_channel_t SHUNT_ADC_CHANNEL = ADC1_CHANNEL_2; // GPIO3 on ESP32-S3.
constexpr int ADC_SAMPLES       = 50;
constexpr float ADC_MAX_RAW     = 4095.0f;
constexpr float ADC_0DB_MAX_MV  = 950.0f;

float currentDuty         = 0.0f;
unsigned long lastPrint   = 0;
float currentMA           = 0.0f;
float lastTargetCurrentMA = 0.0f;
float shuntVoltageMV      = 0.0f;

float energyDeliveredJ          = 0.0f;
unsigned long lastEnergyUpdate  = 0;
bool accumulatorActive          = false;

uint16_t readShuntAdcRegister()
{
  CLEAR_PERI_REG_MASK(SENS_SAR_MEAS1_CTRL2_REG, SENS_MEAS1_START_SAR);
  SET_PERI_REG_MASK(SENS_SAR_MEAS1_CTRL2_REG, SENS_MEAS1_START_FORCE);
  SET_PERI_REG_MASK(SENS_SAR_MEAS1_CTRL2_REG, SENS_MEAS1_START_SAR);

  while (!(GET_PERI_REG_MASK(SENS_SAR_MEAS1_CTRL2_REG, SENS_MEAS1_DONE_SAR)))
  {
  }

  return GET_PERI_REG_BITS2(
      SENS_SAR_MEAS1_CTRL2_REG,
      SENS_MEAS1_DATA_SAR_M,
      SENS_MEAS1_DATA_SAR_S);
}

float adcRawToMilliVolts(uint16_t raw)
{
  return (raw / ADC_MAX_RAW) * ADC_0DB_MAX_MV;
}
}

void setupPwm()
{
  pinMode(PWM_GPIO, OUTPUT);
  pinMode(SHUNT_PIN, INPUT);
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(SHUNT_ADC_CHANNEL, ADC_ATTEN_DB_0);
  adc1_get_raw(SHUNT_ADC_CHANNEL);

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
}

void resetPwmControl()
{
  currentDuty = 0.0f;
  stopPwmOutput();
}

void stopPwmOutput()
{
  currentDuty = 0.0f;
  ledc_set_duty(PWM_MODE, PWM_CHANNEL, 0);
  ledc_update_duty(PWM_MODE, PWM_CHANNEL);
}

void resetEnergyAccumulator()
{
  energyDeliveredJ  = 0.0f;
  lastEnergyUpdate  = 0;
  accumulatorActive = false;
}

void startEnergyAccumulator()
{
  energyDeliveredJ  = 0.0f;
  lastEnergyUpdate  = millis();
  accumulatorActive = true;
}

void pwmControlStep(float targetCurrentMA, float kp)
{
  lastTargetCurrentMA = targetCurrentMA;
  const uint32_t pwmMaxDuty = (1UL << PWM_RESOLUTION) - 1;
  uint32_t sumRaw = 0;

  for (int i = 0; i < ADC_SAMPLES; i++)
  {
    sumRaw += readShuntAdcRegister();
  }

  shuntVoltageMV = adcRawToMilliVolts(sumRaw / ADC_SAMPLES);
  currentMA = shuntVoltageMV / SHUNT_RESISTOR_OHM;
  float fejl = targetCurrentMA - currentMA;

  currentDuty += fejl * kp;

  if (currentDuty > pwmMaxDuty)
  {
    currentDuty = pwmMaxDuty;
  }

  if (currentDuty < 0)
  {
    currentDuty = 0;
  }

  ledc_set_duty(PWM_MODE, PWM_CHANNEL, static_cast<uint32_t>(currentDuty));
  ledc_update_duty(PWM_MODE, PWM_CHANNEL);

  if (accumulatorActive)
  {
    unsigned long now = millis();
    if (lastEnergyUpdate > 0)
   {
    float dt = (now - lastEnergyUpdate) / 1000.0f;  
    float I = currentMA / 1000.0f;
    energyDeliveredJ += I*I*SHUNT_RESISTOR_OHM*dt;
   }
   lastEnergyUpdate = now;
  }

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
    Serial.print(energyDeliveredJ);
    Serial.println("Energy: ");
  }
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

float getDeliveredEnergyJ()
{
  return energyDeliveredJ;
}
