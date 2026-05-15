#include "database.h"

namespace
{
bool sdReady = false;
constexpr uint8_t SD_CS = 10;
constexpr uint8_t SPI_MISO_PIN = 11;
constexpr uint8_t SPI_MOSI_PIN = 13;
constexpr uint8_t SPI_SCK_PIN = 12;
}

namespace
{
String csvPath(const char *path)
{
    String normalized = path;

    if (!normalized.startsWith("/"))
    {
        normalized = "/" + normalized;
    }

    if (!normalized.endsWith(".csv"))
    {
        normalized += ".csv";
    }

    return normalized;
}

bool openCsvFile(const char *path, const char *mode, File &file, const char *action)
{
    String normalized = csvPath(path);
    file = SD.open(normalized, mode);

    if (file)
    {
        return true;
    }

    Serial.print("Kunne ikke aabne CSV til ");
    Serial.print(action);
    Serial.print(": ");
    Serial.println(normalized);
    return false;
}
}

void setupDatabase()
{
    pinMode(SD_CS, OUTPUT);
    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);
    sdReady = false;

    for (int attempt = 1; attempt <= SD_MOUNT_ATTEMPTS; attempt++)
    {
        if (SD.begin(SD_CS, SPI))
        {
            sdReady = true;
            Serial.println("SD CONNECTED SUCCES");
            return;
        }

        Serial.print("SD CONNECTION FAILED, forsog ");
        Serial.print(attempt);
        Serial.print("/");
        Serial.println(SD_MOUNT_ATTEMPTS);
        delay(SD_MOUNT_RETRY_DELAY_MS);
    }

    Serial.println("SD IKKE MONTERET - CSV logging er deaktiveret");
}

bool isSdReady()
{
    return sdReady;
}

bool ensureCsvFile(const char *path, const char *header)
{
    if (!isSdReady())
    {
        Serial.print("SD er ikke monteret, kan ikke oprette CSV: ");
        Serial.println(csvPath(path));
        return false;
    }

    String normalized = csvPath(path);

    if (SD.exists(normalized))
    {
        return true;
    }

    File file = SD.open(normalized, FILE_WRITE, true);
    if (!file)
    {
        Serial.print("Kunne ikke oprette CSV: ");
        Serial.println(normalized);
        return false;
    }

    if (header != nullptr && strlen(header) > 0)
    {
        file.println(header);
    }

    file.close();
    return true;
}

bool appendLineToFile(const char *path, const String &line)
{
    if (!isSdReady())
    {
        Serial.print("SD er ikke monteret, kan ikke skrive CSV: ");
        Serial.println(csvPath(path));
        return false;
    }

    File file;
    if (!openCsvFile(path, FILE_APPEND, file, "skrivning"))
    {
        return false;
    }

    file.println(line);
    file.close();
    return true;
}

bool findLineByFirstCsvField(const char *path, const String &value, String &result)
{
    if (!isSdReady())
    {
        Serial.print("SD er ikke monteret, kan ikke laese CSV: ");
        Serial.println(csvPath(path));
        result = "";
        return false;
    }

    File file;
    if (!openCsvFile(path, FILE_READ, file, "laesning"))
    {
        result = "";
        return false;
    }

    result = "";

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();

        if (line.length() == 0)
        {
            continue;
        }

        int firstComma = line.indexOf(',');
        String firstField = firstComma == -1 ? line : line.substring(0, firstComma);
        firstField.trim();

        if (firstField == value)
        {
            result = line;
            file.close();
            return true;
        }
    }

    file.close();
    return false;
}

void printFileContents(const char *path)
{
    if (!isSdReady())
    {
        Serial.print("SD er ikke monteret, kan ikke printe CSV: ");
        Serial.println(csvPath(path));
        return;
    }

    File file;
    if (!openCsvFile(path, FILE_READ, file, "print"))
    {
        return;
    }

    Serial.print("CSV indhold: ");
    Serial.println(csvPath(path));

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();
        Serial.println(line);
    }

    file.close();
}
