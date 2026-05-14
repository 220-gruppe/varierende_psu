#include "svejse_logs.h"
#include "programs.h"
#include "tempsensor.h"
#include "pwm.h"
#include "auth.h"

int selectedProgram = 0;
unsigned long svejseStartTime = 0;
unsigned long svejseDuration = 0;
unsigned long svejseElapsedTime = 0;
bool svejseAktiv = false;

namespace
{
const WeldProgram *selectedWeldProgram()
{
    if (!confirmProgram())
    {
        return nullptr;
    }

    return &WELD_PROGRAMS[selectedProgram - 1];
}
}

unsigned long getSvejseTime()
{
    const WeldProgram *program = selectedWeldProgram();
    return program == nullptr ? 0 : program->durationMs;
}

float getSvejseTargetCurrentMA()
{
    const WeldProgram *program = selectedWeldProgram();
    return program == nullptr ? 0.0f : program->targetCurrentMA;
}

unsigned long getSvejseElapsedTime()
{
    if (svejseAktiv)
    {
        return millis() - svejseStartTime;
    }

    return svejseElapsedTime;
}

const char *programName(int p)
{
    if (p < 1 || p > WELD_PROGRAM_COUNT)
    {
        return "Intet  valgt";
    }

    return WELD_PROGRAMS[p - 1].name;
}

bool confirmProgram()
{
    return selectedProgram >= 1 && selectedProgram <= WELD_PROGRAM_COUNT;
}

void cycleProgram()
{
    selectedProgram = (selectedProgram % WELD_PROGRAM_COUNT) + 1;
}

void startSvejse()
{
    svejseDuration = getSvejseTime(); 
    float targetCurrentMA = getSvejseTargetCurrentMA();

    if (svejseDuration == 0 || targetCurrentMA <= 0.0f)
    {
        Serial.println("Ugyldigt svejseprogram - mangler tid eller target mA");
        svejseAktiv = false;
        disableSvejsning();
        return;
    }

    svejseStartTime = millis();
    svejseElapsedTime = 0;
    resetEnergy();
    resetWeldFault();
    resetPwmControl();

    if (!initializeSvejsningLog(currentUserUID(), selectedProgram, svejseStartTime, targetCurrentMA, svejseDuration))
    {
        Serial.println("Kunne ikke oprette svejselog - svejsning startes ikke");
        svejseAktiv = false;
        return;
    }

    svejseAktiv = true;
    enableSvejsning();
    Serial.print("startSvejse called. Duration: ");
    Serial.print(svejseDuration);
    Serial.print(" target mA: ");
    Serial.print(targetCurrentMA, 0);
    Serial.print(" startTime: ");
    Serial.println(svejseStartTime);
}

void stopSvejse()
{
    if (svejseAktiv)
    {
        svejseElapsedTime = millis() - svejseStartTime;
    }

    svejseAktiv = false;
    disableSvejsning();
}

bool svejseHandler()
{
    if (!svejseAktiv)
        return false;
    unsigned long elapsed = getSvejseElapsedTime();

    if (hasWeldFault() || elapsed >= svejseDuration)
    {
        stopSvejse();
        return true; 
    }
    return false; 
}
