#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#define SD_MISO 11
#define SD_MOSI 13
#define SD_SCK 12
#define SD_CS 10

// SD library
File svejsningData;

// data
String workerID = "E789FA13";
float heatInput = 70000;
uint32_t mellemLog;
int counter = 0;

// data i en struct
struct SvejseLog
{
  char id[10];
  float heatInput;
  uint32_t tid;
};

// opret ny instans af sd og struct
SPIClass sdSPI(FSPI);
SvejseLog aktuelSvejsning;

void setup()
{
  Serial.begin(115200);
  delay(2000);
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  pinMode(3, INPUT_PULLUP);

  if (!SD.begin(SD_CS, sdSPI))
  {
    Serial.println("SD CONNECTION FAILED");
    return;
  }

  Serial.println("SD CONNECTION FOUND");

  if (!SD.exists("/dokumentation.txt")) // hvis der ikke findes en fil allerede, opret en
  {
    svejsningData = SD.open("/dokumentation.txt", FILE_WRITE); // opret fil
    if (svejsningData)                                         // skriv overskfit
    {
      svejsningData.println("NR. | Operatoer_ID | Heat_Input | Timestamp");
      svejsningData.close();
      Serial.println("MAKING NEW FILE");
    }
  }else{
    Serial.println("USING EXISTING FILE");
  }
}

void saveData()
{
  counter++;
  mellemLog = millis();

  aktuelSvejsning.tid = millis();
  // save data in instans of struct
  strncpy(aktuelSvejsning.id, workerID.c_str(), sizeof(aktuelSvejsning.id)); // workerID
  aktuelSvejsning.heatInput = heatInput;                                     // energy
  aktuelSvejsning.tid = mellemLog / 1000;

  svejsningData = SD.open("/dokumentation.txt", FILE_APPEND); // tid i sekunder                                      // time

  if (svejsningData)
  {

    svejsningData.print(counter);
    svejsningData.print(" | ");
    svejsningData.print(aktuelSvejsning.id);
    svejsningData.print(" | ");
    svejsningData.print(aktuelSvejsning.heatInput);
    svejsningData.print(" | ");
    svejsningData.println(aktuelSvejsning.tid);

    svejsningData.close();
    Serial.println("NEW ROW ADDED");
  }
  else
  {
    Serial.println("NO SD-CARD FOUND");
  }
}
void loop()
{
  if (digitalRead(3) == LOW)
  {
    saveData();
    delay(1000);
  }
}