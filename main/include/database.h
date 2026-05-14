#ifndef DATABASE_H
#define DATABASE_H

#include <SPI.h>
#include <SD.h>

extern const int SD_MOUNT_ATTEMPTS;
extern const unsigned long SD_MOUNT_RETRY_DELAY_MS;

void setupDatabase();
bool isSdReady();
bool ensureCsvFile(const char *path, const char *header = "");
bool appendLineToFile(const char *path, const String &line);
bool findLineByFirstCsvField(const char *path, const String &value, String &result);
void printFileContents(const char *path);

#endif
