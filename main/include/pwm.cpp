#include "pwm.h"

namespace
{
float currentDuty = 0.0f;
unsigned long lastPrint = 0;
}

void setupPwm()
{
  pinMode(PWM_GPIO, OUTPUT);
  pinMode(SHUNT_PIN, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(SHUNT_PIN, ADC_0db);

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

void pwmControlStep(float targetCurrentMA, float kp)
{
  const uint32_t pwmMaxDuty = (1UL << PWM_RESOLUTION) - 1;
  uint32_t sumMV = 0;

  for (int i = 0; i < 50; i++)
  {
    sumMV += analogReadMilliVolts(SHUNT_PIN);
  }

  float shuntVoltageMV = sumMV / 50.0f;
  float currentMA = shuntVoltageMV / SHUNT_RESISTOR_OHM;
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
}
