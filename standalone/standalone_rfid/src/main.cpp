#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

// Common SPI connections
#define SPI_MISO 11
#define SPI_MOSI 13
#define SPI_SCK 12

// RFID pins
#define RFID_SDA 2
#define RFID_RST 1

// SD pins
#define SD_CS 10

MFRC522 rc(RFID_SDA, RFID_RST);

String scannedUID = "";

void setup()
{
  Serial.begin(115200);
  delay(2000);

  // Start SPI
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  rc.PCD_Init();
  delay(100);

  rc.PCD_DumpVersionToSerial(); // 0x12 for klon

  Serial.println("Setup done");
}

void searchUID()
{
  bool fundet = false;

  while (!fundet)
  {
    if (rc.PICC_IsNewCardPresent() && rc.PICC_ReadCardSerial())
    {
      scannedUID = "";

      for (byte i = 0; i < rc.uid.size; i++)
      {
        scannedUID += (rc.uid.uidByte[i] < 0x10 ? "0" : "");
        scannedUID += String(rc.uid.uidByte[i], HEX);
      }
      scannedUID.toUpperCase();

      fundet = true;
    }
    yield();
    delay(10);
  }
  rc.PICC_HaltA();
  rc.PCD_StopCrypto1();

  Serial.println("UID fundet:");
  Serial.println(scannedUID);
}

void loop()
{
  Serial.println("scanner efter uid:");
  searchUID(); //finder UID

  delay(1000);
}
