// egne libraries
#include <logo.h>
#include <variabler.h>
#include <config.h>
#include <functions.h>
#include <server.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

String scannedUID = "";
String workerID = "";
float heatInput = 70000;   // ændres til noget fra sensor
uint32_t mellemLog = 0;    // ændres til reelt tidspunkt
int counter = 0;           // blot en counter til antal
SvejseLog aktuelSvejsning; // instans af struct
File svejsningData;        // instans af sdkort
File logins;               // instans af sdkort

// variabler til opret nyt login
String tempNavn = "";
String tempPin = "";
String tempUID = "";
bool waitforChip = false;

bool manglerPin = false;
bool isLoggedIn = false;
String indtastet = "";
bool ikkeKodet = false;
String nuStatus = "";
String tastet = "";
String korrektPin = "";
String sidsteStatus = "";

unsigned long tidStart = 0;

void setup()
{
  Serial.begin(115200);

  // setup og tjek for allerede oprettet filer
  setupSPI();
  createFile();
}

void loop()
{
  server.handleClient();

  opdaterScreen(); // OPDATER SKÆRM
  kortScan();      // TJEKKER FOR KORT

  if (manglerPin) // MANGLER PIN, TJEK KODE
  {
    numpadLogik();
  }

  if (isLoggedIn)
  {
    inaktivitetTjek(); // >15 MIN, OK?
  }
}
