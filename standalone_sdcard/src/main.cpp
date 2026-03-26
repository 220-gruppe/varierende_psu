#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#define SD_MISO 13
#define SD_MOSI 11
#define SD_SCK 12
#define SD_CS 1

// opret ny instans
SPIClass sdSPI(FSPI);

void setup()
{
  Serial.begin(115200);
  delay(2000);

  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS, sdSPI))
  {
    Serial.println("3 - SD FAIL");
    return;
  }

  Serial.println("4 - SD OK");
}

void loop()
{
  Serial.println("jfoædjhglæd2313213");
  delay(1000);
}