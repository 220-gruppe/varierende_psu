#include "pwm.h"

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