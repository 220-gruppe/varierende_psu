#ifndef RFID_H
#define RFID_H

#include <Arduino.h>
#include <MFRC522.h>

#define RFID_SPI_MISO 11
#define RFID_SPI_MOSI 13
#define RFID_SPI_SCK 12
#define RFID_SDA 43
#define RFID_RST 21

void setupRFID();
String scanUID();

#endif
