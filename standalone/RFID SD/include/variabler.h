#ifndef VARIABLER_H
#define VARIABLER_H

#include <Arduino.h>
#include <MFRC522.h>
#include <SD.h>
#include <config.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>
#include <TFT_eSPI.h>

Adafruit_MPR121 numpad = Adafruit_MPR121();
TFT_eSPI tft = TFT_eSPI();
MFRC522 rc(RFID_SDA, RFID_RST); // ny instans af scanner

// struct
struct SvejseLog
{
  char id[10];
  float heatInput;
  uint32_t tid;
};

// display farver og andre konstanter
#define SPIDER_BG 0xE6D6   // BAGGRUND
#define SPIDER_BLUE 0x201F // BLÅ TEKST
#define TIMER 30000 //30 sekunder MINUTTER


// extern variabler så de kan bruges globalt
extern String scannedUID;
extern MFRC522 rc;
extern String workerID;
extern float heatInput;
extern uint32_t mellemLog;
extern int counter;
extern SvejseLog aktuelSvejsning;
extern File svejsningData;
extern File logins;
extern bool waitforChip;
extern String tempNavn;
extern String tempPin;
extern String tempUID;

extern bool manglerPin;
extern bool isLoggedIn;
extern String indtastet;
extern bool ikkeKodet;
extern String nuStatus;
extern String tastet;
extern String korrektPin;
extern String sidsteStatus;

extern unsigned long tidStart;

#endif