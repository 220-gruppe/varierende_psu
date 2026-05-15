#include "svejse_logs.h"
#include "programs.h"
#include "tempsensor.h"
#include "pwm.h"
#include "auth.h"

int selectedProgram             = 0;
unsigned long svejseStartTime   = 0;
unsigned long svejseDuration    = 0;
unsigned long svejseElapsedTime = 0;
bool svejseAktiv                = false;

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

// const char *programName(int p)
// {
//     if (p < 1 || p > NUM_PROGRAMS) return "Intet valgt";
//     return PROGRAMS[p-1].name;
// }

bool confirmProgram()
{
    return selectedProgram >= 1 && selectedProgram <= WELD_PROGRAM_COUNT;
}

// bool confirmProgram()
// {
//     return (selectedProgram >= 1 && selectedProgram <= 4);
// }

void cycleProgram()
{
    selectedProgram = (selectedProgram % NUM_PROGRAMS) + 1;
}

// void cycleProgram()
// {
//     selectedProgram = (selectedProgram % 4) + 1;
// }

// float getTargetEnergy()
// {
//     const Program& p = currentProgram();
//     return p.mass_kg * 500.0f * (p.targetTemp_C - AVG_TEMP);
// }

float getTargetEnergy()
{
    return getTargetJoule();
}

// float getSvejseProgress()
// {
//     float target = getTargetEnergy();
//     if (target <= 0.0f) return 1.0f;
//     return constrain(getDeliveredEnergyJ() / target, 0.0f, 1.0f);
// }

float getSvejseProgress()
{
    float target = getTargetJoule();
    if (target <= 0.0f)
        return 1.0f;
    return constrain(getDeliveredEnergyJ() / target, 0.0f, 1.0f);
}

// unsigned long getPredictedRemainingTime()
// {
//     float I = getCurrentMA() / 1000.0f;
//     float R = currentProgram().resistance_ohm;
//     float power = I * I * R;
//     if (power < 0.001f) return 99999;
//     float E_remaining = getTargetEnergy() - getDeliveredEnergyJ();
//     if (E_remaining <= 0.0f) return 0;
//     return (unsigned long)((E_remaining / power) * 1000.0f);
// }

unsigned long getPredictedRemainingTime()
{
    float I = getCurrentMA() / 1000.0f;
    float R = PROGRAMS[selectedProgram - 1].resistance_ohm;
    float power = I * I * R;
    if (power < 0.001f)
        return 99999;
    float E_remaining = getTargetEnergy() - getDeliveredEnergyJ();
    if (E_remaining <= 0.0f)
        return 0;
    return (unsigned long)((E_remaining / power) * 1000.0f);
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
    // stopPwmOutput();
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
