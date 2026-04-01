#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <config.h>
#include <variabler.h>
#include <server.h>

void createFile()
{
    if (!SD.exists("/dokumentation.csv")) // hvis der ikke findes en fil allerede, opret en
    {
        svejsningData = SD.open("/dokumentation.csv", FILE_WRITE); // opret fil
        if (svejsningData)                                         // skriv overskfit
        {
            svejsningData.print("NR");
            svejsningData.print(";");
            svejsningData.print("Operatoer_ID");
            svejsningData.print(";");
            svejsningData.print("Heat_Input");
            svejsningData.print(";");
            svejsningData.println("Timestamp");

            svejsningData.close();
            Serial.println("Opretter ny fil");
        }
    }
    else
    {
        Serial.println("Bruger eksisterende fil");
    }
}

void saveData()
{
    counter++;
    mellemLog = millis();

    aktuelSvejsning.tid = millis();
    // save data in instans of struct
    strncpy(aktuelSvejsning.id, workerID.c_str(), sizeof(aktuelSvejsning.id)); // workerID
    aktuelSvejsning.heatInput = heatInput;                                     // energy
    aktuelSvejsning.tid = mellemLog / 1000;

    svejsningData = SD.open("/dokumentation.csv", FILE_APPEND); // tid i sekunder                                      // time

    if (svejsningData)
    {

        svejsningData.print(counter);
        svejsningData.print(";");
        svejsningData.print(aktuelSvejsning.id);
        svejsningData.print(";");
        svejsningData.print(aktuelSvejsning.heatInput);
        svejsningData.print(";");
        svejsningData.println(aktuelSvejsning.tid);

        svejsningData.close();
        Serial.println("Data gemt!");
    }
    else
    {
        Serial.println("Ingen sd-kort fundet, data kan ikke gemmes.");
    }
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

            workerID = scannedUID;

            fundet = true;
        }
        yield();
        delay(50);
    }
    rc.PICC_HaltA();
    rc.PCD_StopCrypto1();

    Serial.print("Kort-UID fundet: ");
    Serial.println(workerID);
}

void setupSPI()
{
    WiFi.softAP(SSID, PASSWORD);
    IPAddress IP = WiFi.softAPIP();
    server.on("/", handleRoot);
    server.begin();
    Serial.print("Server er oppe på: ");
    Serial.println(IP);

    // Start SPI
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    // SD start
    if (!SD.begin(SD_CS, SPI))
    {
        Serial.println("SD CONNECTION FAILED");
    }
    else
    {
        Serial.println("SD CONNECTED SUCCES");
    }

    // RFID start
    rc.PCD_Init();
    delay(100);
    rc.PCD_DumpVersionToSerial(); // 0x12 for klon
}

#endif
