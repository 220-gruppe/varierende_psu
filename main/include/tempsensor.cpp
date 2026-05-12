#include "tempsensor.h"

namespace
{
constexpr uint8_t I2C_SDA = 16;
constexpr uint8_t I2C_SCL = 17;
constexpr uint8_t MLX90614_ADDR = 0x5E;
constexpr uint8_t MLX90614_ID_NUMBER = 0x1C | 0x20;
constexpr uint8_t MLX90614_TOBJ1 = 0x07;

bool readMlxRegister(uint8_t reg, uint16_t &value)
{
    Wire.beginTransmission(MLX90614_ADDR);
    Wire.write(reg);

    if (Wire.endTransmission(false) != 0)
    {
        return false;
    }

    const uint8_t bytesRead = Wire.requestFrom(MLX90614_ADDR, static_cast<uint8_t>(3));
    if (bytesRead < 3)
    {
        while (Wire.available())
        {
            Wire.read();
        }
        return false;
    }

    uint8_t lsb = Wire.read();
    uint8_t msb = Wire.read();
    Wire.read(); // PEC byte, not needed for this simple read path.

    value = (static_cast<uint16_t>(msb) << 8) | lsb;
    return value != 0;
}

bool mlxIsReady()
{
    Wire.begin(I2C_SDA, I2C_SCL);

    uint16_t sensorId = 0;
    return readMlxRegister(MLX90614_ID_NUMBER, sensorId);
}

float readObjectTempCelsius()
{
    uint16_t rawTemp = 0;
    if (!readMlxRegister(MLX90614_TOBJ1, rawTemp))
    {
        return NAN;
    }

    return rawTemp * 0.02f - 273.15f;
}
}

float AVG_TEMP = 0.0f;

void setupTempSensor()
{
    while (!mlxIsReady())
    {
        Serial.println("Fejl ved initialisering af temp sensor, prøver igen...");
        delay(300);
    }
    Serial.println("TEMPS sensor OK");
}

float takeTempMeasurement()
{
    const int samples = 20;
    float sum = 0.0f;
    int validSamples = 0;

    for (int i = 0; i < samples; i++)
    {
        float reading = readObjectTempCelsius();
        if (!isnan(reading) && reading < 200.0f)
        {
            sum += reading;
            validSamples++;
        }
        delay(500);
    }

    if (validSamples > 0)
    {
        AVG_TEMP = sum / validSamples;
    }

    return AVG_TEMP;
}
