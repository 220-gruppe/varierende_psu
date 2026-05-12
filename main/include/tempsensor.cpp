#include "tempsensor.h"
#include <Adafruit_MLX90614.h>

namespace
{
    constexpr uint8_t I2C_SDA = 16;
    constexpr uint8_t I2C_SCL = 17;
    constexpr uint8_t TEMP_SENSOR_ADDR = 0x5E;

    Adafruit_MLX90614 mlx = Adafruit_MLX90614();

    bool mlxIsReady()
    {
        Wire.begin(I2C_SDA, I2C_SCL);
        return mlx.begin(TEMP_SENSOR_ADDR);
    }

    float readObjectTempCelsius()
    {
        float reading = mlx.readObjectTempC();
        if (isnan(reading))
        {
            return NAN;
        }

        return reading;
    }
}

float AVG_TEMP = 0.0f;

void setupTempSensor()
{
    while (!mlxIsReady())
    {
        Serial.println("Fejl ved initialisering af temp sensor, prover igen...");
        delay(300);
    }
    Serial.println("MLX90614 temp sensor OK");
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
            delay(20);
        }

        if (validSamples > 0)
        {
            AVG_TEMP = sum / validSamples;
        }

        return AVG_TEMP;
    }
}