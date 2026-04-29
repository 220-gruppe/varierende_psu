#ifndef DATABASE_H
#define DATABASE_H

#include <SPI.h>
#include <SD.h>

#define SD_CS 10
void setupDatabase();
bool DB(const String &db, const String &columns = "");
bool write(const String &data);
bool search(const String &query, String &result);
//void saveData();
//void searchUID();
//void opretLogin();
//bool tjekLogin(String fundetUID);

#endif
