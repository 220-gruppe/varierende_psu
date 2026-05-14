#include "pwm.h"
#include "programs.h"
#include "svejse_logs.h"

#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <esp_adc/adc_oneshot.h>

namespace
{
constexpr uint8_t PWM_GPIO = 1;
constexpr uint8_t SHUTDOWN_PIN = 2;
constexpr uint8_t SHUNT_PIN = 3;
constexpr uint8_t VOLTAGE_ADC = 18;
constexpr uint32_t PWM_FREQ_HZ = 50000;
constexpr ledc_timer_bit_t PWM_RESOLUTION = LEDC_TIMER_9_BIT;
constexpr ledc_mode_t PWM_MODE = LEDC_LOW_SPEED_MODE;
constexpr ledc_timer_t PWM_TIMER = LEDC_TIMER_0;
constexpr ledc_channel_t PWM_CHANNEL = LEDC_CHANNEL_0;

float currentDuty = 255.0f;
unsigned long lastPrint = 0;
float currentMA = 0.0f;
float lastTargetCurrentMA = 0.0f;
float shuntVoltageMV = 0.0f;
float totalJoule = 0.0f;
WeldFault activeFault = WeldFault::None;
WeldFault pendingFault = WeldFault::None;
unsigned long pendingFaultSinceMs = 0;
adc_oneshot_unit_handle_t adc1Handle = nullptr;
adc_oneshot_unit_handle_t adc2Handle = nullptr;
adc_unit_t shuntAdcUnit = ADC_UNIT_1;
adc_channel_t shuntAdcChannel = ADC_CHANNEL_0;
adc_unit_t voltageAdcUnit = ADC_UNIT_1;
adc_channel_t voltageAdcChannel = ADC_CHANNEL_0;
bool shuntAdcReady = false;
bool voltageAdcReady = false;
adc_cali_handle_t shuntAdcCali = nullptr;
adc_cali_handle_t voltageAdcCali = nullptr;

adc_oneshot_unit_handle_t *adcHandleSlot(adc_unit_t unit)
{
  return unit == ADC_UNIT_1 ? &adc1Handle : &adc2Handle;
}

bool ensureAdcUnit(adc_unit_t unit)
{
  adc_oneshot_unit_handle_t *handle = adcHandleSlot(unit);
  if (*handle != nullptr)
  {
    return true;
  }

  adc_oneshot_unit_init_cfg_t unitConfig = {};
  unitConfig.unit_id = unit;
  unitConfig.ulp_mode = ADC_ULP_MODE_DISABLE;

  esp_err_t err = adc_oneshot_new_unit(&unitConfig, handle);
  if (err != ESP_OK)
  {
    Serial.print("ADC unit init failed: ");
    Serial.println((int)err);
    *handle = nullptr;
    return false;
  }

  return true;
}

bool setupAdcCalibration(uint8_t pin, adc_unit_t unit, adc_channel_t channel, adc_atten_t attenuation, adc_cali_handle_t &caliHandle)
{
  caliHandle = nullptr;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
  adc_cali_curve_fitting_config_t caliConfig = {};
  caliConfig.unit_id = unit;
  caliConfig.chan = channel;
  caliConfig.atten = attenuation;
  caliConfig.bitwidth = ADC_BITWIDTH_12;

  esp_err_t err = adc_cali_create_scheme_curve_fitting(&caliConfig, &caliHandle);
  if (err == ESP_OK)
  {
    Serial.print("ADC eFuse calibration OK pin ");
    Serial.println(pin);
    return true;
  }
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
  adc_cali_line_fitting_config_t caliConfig = {};
  caliConfig.unit_id = unit;
  caliConfig.atten = attenuation;
  caliConfig.bitwidth = ADC_BITWIDTH_12;

  esp_err_t err = adc_cali_create_scheme_line_fitting(&caliConfig, &caliHandle);
  if (err == ESP_OK)
  {
    Serial.print("ADC eFuse calibration OK pin ");
    Serial.println(pin);
    return true;
  }
#endif

  Serial.print("ADC eFuse calibration ikke tilgaengelig pin ");
  Serial.print(pin);
  Serial.println(" - bruger fallback scale");
  return false;
}

bool configureFastAdcPin(uint8_t pin, adc_atten_t attenuation, adc_unit_t &unit, adc_channel_t &channel, adc_cali_handle_t &caliHandle)
{
  esp_err_t err = adc_oneshot_io_to_channel(pin, &unit, &channel);
  if (err != ESP_OK)
  {
    Serial.print("Pin er ikke ADC pin: ");
    Serial.println(pin);
    return false;
  }

  if (!ensureAdcUnit(unit))
  {
    return false;
  }

  adc_oneshot_chan_cfg_t channelConfig = {};
  channelConfig.bitwidth = ADC_BITWIDTH_12;
  channelConfig.atten = attenuation;

  err = adc_oneshot_config_channel(*adcHandleSlot(unit), channel, &channelConfig);
  if (err != ESP_OK)
  {
    Serial.print("ADC channel config failed pin ");
    Serial.print(pin);
    Serial.print(": ");
    Serial.println((int)err);
    return false;
  }

  setupAdcCalibration(pin, unit, channel, attenuation, caliHandle);
  return true;
}

int readFastAdcRaw(adc_unit_t unit, adc_channel_t channel)
{
  adc_oneshot_unit_handle_t handle = *adcHandleSlot(unit);
  if (handle == nullptr)
  {
    return 0;
  }

  int raw = 0;
  if (adc_oneshot_read(handle, channel, &raw) != ESP_OK)
  {
    return 0;
  }

  return raw < 0 ? 0 : raw;
}

float rawToMilliVolts(float raw, float fullScaleMV, adc_cali_handle_t caliHandle)
{
  if (caliHandle != nullptr)
  {
    int calibratedMV = 0;
    int roundedRaw = static_cast<int>(raw + 0.5f);
    if (adc_cali_raw_to_voltage(caliHandle, roundedRaw, &calibratedMV) == ESP_OK)
    {
      return calibratedMV;
    }
  }

  return (raw * fullScaleMV) / ADC_RAW_MAX;
}

float readFastAdcMilliVolts(adc_unit_t unit, adc_channel_t channel, int samples, float fullScaleMV, adc_cali_handle_t caliHandle)
{
  if (samples <= 0)
  {
    return 0.0f;
  }

  uint32_t sumRaw = 0;
  for (int i = 0; i < samples; i++)
  {
    sumRaw += readFastAdcRaw(unit, channel);
  }

  float averageRaw = sumRaw / static_cast<float>(samples);
  return rawToMilliVolts(averageRaw, fullScaleMV, caliHandle);
}

const char *faultText(WeldFault fault)
{
  switch (fault)
  {
  case WeldFault::LowVoltage:
    return "FEJL_SPAENDING_LAV";
  case WeldFault::LowCurrent:
    return "FEJL_STROEM_LAV";
  case WeldFault::None:
  default:
    return "NONE";
  }
}

WeldFault detectWeldFault(float voltageBUS, float measuredCurrentMA, float targetCurrentMA)
{
  if (getSvejseElapsedTime() < WELD_FAULT_GRACE_MS)
  {
    return WeldFault::None;
  }

  if (voltageBUS < WELD_MIN_VALID_VOLTAGE_V)
  {
    return WeldFault::LowVoltage;
  }

  if (targetCurrentMA > WELD_MIN_VALID_CURRENT_MA && measuredCurrentMA < WELD_MIN_VALID_CURRENT_MA)
  {
    return WeldFault::LowCurrent;
  }

  return WeldFault::None;
}

void updateWeldFault(WeldFault detectedFault, unsigned long nowMs)
{
  if (activeFault != WeldFault::None)
  {
    return;
  }

  if (detectedFault == WeldFault::None)
  {
    pendingFault = WeldFault::None;
    pendingFaultSinceMs = 0;
    return;
  }

  if (pendingFault != detectedFault)
  {
    pendingFault = detectedFault;
    pendingFaultSinceMs = nowMs;
    return;
  }

  if (nowMs - pendingFaultSinceMs < WELD_FAULT_CONFIRM_MS)
  {
    return;
  }

  activeFault = detectedFault;
  Serial.print("SVEJSEFEJL: ");
  Serial.println(faultText(activeFault));
}
}

void setupPwm()
{
  pinMode(PWM_GPIO, OUTPUT);
  pinMode(SHUTDOWN_PIN, OUTPUT);
  digitalWrite(SHUTDOWN_PIN, LOW);

  pinMode(SHUNT_PIN, INPUT);
  pinMode(VOLTAGE_ADC, INPUT);
  shuntAdcReady = configureFastAdcPin(SHUNT_PIN, ADC_ATTEN_DB_0, shuntAdcUnit, shuntAdcChannel, shuntAdcCali);
  voltageAdcReady = configureFastAdcPin(VOLTAGE_ADC, ADC_ATTEN_DB_12, voltageAdcUnit, voltageAdcChannel, voltageAdcCali);

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

  stopPwmOutput();
}

void pwmControlStep(float targetCurrentMA, float kp)
{
  lastTargetCurrentMA = targetCurrentMA;
  const uint32_t pwmMaxDuty = (1UL << PWM_RESOLUTION) - 1;
  shuntVoltageMV = shuntAdcReady
                       ? readFastAdcMilliVolts(shuntAdcUnit, shuntAdcChannel, SHUNT_ADC_SAMPLES, SHUNT_ADC_FULL_SCALE_MV, shuntAdcCali)
                       : 0.0f;
  currentMA = shuntVoltageMV / SHUNT_RESISTOR_OHM;

  if (!svejseAktiv || targetCurrentMA <= 0.0f)
  {
    stopPwmOutput();
    return;
  }

  float voltageV = readVoltage();
  Serial.print("voltage: ");
  Serial.println(voltageV);
  float voltageBUS = voltageV * VOLTAGE_DIVIDER_RATIO;
  float watts = voltageBUS * (currentMA / 1000.0f);
  totalJoule += watts * 0.02f;
  unsigned long nowMs = millis();
  appendSvejsningMeasurement(nowMs, currentMA, voltageBUS, totalJoule);

  updateWeldFault(detectWeldFault(voltageBUS, currentMA, targetCurrentMA), nowMs);
  if (hasWeldFault())
  {
    stopPwmOutput();
    return;
  }

  if (nowMs - lastPrint > PWM_DEBUG_PRINT_INTERVAL_MS)
  {
    lastPrint = millis();
    float pwmPct = (currentDuty / pwmMaxDuty) * 100.0f;

    Serial.print("V_load: ");
    Serial.print(voltageBUS, 2);
    Serial.print("V | mA: ");
    Serial.print(currentMA, 0);
    Serial.print(" / target: ");
    Serial.print(targetCurrentMA, 0);
    Serial.print(" | PWM: ");
    Serial.print(pwmPct, 1);
    Serial.print("% | Joule: ");
    Serial.println(totalJoule, 2);
  }

  float fejl = targetCurrentMA - currentMA;
  currentDuty += fejl * kp;

  if (currentDuty > PWM_MAX_DUTY)
    currentDuty = PWM_MAX_DUTY;
  if (currentDuty < PWM_MIN_DUTY)
    currentDuty = PWM_MIN_DUTY;

  ledc_set_duty(PWM_MODE, PWM_CHANNEL, static_cast<uint32_t>(currentDuty));
  ledc_update_duty(PWM_MODE, PWM_CHANNEL);
}

void resetPwmControl()
{
  currentDuty = PWM_MIN_DUTY;
  lastPrint = 0;
  ledc_set_duty(PWM_MODE, PWM_CHANNEL, static_cast<uint32_t>(currentDuty));
  ledc_update_duty(PWM_MODE, PWM_CHANNEL);
}

void stopPwmOutput()
{
  ledc_set_duty(PWM_MODE, PWM_CHANNEL, 0);
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

float readVoltage()
{
  if (!voltageAdcReady)
  {
    return 0.0f;
  }

  float voltageMV = readFastAdcMilliVolts(voltageAdcUnit, voltageAdcChannel, VOLTAGE_ADC_SAMPLES, VOLTAGE_ADC_FULL_SCALE_MV, voltageAdcCali);
  return voltageMV / 1000.0f;
}

float calculatePowerW()
{
  float currentA = getCurrentMA() / 1000.0f;
  return currentA * readVoltage();
}

void resetEnergy()
{
  totalJoule = 0.0f;
}

float getTotalJoule()
{
  return totalJoule;
}

bool hasReachedTarget()
{
  return false;
}

void resetWeldFault()
{
  activeFault = WeldFault::None;
  pendingFault = WeldFault::None;
  pendingFaultSinceMs = 0;
}

bool hasWeldFault()
{
  return activeFault != WeldFault::None;
}

WeldFault getWeldFault()
{
  return activeFault;
}

const char *weldFaultText()
{
  return faultText(activeFault);
}

void enableSvejsning()
{
  digitalWrite(SHUTDOWN_PIN, HIGH);
  Serial.println("Svejsning ENABLED (Shutdown pin HIGH)");
}

void disableSvejsning()
{
  digitalWrite(SHUTDOWN_PIN, LOW);
  stopPwmOutput();
  Serial.println("Svejsning DISABLED (Shutdown pin LOW, PWM = 0)");
}
