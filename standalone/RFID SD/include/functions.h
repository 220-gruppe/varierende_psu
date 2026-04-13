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
    server.on("/reset", handleResetFile);                      // reset logins for test
    server.on("/logout", handleLogout);                        // log ud
    server.begin();
    Serial.print("Server er oppe på: ");
    Serial.println(IP);

    // intitialiser pinmodes
    pinMode(SD_CS, OUTPUT);
    pinMode(RFID_SDA, OUTPUT);
    pinMode(3, INPUT_PULLUP);

    // SKÆRM
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);

    // LILYGO DISPLAY
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(0xE6D6);
    tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
    tft.setTextSize(3);
    tft.setSwapBytes(true);
    tft.pushImage(35, 10, 250, 77, logo);

    // Start SPI
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    // START I2C SDA, SCL
    Wire.begin(21, 16);

    // NUMPAD
    if (!numpad.begin(0x5A, &Wire))
    {
        Serial.println("STOP! MPR121 ikke fundet.");
        while (1)
            delay(10);
    }
    Serial.println("Den er fundet makker");
    numpad.setAutoconfig(true);

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

void numpadLogik()
{
    uint16_t talTrykket = numpad.touched();
    if (talTrykket > 0)
    {
        int aktivtBit = -1;
        for (uint8_t i = 0; i < 12; i++)
        {
            if (talTrykket & _BV(i))
            {
                aktivtBit = i;
                break;
            }
        }

        if (aktivtBit != -1)
        {
            String tal = "";
            bool slet = false;
            bool enter = false;

            switch (aktivtBit)
            {
            case 7:
                tal = "0";
                break;
            case 0:
                tal = "1";
                break;
            case 4:
                tal = "2";
                break;
            case 8:
                tal = "3";
                break;
            case 1:
                tal = "4";
                break;
            case 5:
                tal = "5";
                break;
            case 9:
                tal = "6";
                break;
            case 2:
                tal = "7";
                break;
            case 6:
                tal = "8";
                break;
            case 10:
                tal = "9";
                break;
            case 3:
                slet = true;
                break; // SLET
            case 11:
                enter = true;
                break; // ENTER
            }

            if (tal != "")
            {
                indtastet += tal;
                tastet += "*";
                Serial.print(tal + " ");
            }

            if (slet)
            {
                indtastet = "";
                tastet = "";
                tft.fillRect(0, 120, 320, 50, SPIDER_BG);
                Serial.println("SLET");
            }

            if (indtastet.length() > 4 && indtastet != korrektPin)
            {
                tft.fillRect(0, 120, 320, 50, SPIDER_BG);
                indtastet = "";
                tastet = "";
            }

            if (enter)
            {
                Serial.println("#");
                if (indtastet == korrektPin)
                {
                    isLoggedIn = true;
                    manglerPin = false;
                    indtastet = "";
                    tastet = "";
                    Serial.println("ADGANG GODKENDT");
                }
                else
                {
                    Serial.println("FORKERT KODE!");
                    tft.fillRect(0, 120, 320, 50, SPIDER_BG);
                    tft.drawString("FORKERT KODE!", 160, 135, 1);
                    delay(1000);
                    indtastet = "";
                    tastet = "";
                    tft.fillRect(0, 120, 320, 50, SPIDER_BG);
                }
            }

            if (manglerPin && !slet)
            {
                tft.setTextColor(TFT_RED, SPIDER_BG);
                tft.setTextDatum(MC_DATUM);
                tft.drawString(tastet, 160, 135, 1);
            }
            delay(250);
        }
    }
}



void opdaterScreen()
{
    if (isLoggedIn)
        nuStatus = "VELKOMMEN " + workerID;
    else if (manglerPin)
        nuStatus = "INDTAST PIN:";
    else if (waitforChip)
        nuStatus = "SCAN NY CHIP...";
    else if (ikkeKodet)
    {
        nuStatus = "CHIP FINDES IKKE!";
    }
    else
        nuStatus = "KLAR TIL SCAN";

    if (nuStatus != sidsteStatus)
    {

        tft.fillRect(0, 97, 320, 222, SPIDER_BG);

        if (nuStatus == "CHIP FINDES IKKE!")
        {
            tft.setTextColor(TFT_RED, SPIDER_BG);
            tft.setTextDatum(MC_DATUM);
            tft.drawString(nuStatus, 160, 110, 1);
            delay(2000);
            ikkeKodet = false;
        }
        else
        {
            tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
            tft.setTextDatum(MC_DATUM);
            tft.drawString(nuStatus, 160, 110, 1);
        }
        sidsteStatus = nuStatus;
    }
}

void kortScan()
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

            if (waitforChip)
            {
                tempUID = fundetUID;
                opretLogin(); // Gemmer i logins.csv og sætter waitforChip = false
            }
            else
            {
                if (tjekLogin(fundetUID)) // matcher fundet UID med data
                {
                    manglerPin = true;
                    Serial.println("Chip fundet. Venter på kode...");
                }
                else
                {
                    ikkeKodet = true;
                    Serial.println("CHIP FINDES IKKE!");
                }
            }
            rc.PICC_HaltA();
            rc.PCD_StopCrypto1();
        }
    }
}

#endif