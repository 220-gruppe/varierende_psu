#ifndef PWM_H
#define PWM_H

#include <Arduino.h>
#include <driver/ledc.h>

extern const float SHUNT_RESISTOR_OHM;
extern const float VOLTAGE_DIVIDER_RATIO;
extern const int SHUNT_ADC_SAMPLES;
extern const int VOLTAGE_ADC_SAMPLES;
extern const float ADC_RAW_MAX;
extern const float SHUNT_ADC_FULL_SCALE_MV;
extern const float VOLTAGE_ADC_FULL_SCALE_MV;
extern const float PWM_MIN_DUTY;
extern const float PWM_MAX_DUTY;
extern const unsigned long PWM_DEBUG_PRINT_INTERVAL_MS;
extern const float WELD_MIN_VALID_VOLTAGE_V;
extern const float WELD_MIN_VALID_CURRENT_MA;
extern const unsigned long WELD_FAULT_GRACE_MS;
extern const unsigned long WELD_FAULT_CONFIRM_MS;

enum class WeldFault
{
    None,
    LowVoltage,
    LowCurrent
};

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
float readVoltage();
float calculatePowerW();
void resetEnergy();
float getTotalJoule();
bool hasReachedTarget();
void resetWeldFault();
bool hasWeldFault();
WeldFault getWeldFault();
const char *weldFaultText();
void enableSvejsning();
void disableSvejsning();


#endif
