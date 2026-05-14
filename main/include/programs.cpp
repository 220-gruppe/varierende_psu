#include "programs.h"
#include "tempsensor.h"
#include "pwm.h"

// const unsigned long svejseTime_1 = 50000; // test værdi, adjust l8r
// const unsigned long svejseTime_2 = 60000;
// const unsigned long svejseTime_3 = 70000;
// const unsigned long svejseTime_4 = 183000;


const Program PROGRAMS[] = {
    { "Program 1 - Standard" , 0.3729f, 0.032f, 4300.0f }, 
    { "Program 2 - XXXXXXXX" , 0.0f, 0.0f, 0.0f }, //placeholder værdier
    { "Program 3 - XXXXXXXX" , 0.0f, 0.0f, 0.0f },
    { "Program 4 - XXXXXXXX" , 0.0f, 0.0f, 0.0f },
};
const int NUM_PROGRAMS = sizeof(PROGRAMS) / sizeof(PROGRAMS[0]);

int           selectedProgram = 0;
unsigned long svejseStartTime = 0;
unsigned long svejseDuration  = 0;
bool          svejseAktiv     = false;
 
// unsigned long getSvejseTime()
// {
//     switch (selectedProgram)
//     {
//     case 1:
//         return svejseTime_1;
//     case 2:
//         return svejseTime_2;
//     case 3:
//         return svejseTime_3;
//     case 4:
//         return svejseTime_4;
//     default:
//         return 0;
//     }
// }

// const char *programName(int p)
// {
//     switch (p)
//     {
//     case 1:
//         return "Program 1 "; 
//     case 2:
//         return "Program 2 ";
//     case 3:
//         return "Program 3 ";
//     case 4:
//         return "Program 4 ";
//     default:
//         return "Intet  valgt";
//     }
// }

const Program& currentProgram() 
{
    int index = constrain(selectedProgram - 1, 0, NUM_PROGRAMS -1);
    return PROGRAMS[index];
}

const char* programName(int p)
{
    if (p < 1 || p > NUM_PROGRAMS) return "Intet valgt";
    return PROGRAMS[p-1].name;
}

bool confirmPRogram()
{
 return (selectedProgram >= 1 && selectedProgram <= NUM_PROGRAMS);
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


float getTargetEnergy()
{
    const Program& p = currentProgram();
    return p.mass_kg * 500.0f * (p.targetTemp_C - AVG_TEMP); 
}

float getSvejseProgress()
{
    float target = getTargetEnergy();
    if (target <= 0.0f) return 1.0f;
    return constrain(getDeliveredEnergyJ() / target, 0.0f, 1.0f);
}

unsigned long getPredictedRemainingTime()
{
    float I = getCurrentMA() / 1000.0f;
    float R = currentProgram().resistance_ohm;
    float power = I * I * R;
    if (power < 0.001f) return 99999;
    float E_remaining = getTargetEnergy() - getDeliveredEnergyJ();
    if (E_remaining <= 0.0f) return 0;
    return (unsigned long)((E_remaining / power) * 1000.0f);
}

void startSvejse()
{
    // svejseDuration = getSvejseTime(); 
    svejseStartTime = millis();
    svejseDuration = 0;
    svejseAktiv = true;
    Serial.print("startSvejse called. Duration: ");
    Serial.print(svejseDuration);
    Serial.print(" startTime: ");
    Serial.println(svejseStartTime);

    startEnergyAccumulator();

    // Placeholder: Activate welding output pin here
    // add turn on svejsning
}

bool svejseHandler()
{
    if (!svejseAktiv) return true;

    unsigned long elapsed       = millis() - svejseStartTime;
    unsigned long remaining     = getPredictedRemainingTime();
    svejseDuration              = elapsed + remaining;

    return getSvejseProgress() >= 1.0f;

    // if (!svejseAktiv)
    //     return false;
    // unsigned long elapsed = millis() - svejseStartTime;

    // if (elapsed >= svejseDuration)
    // {
    //     stopSvejse();
    //     return true; 
    // }
    // return false; 
}

void stopSvejse()
{
    svejseAktiv = false;
    // stopPwmOutput();
}




