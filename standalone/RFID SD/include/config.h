#ifndef CONFIG_H
#define CONFIG_H

#include <SD.h>
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

// server settings
const char *SSID = "Spider-feet";
const char *PASSWORD = "";

// SPI Pins
#define SPI_MISO 11
#define SPI_MOSI 13
#define SPI_SCK 12

// RFID Pins
#define RFID_SDA 2
#define RFID_RST 1

// SD og Knap Pins
#define SD_CS 10
#define BUTTON_PIN 3

#endif