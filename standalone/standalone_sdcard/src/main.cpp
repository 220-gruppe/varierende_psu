#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#define SD_MISO 11
#define SD_MOSI 13
#define SD_SCK 12
#define SD_CS 10

// data
File svejsningData;
String workerID = "E789FA13";
float heatInput = 70000;

struct SvejseLog
{
  char id[10];
  float heatInput;
  uint32_t tid;
};

// opret ny instans
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
    Serial.println("SD FORBINDELSE FEJLET");
    return;
  }

  Serial.println("SD FORBINDELSE FORBUNDET");
}

void saveData()
{
  // save data in instans of struct
  strncpy(aktuelSvejsning.id, workerID.c_str(), sizeof(aktuelSvejsning.id)); // workerID
  aktuelSvejsning.heatInput = heatInput;                                     // energy
  aktuelSvejsning.tid = millis();                                            // time
  

  svejsningData = SD.open("/dokumentation.txt", FILE_WRITE);

  if (svejsningData)
  {
    Serial.println("GEMMER DATA TIL SD");
    svejsningData.print("Operatør ID: ");
    svejsningData.print(aktuelSvejsning.id);
    svejsningData.print(", heatInput: ");
    svejsningData.print(aktuelSvejsning.heatInput);
    svejsningData.print(", Tidspunkt: ");
    svejsningData.println(aktuelSvejsning.tid);
    Serial.println("DATA GEMT");
    svejsningData.close();
  }
  else
  {
    Serial.println("Kunne ikke skrive til SD-kort");
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