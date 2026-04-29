#include "database.h"

String currentDB = "";

namespace
{
String buildDbPath(const String &db)
{
    String path = db;

    if (!path.startsWith("/"))
    {
        path = "/" + path;
    }

    if (!path.endsWith(".csv"))
    {
        path += ".csv";
    }

    return path;
}
}

void setupDatabase(){
    pinMode(SD_CS, OUTPUT);
    if (!SD.begin(SD_CS, SPI))
    {
        Serial.println("SD CONNECTION FAILED");
    }
    else
    {
        Serial.println("SD CONNECTED SUCCES");
    }
}

bool DB(const String &db, const String &columns)
{
    String path = buildDbPath(db);
    bool exists = SD.exists(path);

    currentDB = path;

    if (currentDB.length() == 0)
    {
        Serial.println("Failed to select database path");
        return false;
    }

    if (exists)
    {
        return true;
    }

    File dbFile = SD.open(path, FILE_WRITE, true);
    if (!dbFile)
    {
        Serial.print("Failed to create database: ");
        Serial.println(path);
        return false;
    }

    if (columns.length() > 0)
    {
        dbFile.println(columns);
    }

    dbFile.close();
    return true;
}

bool write(const String &data)
{
    if (currentDB.length() == 0)
    {
        Serial.println("DB is not selected");
        return false;
    }

    File dbFile = SD.open(currentDB, FILE_APPEND);
    if (!dbFile)
    {
        Serial.print("Failed to open database for write: ");
        Serial.println(currentDB);
        return false;
    }

    dbFile.println(data);
    dbFile.close();
    return true;
}

bool databaseSearch(const String &query, String &result)
{
    if (currentDB.length() == 0)
    {
        Serial.println("DB is not selected");
        result = "";
        return false;
    }

    File dbFile = SD.open(currentDB, FILE_READ);
    if (!dbFile)
    {
        Serial.print("Failed to open database for read: ");
        Serial.println(currentDB);
        result = "";
        return false;
    }

    result = "";

    while (dbFile.available())
    {
        String line = dbFile.readStringUntil('\n');
        line.trim();

        if (line.indexOf(query) != -1)
        {
            result = line;
            dbFile.close();
            return true;
        }
    }

    dbFile.close();
    return false;
}

//void saveData()
//{
//    counter++;
//    mellemLog = millis();
//
//    // save data in instans of struct
//    strncpy(aktuelSvejsning.id, workerID.c_str(), sizeof(aktuelSvejsning.id)); // workerID
//    aktuelSvejsning.heatInput = heatInput;                                     // energy
//    aktuelSvejsning.tid = mellemLog / 1000;
//
//    svejsningData = SD.open("/dokumentation.csv", FILE_APPEND);
//
//    if (svejsningData)
//    {
//        svejsningData.print(counter);
//        svejsningData.print(";");
//        svejsningData.print(aktuelSvejsning.id);
//        svejsningData.print(";");
//        svejsningData.print(aktuelSvejsning.heatInput);
//        svejsningData.print(";");
//        svejsningData.println(aktuelSvejsning.tid);
//
//        svejsningData.close();
//        Serial.println("Data gemt!");
//    }
//    else
//    {
//        Serial.println("Ingen sd-kort fundet, data kan ikke gemmes.");
//    }
//}

//void searchUID()
//{
//    bool fundet = false;
//
//    while (!fundet)
//    {
//        if (rc.PICC_IsNewCardPresent() && rc.PICC_ReadCardSerial())
//        {
//            scannedUID = "";
//
//            for (byte i = 0; i < rc.uid.size; i++)
//            {
//                scannedUID += (rc.uid.uidByte[i] < 0x10 ? "0" : "");
//                scannedUID += String(rc.uid.uidByte[i], HEX);
//            }
//            scannedUID.toUpperCase();
//
//            workerID = scannedUID;
//
//            fundet = true;
//        }
//        yield();
//        delay(50);
//    }
//    rc.PICC_HaltA();
//    rc.PCD_StopCrypto1();
//
//    Serial.print("Kort-UID fundet: ");
//    Serial.println(workerID);
//}

//void opretLogin()
//{
//    Users = SD.open("/users.csv", FILE_APPEND);
//
//    if (!users) {
//        Serial.println("faild to open file");
//    }
//        users.print(tempUID);
//        users.print(";");
//        users.print(tempNavn);
//        users.print(";");
//        users.println(tempPin);
//
//        users.close();
//        Serial.print("Nyt login oprettet til: ");
//        Serial.print(tempNavn);
//        Serial.print(", ");
//        Serial.print(tempUID);
//        Serial.print(", ");
//        Serial.println(tempPin);
//   
//   //   waitforChip = false;
//}
//
//bool tjekLogin(String fundetUID)
//{
//    File loginFil = SD.open("/logins.csv", FILE_READ);
//    if (!loginFil)
//    {
//        Serial.print("Kunne ikke åbne logins.csv...");
//        return false;
//    }
//
//    while (loginFil.available())
//    {
//        String linje = loginFil.readStringUntil('\n');
//        linje.trim();
//
//        int fsemi = linje.indexOf(';');
//        int asemi = linje.indexOf(';', fsemi + 1);
//        if (fsemi != -1)
//        {
//            String filUID = linje.substring(0, fsemi);
//
//            if (filUID == fundetUID)
//            {
//                int asemi = linje.indexOf(';', fsemi + 1);
//
//                if (asemi != -1)
//                {
//                    workerID = linje.substring(fsemi + 1, asemi);
//                    korrektPin = linje.substring(asemi + 1);
//
//                    loginFil.close();
//
//                    return true;
//                }
//            }
//        }
//    }
//    loginFil.close();
//    return false;
//}
