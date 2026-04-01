// egne libraries
#include <server.h>
#include <config.h>
#include <functions.h>
#include <variabler.h>

String scannedUID = "";
MFRC522 rc(RFID_SDA, RFID_RST); // ny instans af scanner
String workerID = "E789FA13";
float heatInput = 70000;   // ændres til noget fra sensor
uint32_t mellemLog = 0;    // ændres til reelt tidspunkt
int counter = 0;           // blot en counter til antal
SvejseLog aktuelSvejsning; // instans af struct
File svejsningData;        // instans af sdkort

void setup()
{
  Serial.begin(115200);
  delay(2000);

  setupSPI();
  createFile();

  pinMode(SD_CS, OUTPUT);
  pinMode(RFID_SDA, OUTPUT);
  pinMode(3, INPUT_PULLUP);
}

void loop()
{
  server.handleClient();
  if (digitalRead(3) == LOW)
  {
    Serial.println("Søger efter adgangskort: ");
    searchUID(); // finder UID
    saveData();
    delay(1000);
  }
}