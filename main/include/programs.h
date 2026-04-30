#ifndef PROGRAMS_H
#define PROGRAMS_H

#include <Arduino.h>
#include <config.h>

const unsigned long svejseTime_1 = 50000; // test værdi, adjust l8r
const unsigned long svejseTime_2 = 60000;
const unsigned long svejseTime_3 = 70000;
const unsigned long svejseTime_4 = 80000;

int selectedProgram = 0;
unsigned long svejseStartTime = 0;
unsigned long svejseDuration = 0;
bool svejseAktiv = false;

const float CURRENT_A = 32.0; // test værdi, adjust l8r

extern float AVG_TEMP;

float MODSTAND_OHM = 0.3729f; // resistance of thicker wire

float getTargetEnergy()
{
return MODSTAND_OHM * 500.0f * (2100.0f - AVG_TEMP); // ask Jakob ...
}

unsigned long getSvejseTime()
{
  switch (selectedProgram)
  {
  case 1:
    return svejseTime_1;
  case 2:
    return svejseTime_2;
  case 3:
    return svejseTime_3;
  case 4:
    return svejseTime_4;
  default:
    return 0;
  }
}

const char *programName(int p)
{
  switch (p)
  {
  case 1:
    return "Program 1 <-10..+10C>"; // test værdier, adjust l8r
  case 2:
    return "Program 2 <+10..+20C>";
  case 3:
    return "Program 3 <+20..+30C>";
  case 4:
    return "Program 4 <+30..+40C>";
  default:
    return "Intet  valgt";
  }
}

bool confirmProgram()
{
  return (selectedProgram >= 1 && selectedProgram <= 4);
}

void cycleProgram()
{
  selectedProgram = (selectedProgram % 4) + 1;
}

void startSvejse()
{
  // svejseDuration = getSvejseTime();
  svejseDuration = getSvejseTime(); // test værdi, adjust l8r
  svejseStartTime = millis();
  svejseAktiv = true;
  Serial.print("startSvejse called. Duration: ");
  Serial.print(svejseDuration);
  Serial.print(" startTime: ");
  Serial.println(svejseStartTime);
  // Placeholder: Activate welding output pin here
  // add turn on svejsning
}

void stopSvejse()
{
  svejseAktiv = false;
  // TODO: deactivate svejsning / output pin here
}

// svejse state caller
bool svejseHandler()
{
  if (!svejseAktiv)
    return false;
  unsigned long elapsed = millis() - svejseStartTime;

  if (elapsed >= svejseDuration)
  {
    stopSvejse();
    return true; // svejsning completed
  }
  return false; // svejsning still active or not started
}

float calcLevereretEnergi()
{ // replace ohm with real value
  float t = svejseDuration / 1000.0f;
  return CURRENT_A * CURRENT_A * MODSTAND_OHM * t;
}

bool energiOk()
{
  return calcLevereretEnergi() >= getTargetEnergy();
}

#endif
