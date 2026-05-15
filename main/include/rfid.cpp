#include "rfid.h"
#include <SPI.h>

namespace
{
constexpr uint8_t RFID_SPI_MISO = 11;
constexpr uint8_t RFID_SPI_MOSI = 13;
constexpr uint8_t RFID_SPI_SCK = 12;
constexpr uint8_t RFID_SDA = 43;
constexpr uint8_t RFID_RST = 21;

MFRC522 rc(RFID_SDA, RFID_RST);
}

void setupRFID()
{
    pinMode(RFID_SDA, OUTPUT);
    digitalWrite(RFID_SDA, HIGH);

    SPI.begin(RFID_SPI_SCK, RFID_SPI_MISO, RFID_SPI_MOSI, RFID_SDA);
    rc.PCD_Init();
}

String scanUID()
{
    if (!rc.PICC_IsNewCardPresent() || !rc.PICC_ReadCardSerial())
    {
        return "";
    }

    String scannedUID = "";

    for (byte i = 0; i < rc.uid.size; i++)
    {
        scannedUID += (rc.uid.uidByte[i] < 0x10 ? "0" : "");
        scannedUID += String(rc.uid.uidByte[i], HEX);
    }

    scannedUID.toUpperCase();
    rc.PICC_HaltA();
    rc.PCD_StopCrypto1();
    Serial.print("Kort-UID fundet: ");
    Serial.println(scannedUID);
    return scannedUID;
}
