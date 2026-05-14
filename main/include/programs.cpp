#include "programs.h"
#include "tempsensor.h"
#include "pwm.h"

const unsigned long svejseTime_1 = 50000; // test værdi, adjust l8r
const unsigned long svejseTime_2 = 60000;
const unsigned long svejseTime_3 = 70000;
const unsigned long svejseTime_4 = 183000;

const float targetJoule_1 = 500.0f;
const float targetJoule_2 = 1000.0f;
const float targetJoule_3 = 1500.0f;
const float targetJoule_4 = 2000.0f;

int selectedProgram = 0;
unsigned long svejseStartTime = 0;
unsigned long svejseDuration = 0;
bool svejseAktiv = false;

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

float getTargetJoule()
{
    switch (selectedProgram)
    {
    case 1:
        return targetJoule_1;
    case 2:
        return targetJoule_2;
    case 3:
        return targetJoule_3;
    case 4:
        return targetJoule_4;
    default:
        return 0.0f;
    }
}

const char *programName(int p)
{
    switch (p)
    {
    case 1:
        return "Program 1 "; 
    case 2:
        return "Program 2 ";
    case 3:
        return "Program 3 ";
    case 4:
        return "Program 4 ";
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
    svejseDuration = getSvejseTime(); 
    svejseStartTime = millis();
    svejseAktiv = true;
    resetEnergy();
    enableSvejsning();
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
    disableSvejsning();
    // TODO: deactivate svejsning / output pin here
}

bool svejseHandler()
{
    if (!svejseAktiv)
        return false;
    unsigned long elapsed = millis() - svejseStartTime;

    if (hasReachedTarget() || elapsed >= svejseDuration)
    {
        stopSvejse();
        return true; 
    }
    return false; 
}


