#ifndef RFID_H
#define RFID_H

#include <Arduino.h>
#include <MFRC522.h>

#define RFID_SDA 2
#define RFID_RST 1

MFRC522 rc;

rc.PCD_Init();

void searchUID();
void scanCard();

#endif