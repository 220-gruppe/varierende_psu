#ifndef CONFIG_H
#define CONFIG_H

#include <SD.h>
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <TFT_eSPI.h>

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

// I2C setup
#define I2C0_SDA 21  // Wire  → numpad
#define I2C0_SCL 16
#define I2C1_SDA 17 // Wire1 → temp sensor
#define I2C1_SCL 18

#endif