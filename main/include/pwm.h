#ifndef PWM_H
#define PWM_H

#include <Arduino.h>
#include <driver/ledc.h>

#define PWM_GPIO            1
#define SHUNT_PIN           3
#define SHUNT_RESISTOR_OHM  0.1055
#define PWM_FREQ_HZ         50000
#define PWM_RESOLUTION      LEDC_TIMER_9_BIT
#define PWM_MODE            LEDC_LOW_SPEED_MODE
#define PWM_TIMER           LEDC_TIMER_0
#define PWM_CHANNEL         LEDC_CHANNEL_0

void setupPwm();
void resetPwmControl();
void stopPwmOutput();
void pwmControlStep(float targetCurrentMA, float kp);

float getCurrentMA();
float getTargetCurrentMA();
float getShuntVoltageMV();
float getDeliveredEnergyJ();

void resetEnergyAccumulator();
void startEnergyAccumulator();

#endif
