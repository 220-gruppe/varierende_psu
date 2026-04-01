#ifndef VARIABLER_H
#define VARIABLER_H

#include <Arduino.h>
#include <MFRC522.h>
#include <SD.h>
#include <config.h>

// Struct definitionen må gerne stå her (da det er en opskrift, ikke en variabel)
struct SvejseLog {
  char id[10];
  float heatInput;
  uint32_t tid;
};

// EXTERN fortæller andre filer at disse variabler findes
extern String scannedUID;
extern MFRC522 rc;
extern String workerID;
extern float heatInput;
extern uint32_t mellemLog;
extern int counter;
extern SvejseLog aktuelSvejsning;
extern File svejsningData;

#endif