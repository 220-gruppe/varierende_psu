#ifndef PROGRAMS_H
#define PROGRAMS_H

#include <Arduino.h>

extern int selectedProgram = 0;
extern unsigned long svejseStartTime = 0;
extern unsigned long svejseDuration = 0;
extern bool svejseAktiv = false;
extern float AVG_TEMP;

const char *programName(int p);
bool confirmProgram();
void cycleProgram();
void startSvejse();
void stopSvejse();
bool svejseHandler();
float calcLevereretEnergi();
float getTargetEnergy()
bool energiOk();
unsigned long getSvejseTime();

#endif
