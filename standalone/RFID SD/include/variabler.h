#ifndef VARIABLER_H
#define VARIABLER_H

#include <Arduino.h>
#include <MFRC522.h>
#include <SD.h>
#include <config.h>

TFT_eSPI tft = TFT_eSPI();

// struct
struct SvejseLog
{
  char id[10];
  float heatInput;
  uint32_t tid;
};
//display farver osv
#define SPIDER_BG 0xE6D6 //BAGGRUND
#define SPIDER_BLUE 0x201F //BLÅ TEKST 


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
extern String korrektPin;
extern bool manglerPin;
extern bool isLoggedIn;

#endif