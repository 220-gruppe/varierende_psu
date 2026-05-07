#include "tempsensor.h"

extern DFRobot_MLX90614_I2C sensor(0x5A, &Wire);

float AVG_TEMP = 0.0f;

void setupTempSensor()
{
    while (NO_ERR != sensor.begin())
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
    sensor.enterSleepMode(false);
    delay(500);
    for (int i = 0; i < samples; i++)
    {
        float reading = sensor.getObjectTempCelsius();
        if (reading < 200.0f)
            sum += reading;
        delay(500);
    }
    sensor.enterSleepMode(true);
    AVG_TEMP = sum / samples;
    return AVG_TEMP;
}
