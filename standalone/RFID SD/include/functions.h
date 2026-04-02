#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <config.h>
#include <variabler.h>
#include <server.h>

void createFile()
{
    if (!SD.exists("/logins.csv"))
    {
        logins = SD.open("/logins.csv", FILE_WRITE); // opret fil
        if (logins)                                  // skriv overskfit
        {
            logins.print("UID");
            logins.print(";");
            logins.print("NAVN");
            logins.print(";");
            logins.println("PIN");

            logins.close();
        }
        Serial.println("Oprettet Login.csv");
    }

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
        }
        Serial.println("Oprettet dokumentaions.csv");
    }
    else
    {
        Serial.println("Bruger eksisterende logins.csv og dokumentaions.csv");
    }
}

void saveData()
{
    counter++;
    mellemLog = millis();

    // save data in instans of struct
    strncpy(aktuelSvejsning.id, workerID.c_str(), sizeof(aktuelSvejsning.id)); // workerID
    aktuelSvejsning.heatInput = heatInput;                                     // energy
    aktuelSvejsning.tid = mellemLog / 1000;

    svejsningData = SD.open("/dokumentation.csv", FILE_APPEND);

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
    WiFi.softAP(SSID, PASSWORD);                               // hotspot kode
    IPAddress IP = WiFi.softAPIP();                            // initialiser hotspot
    server.on("/gemLogin", HTTP_POST, handleGemLogin);         // gemLogin funktion
    server.on("/", handleRoot);                                //
    server.on("/verificerPin", HTTP_POST, handleVerificerPin); // tjek kode funktion
    server.on("/opret", handleOpretSide);                      // opret nyt login funktion
    server.on("/checkStatus", handleCheckStatus);              // sørger for at opdatere siden på telefonen
    server.on("/logout", handleLogout);                        // log ud
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

    rc.PCD_Init();
}

void opretLogin()
{
    logins = SD.open("/logins.csv", FILE_APPEND);

    if (logins)
    {
        logins.print(tempUID);
        logins.print(";");
        logins.print(tempNavn);
        logins.print(";");
        logins.println(tempPin);

        logins.close();
        Serial.print("Nyt login oprettet til: ");
        Serial.print(tempNavn);
        Serial.print(", ");
        Serial.print(tempUID);
        Serial.print(", ");
        Serial.println(tempPin);
    }
    waitforChip = false;
}

bool tjekLogin(String fundetUID)
{
    File loginFil = SD.open("/logins.csv", FILE_READ);
    if (!loginFil)
    {
        Serial.print("Kunne ikke åbne logins.csv...");
        return false;
    }

    while (loginFil.available())
    {
        String linje = loginFil.readStringUntil('\n');
        linje.trim();

        int fsemi = linje.indexOf(';');
        int asemi = linje.indexOf(';', fsemi + 1);
        if (fsemi != -1)
        {
            String filUID = linje.substring(0, fsemi);

            if (filUID == fundetUID)
            {
                int asemi = linje.indexOf(';', fsemi + 1);

                if (asemi != -1)
                {
                    workerID = linje.substring(fsemi + 1, asemi);
                    korrektPin = linje.substring(asemi + 1);

                    loginFil.close();
                    
                    return true;
                }
            }
        }
    }
    loginFil.close();
    return false;
}

#endif