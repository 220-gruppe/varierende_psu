#include "svejse_logs.h"
#include "tempsensor.h"
#include "programs.h"
#include "pwm.h"
#include "auth.h"
#include "database.h"

constexpr float MODSTAND_OHM = 0.3729f; // resistance of thicker wire

extern unsigned long svejseDuration;
extern float AVG_TEMP;

static SvejsningStatus lastStatus = SvejsningStatus::NotApproved;

static const char *SVEJSE_LOG_TABLE = "svejse_logs";
static const char *SVEJSE_LOG_COLUMNS = "USER, UID, STATUS, CALCULATED_ENERGY, TARGET_ENERGY, AVG_TEMP, SVEJSNING_TIME";
static const String SVEJSE_LOG_DIR = "/svejsninger";
static String activeLogFilePath = "";
static String activeWeldUid = "";
static String activeWeldUser = "";
static int activeWeldProgram = 0;
static unsigned long activeWeldStartMs = 0;
static unsigned long lastMeasurementWriteMs = 0;
static float activeWeldTargetJoule = 0.0f;
static bool isSvejsningLogOpen = false;

static bool ensureSvejsningDirectory()
{
    if (SD.exists(SVEJSE_LOG_DIR))
    {
        return true;
    }

    if (SD.mkdir(SVEJSE_LOG_DIR))
    {
        Serial.print("Oprettede svejsningsmappe: ");
        Serial.println(SVEJSE_LOG_DIR);
        return true;
    }

    Serial.print("Kunne ikke oprette svejsningsmappe: ");
    Serial.println(SVEJSE_LOG_DIR);
    return false;
}

static int getNextSvejsningIndex()
{
    File dir = SD.open(SVEJSE_LOG_DIR);
    if (!dir || !dir.isDirectory())
    {
        if (dir)
        {
            dir.close();
        }
        return 1;
    }

    int count = 0;
    File entry;
    while ((entry = dir.openNextFile()))
    {
        count++;
        entry.close();
    }
    dir.close();
    return count + 1;
}

static String buildSvejsningLogPath(const String &uid, int programNumber, unsigned long startMs)
{
    int index = getNextSvejsningIndex();
    String fileName = SVEJSE_LOG_DIR;
    fileName += "/svejsning_";
    fileName += String(index);
    fileName += "_";
    fileName += uid.length() > 0 ? uid : "UNKNOWN";
    fileName += "_p";
    fileName += String(programNumber);
    fileName += "_";
    fileName += String(startMs);
    fileName += ".csv";
    return fileName;
}

float calculatedOutputEnergy()
{
    float t = svejseDuration / 1000.0f;
    return getCurrentMA() * getCurrentMA() * MODSTAND_OHM * t;
}

float getTargetEnergy()
{
    return getTargetJoule();
}

void saveSvejsningResult()
{
    lastStatus = (getTotalJoule() >= getTargetJoule())
                     ? SvejsningStatus::Approved
                     : SvejsningStatus::NotApproved;
}

bool wasApproved()
{
    return lastStatus == SvejsningStatus::Approved;
}

bool initializeSvejsningLog(const String &uid, int programNumber, unsigned long startMs, float targetJoule)
{
    if (!ensureSvejsningDirectory())
    {
        return false;
    }

    activeWeldUid = uid.length() > 0 ? uid : "UNKNOWN";
    activeWeldUser = currentUserName();
    activeWeldProgram = programNumber;
    activeWeldStartMs = startMs;
    activeWeldTargetJoule = targetJoule;
    activeLogFilePath = buildSvejsningLogPath(activeWeldUid, programNumber, startMs);

    File file = SD.open(activeLogFilePath, FILE_WRITE, true);
    if (!file)
    {
        Serial.print("Kunne ikke oprette svejsningslogfil: ");
        Serial.println(activeLogFilePath);
        activeLogFilePath = "";
        return false;
    }

    file.println("UID,USER,PROGRAM,TARGET_JOULE,START_MS");
    file.print(activeWeldUid);
    file.print(",");
    file.print(activeWeldUser);
    file.print(",");
    file.print(activeWeldProgram);
    file.print(",");
    file.print(activeWeldTargetJoule, 2);
    file.print(",");
    file.println(activeWeldStartMs);
    file.println();
    file.println("TIMESTAMP_MS,ELAPSED_MS,current_mA,voltage_V,total_JOULE");
    file.close();

    lastMeasurementWriteMs = startMs;
    isSvejsningLogOpen = true;
    return true;
}

bool appendSvejsningMeasurement(unsigned long timestampMs, float currentMA, float voltageV, float totalJoule)
{
    if (!isSvejsningLogOpen || activeLogFilePath.length() == 0)
    {
        return false;
    }

    if (timestampMs - lastMeasurementWriteMs < 1000UL)
    {
        return false;
    }

    lastMeasurementWriteMs = timestampMs;
    File file = SD.open(activeLogFilePath, FILE_APPEND);
    if (!file)
    {
        Serial.print("Kunne ikke åbne aktiv svejsningslogfil: ");
        Serial.println(activeLogFilePath);
        return false;
    }

    file.print(timestampMs);
    file.print(",");
    file.print(timestampMs - activeWeldStartMs);
    file.print(",");
    file.print(currentMA, 2);
    file.print(",");
    file.print(voltageV, 3);
    file.print(",");
    file.println(totalJoule, 3);
    file.close();
    return true;
}

void finalizeSvejsningLog(const String &status, float totalJoule, float targetJoule, const String &avgTemp, float svejsningTime)
{
    if (!isSvejsningLogOpen || activeLogFilePath.length() == 0)
    {
        return;
    }

    File file = SD.open(activeLogFilePath, FILE_APPEND);
    if (!file)
    {
        Serial.print("Kunne ikke åbne aktiv svejsningslogfil til afslutning: ");
        Serial.println(activeLogFilePath);
        return;
    }

    file.println();
    file.println("SUMMARY");
    file.print("STATUS,");
    file.println(status);
    file.print("TOTAL_JOULE,");
    file.println(totalJoule, 2);
    file.print("TARGET_JOULE,");
    file.println(targetJoule, 2);
    file.print("AVG_TEMP,");
    file.println(avgTemp);
    file.print("SVEJSNING_TIME_s,");
    file.println(svejsningTime, 3);
    file.close();

    activeLogFilePath = "";
    activeWeldUid = "";
    activeWeldUser = "";
    activeWeldProgram = 0;
    activeWeldStartMs = 0;
    lastMeasurementWriteMs = 0;
    activeWeldTargetJoule = 0.0f;
    isSvejsningLogOpen = false;
}

void LogSvejseData()
{
    String svejsningResult = wasApproved() ? "GODKENDT" : "IKKE GODKENDT";
    String calculatedEnergy = String(getTotalJoule(), 2);
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
    String user = currentUserName();
    String uid = currentUserUID();
    String line = user + "," + uid + "," + svejsningResult + "," + calculatedEnergy + "," + String(getTargetJoule()) + "," + avgTemp + "," + String(svejsningTime, 3);
    bool written = databaseWrite(line);
    finalizeSvejsningLog(svejsningResult, getTotalJoule(), getTargetJoule(), avgTemp, svejsningTime);
    return written;
}