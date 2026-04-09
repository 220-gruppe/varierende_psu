// egne libraries
#include <logo.h>
#include <variabler.h>
#include <config.h>
#include <functions.h>
#include <server.h>

WebServer server(80);

String scannedUID = "";
MFRC522 rc(RFID_SDA, RFID_RST); // ny instans af scanner
String workerID = "E789FA13";
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
bool isLoggedIn = false;

void setup()
{
  Serial.begin(115200);
  delay(2000);

  // setup og tjek for allerede oprettet filer
  setupSPI();
  createFile();
}

String sidsteStatus = "";
bool ikkeKodet = false;

void loop()
{
  server.handleClient();

  String nuStatus = "";
  if (isLoggedIn)
    nuStatus = "VELKOMMEN " + workerID;
  else if (manglerPin)
    nuStatus = "INDTAST PIN";
  else if (waitforChip)
    nuStatus = "SCAN NY CHIP...";
  else if (ikkeKodet)
  {
    nuStatus = "CHIP FINDES IKKE!";
  }
  else
    nuStatus = "KLAR TIL SCAN";

  if (nuStatus != sidsteStatus)
  {
    tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
    tft.setTextDatum(MC_DATUM);

    tft.fillRect(0, 140, 320, 20, SPIDER_BG);

    tft.drawString(nuStatus, 160, 155, 1);
    sidsteStatus = nuStatus;

    if (nuStatus == "CHIP FINDES IKKE!")
    {
      delay(3000);
      ikkeKodet = false;
      waitforChip = true;
      // Vi behøver ikke nulstille sidsteStatus,
      // loopet finder selv ud af at nuStatus er ændret næste gang.
    }
  }

  if (!isLoggedIn)
  {
    if (rc.PICC_IsNewCardPresent() && rc.PICC_ReadCardSerial())
    {
      String fundetUID = "";
      for (byte i = 0; i < rc.uid.size; i++)
      {
        fundetUID += (rc.uid.uidByte[i] < 0x10 ? "0" : "");
        fundetUID += String(rc.uid.uidByte[i], HEX);
      }
      fundetUID.toUpperCase();

      if (waitforChip)
      {
        tempUID = fundetUID;
        opretLogin(); // Gemmer i logins.csv og sætter waitforChip = false
      }
      else
      {
        if (tjekLogin(fundetUID))
        {
          manglerPin = true;
          ikkeKodet = false;
          Serial.println("Chip fundet. Venter på kode...");
        }
        else
        {
          Serial.println("Chip findes ikke...");
          ikkeKodet = true;
        }
      }
      rc.PICC_HaltA();
      rc.PCD_StopCrypto1();
    }
  }
}