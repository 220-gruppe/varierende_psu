#include "rfid.h"

namespace
{
MFRC522 rc(RFID_SDA, RFID_RST);
}

void setupRFID()
{
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
