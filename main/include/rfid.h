#ifndef RFID_H
#define RFID_H

#include <Arduino.h>
#include <MFRC522.h>

void setupRFID();
String scanUID();

#endif
