#ifndef RFID_H
#define RFID_H

#include <Arduino.h>
#include <MFRC522.h>

#define RFID_SDA 16
#define RFID_RST 21

void setupRFID();
String scanUID();

#endif
