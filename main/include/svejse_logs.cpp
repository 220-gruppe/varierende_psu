#include "svejse_logs.h"
#include "tempsensor.h"
#include "programs.h"
#include "pwm.h"
#include "database.h"

constexpr float MODSTAND_OHM = 0.3729f; // resistance of thicker wire

extern float sumMV;
extern unsigned long svejseDuration;
extern float AVG_TEMP;

static SvejsningStatus lastStatus = SvejsningStatus::NotApproved;

static const char *SVEJSE_LOG_TABLE = "svejse_logs";
static const char *SVEJSE_LOG_COLUMNS = "STATUS, CALCULATED_ENERGY, TARGET_ENERGY, AVG_TEMP, SVEJSNING_TIME";

float calculatedOutputEnergy()
{ 
    float t = svejseDuration / 1000.0f;
    return getCurrentMA() * getCurrentMA() * MODSTAND_OHM * t;
}

float getTargetEnergy()
{
    return 0.032f * 500.0f * (4300.0f - AVG_TEMP); // thicker wire for mass (0.032)
}

SvejsningStatus GetSvejsningStatus()
{
    lastStatus = (calculatedOutputEnergy() >= getTargetEnergy())
                     ? SvejsningStatus::Approved
                     : SvejsningStatus::NotApproved;
    return lastStatus;
}

void saveSvejsningResult()
{
    GetSvejsningStatus();
}

bool wasApproved()
{
    return lastStatus == SvejsningStatus::Approved;
}

void LogSvejseData()
{
    String svejsningResult = wasApproved() ? "GODKENDT" : "IKKE GODKENDT";
    String calculatedEnergy = String(calculatedOutputEnergy(), 2);
    String avgTemp = String(AVG_TEMP, 2);
    float svejsningTime = svejseDuration / 1000.0f;

    writeSvejseLog(svejsningResult, calculatedEnergy, avgTemp, svejsningTime);
}

bool writeSvejseLog(const String &svejsningResult, const String &calculatedEnergy, const String &avgTemp, float svejsningTime)
{
    if (svejsningResult.length() == 0 || calculatedEnergy.length() == 0 || avgTemp.length() == 0 || svejsningTime <= 0.0f)
    {
        return false;
    }
    DB(SVEJSE_LOG_TABLE, SVEJSE_LOG_COLUMNS);
    String line = svejsningResult + "," + calculatedEnergy + "," + String(getTargetEnergy()) + "," + avgTemp + "," + String(svejsningTime, 3);
    return DB(line);
}