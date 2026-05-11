#ifndef PWM_H
#define PWM_H

#include <Arduino.h>
#include <driver/ledc.h>

#define PWM_GPIO 18
#define SHUNT_PIN 4
#define SHUNT_RESISTOR_OHM 50.0
#define PWM_FREQ_HZ 60000
#define PWM_RESOLUTION LEDC_TIMER_8_BIT
#define PWM_MODE LEDC_LOW_SPEED_MODE
#define PWM_TIMER LEDC_TIMER_0
#define PWM_CHANNEL LEDC_CHANNEL_0

void setupPwm();
void pwmControlStep(float targetCurrentMA, float kp);
float getCurrentMA();
float getTargetCurrentMA();
float getShuntVoltageMV();

#endif
