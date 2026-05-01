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

bool openCurrentDb(const char *mode, File &dbFile, const char *action)
{
    if (currentDB.length() == 0)
    {
        Serial.println("DB is not selected");
        return false;
    }

    dbFile = SD.open(currentDB, mode);
    if (!dbFile)
    {
        Serial.print("Failed to open database for ");
        Serial.print(action);
        Serial.print(": ");
        Serial.println(currentDB);
        return false;
    }

    return true;
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

bool databaseWrite(const String &data)
{
    File dbFile;
    if (!openCurrentDb(FILE_APPEND, dbFile, "write"))
    {
        return false;
    }

    dbFile.println(data);
    dbFile.close();
    return true;
}

bool databaseSearch(const String &query, String &result)
{
    File dbFile;
    if (!openCurrentDb(FILE_READ, dbFile, "read"))
    {
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

void databaseRead()
{
    File dbFile;
    if (!openCurrentDb(FILE_READ, dbFile, "read"))
    {
        return;
    }

    Serial.print("Database content from ");
    Serial.println(currentDB);

    while (dbFile.available())
    {
        String line = dbFile.readStringUntil('\n');
        line.trim();
        Serial.println(line);
    }

    dbFile.close();
}
