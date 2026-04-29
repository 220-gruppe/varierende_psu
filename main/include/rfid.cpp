#include "rfid.h"

void setupRFID(){
    pinMode(RFID_SDA, OUTPUT);
}

String searchUID()
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

String scanCard()
{
    if (!isLoggedIn)
    {
        if (rc.PICC_IsNewCardPresent() && rc.PICC_ReadCardSerial())
        {
            String fundetUID = "";
            for (byte i = 0; i < rc.uid.size; i++)
            {
                fundetUID += (rc.uid.uidByte[i] < 0x10 ? "0" : "");
                fundetUID += String(rc.uid.uidByte[i], HEX);
            }
            fundetUID.toUpperCase();

            rc.PICC_HaltA();
            rc.PCD_StopCrypto1();

            return fundetUID;
        }
    }
}