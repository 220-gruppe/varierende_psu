#ifndef DATABASE_H
#define DATABASE_H

#include <SPI.h>
#include <SD.h>

#define SD_CS 10
void setupDatabase();
bool DB(const String &db, const String &columns = "");
bool databaseWrite(const String &data);
bool databaseSearch(const String &query, String &result);
void databaseRead();
#endif
