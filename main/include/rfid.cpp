#include "rfid.h"


MFRC522 rc;

void setupRFID(){
    pinMode(RFID_SDA, OUTPUT);
    rc.PCD_Init();
}

String scanUID()
{ 
    String scannedUID;
    bool fundet;
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
        delay(50);
    }
    rc.PICC_HaltA();
    rc.PCD_StopCrypto1();

    return scannedUID;
    
    Serial.print("Kort-UID fundet: ");
    Serial.println(scannedUID);
}