#include "svejse_logs.h"
#include "tempsensor.h"
#include "programs.h"
#include "pwm.h"
#include "auth.h"
#include "database.h"

extern float AVG_TEMP;

static SvejsningStatus lastStatus = SvejsningStatus::NotApproved;

static const char *SVEJSE_SUMMARY_FILE = "/svejse_logs.csv";
static const char *SVEJSE_LOG_COLUMNS  = "USER,UID,STATUS,TOTAL_JOULE,TARGET_CURRENT_MA,AVG_TEMP,SVEJSNING_TIME,TARGET_TIME_S";
static const String SVEJSE_LOG_DIR     = "/svejsninger";
static String activeLogFilePath        = "";
static String lastCompletedLogFilePath = "";
static String activeWeldUid            = "";
static String activeWeldUser           = "";

static int activeWeldProgram                    = 0;
static unsigned long activeWeldStartMs          = 0;
static unsigned long lastMeasurementWriteMs     = 0;
static float activeWeldTargetCurrentMA          = 0.0f;
static unsigned long activeWeldTargetDurationMs = 0;

static bool isSvejsningLogOpen                  = false;
static bool currentWeldSummarySaved             = false;

static bool ensureSvejsningDirectory()
{
    if (!isSdReady())
    {
        Serial.println("SD er ikke monteret, kan ikke bruge svejselog mappe");
        return false;
    }

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

static String fileNameOnly(const String &path)
{
    int slash = path.lastIndexOf('/');
    return slash == -1 ? path : path.substring(slash + 1);
}

static String fullSvejsningPath(const String &entryName)
{
    if (entryName.startsWith("/"))
    {
        return entryName;
    }

    String path = SVEJSE_LOG_DIR;
    path += "/";
    path += entryName;
    return path;
}

static int svejsningIndexFromName(const String &entryName)
{
    String name = fileNameOnly(entryName);
    constexpr char PREFIX[] = "svejsning_";

    if (!name.startsWith(PREFIX))
    {
        return 0;
    }

    int indexStart = strlen(PREFIX);
    int indexEnd = name.indexOf('_', indexStart);
    if (indexEnd == -1)
    {
        return 0;
    }

    return name.substring(indexStart, indexEnd).toInt();
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

    int highestIndex = 0;
    File entry;
    while ((entry = dir.openNextFile()))
    {
        if (!entry.isDirectory())
        {
            int index = svejsningIndexFromName(entry.name());
            if (index > highestIndex)
            {
                highestIndex = index;
            }
        }

        entry.close();
    }
    dir.close();
    return highestIndex + 1;
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

static void printFilePreview(File &file, size_t maxLines)
{
    file.seek(0);

    size_t lineCount = 0;
    while (file.available() && lineCount < maxLines)
    {
        String line = file.readStringUntil('\n');
        line.trim();
        Serial.print("    ");
        Serial.println(line);
        lineCount++;
    }

    if (file.available())
    {
        Serial.println("    ...");
    }
}

static void printDirectory(File dir, int depth)
{
    File entry;
    while ((entry = dir.openNextFile()))
    {
        String entryName = entry.name();
        for (int i = 0; i < depth; i++)
        {
            Serial.print("  ");
        }

        if (entry.isDirectory())
        {
            Serial.print("[DIR] ");
            Serial.println(entryName);
            printDirectory(entry, depth + 1);
        }
        else
        {
            Serial.print("[FILE] ");
            Serial.print(entryName);
            Serial.print(" (");
            Serial.print(entry.size());
            Serial.println(" bytes)");

            if (entryName.endsWith(".csv"))
            {
                printFilePreview(entry, 12);
            }
        }

        entry.close();
    }
}

float calculatedOutputEnergy()
{
    float t = getSvejseElapsedTime() / 1000.0f;
    float currentA = getCurrentMA() / 1000.0f;
    return currentA * currentA * OUTPUT_RESISTANCE_OHM * t;
}

void saveSvejsningResult()
{
    lastStatus = (!hasWeldFault() && getSvejseTime() > 0 && getSvejseElapsedTime() >= getSvejseTime())
                     ? SvejsningStatus::Approved
                     : SvejsningStatus::NotApproved;
}

bool wasApproved()
{
    return lastStatus == SvejsningStatus::Approved;
}

bool initializeSvejsningLog(const String &uid, int programNumber, unsigned long startMs, float targetCurrentMA, unsigned long targetDurationMs)
{
    if (!ensureSvejsningDirectory())
    {
        return false;
    }

    activeWeldUid              = uid.length() > 0 ? uid : "UNKNOWN";
    activeWeldUser             = currentUserName();
    activeWeldProgram          = programNumber;
    activeWeldStartMs          = startMs;
    activeWeldTargetCurrentMA  = targetCurrentMA;
    activeWeldTargetDurationMs = targetDurationMs;
    activeLogFilePath          = buildSvejsningLogPath(activeWeldUid, programNumber, startMs);
    currentWeldSummarySaved    = false;

    File file = SD.open(activeLogFilePath, FILE_WRITE, true);
    if (!file)
    {
        Serial.print("Kunne ikke oprette svejsningslogfil: ");
        Serial.println(activeLogFilePath);
        activeLogFilePath = "";
        return false;
    }

    file.println("UID,USER,PROGRAM,TARGET_CURRENT_MA,TARGET_TIME_MS,START_MS");
    file.print(activeWeldUid);
    file.print(",");
    file.print(activeWeldUser);
    file.print(",");
    file.print(activeWeldProgram);
    file.print(",");
    file.print(activeWeldTargetCurrentMA, 2);
    file.print(",");
    file.print(activeWeldTargetDurationMs);
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

    if (timestampMs - lastMeasurementWriteMs < SVEJSNING_MEASUREMENT_INTERVAL_MS)
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

void finalizeSvejsningLog(const String &status, float totalJoule, float targetCurrentMA, const String &avgTemp, float svejsningTime, unsigned long targetDurationMs)
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
    file.print("FAULT_REASON,");
    file.println(weldFaultText());
    file.print("TOTAL_JOULE,");
    file.println(totalJoule, 2);
    file.print("TARGET_CURRENT_MA,");
    file.println(targetCurrentMA, 2);
    file.print("TARGET_TIME_s,");
    file.println(targetDurationMs / 1000.0f, 3);
    file.print("AVG_TEMP,");
    file.println(avgTemp);
    file.print("SVEJSNING_TIME_s,");
    file.println(svejsningTime, 3);
    file.close();

    lastCompletedLogFilePath   = activeLogFilePath;
    activeLogFilePath          = "";
    activeWeldUid              = "";
    activeWeldUser             = "";
    activeWeldProgram          = 0;
    activeWeldStartMs          = 0;
    lastMeasurementWriteMs     = 0;
    activeWeldTargetCurrentMA  = 0.0f;
    activeWeldTargetDurationMs = 0;
    isSvejsningLogOpen         = false;
}

void printSdCardContents()
{
    Serial.println();
    Serial.println("========== SD KORT INDHOLD ==========");

    if (!isSdReady())
    {
        Serial.println("SD er ikke monteret");
        Serial.println("======================================");
        Serial.println();
        return;
    }

    File root = SD.open("/");
    if (!root || !root.isDirectory())
    {
        Serial.println("Kunne ikke aabne SD root");
        if (root)
        {
            root.close();
        }
        Serial.println("======================================");
        return;
    }

    printDirectory(root, 0);
    root.close();
    Serial.println("======================================");
    Serial.println();
}

void removeSvejsningTestLogs()
{
    if (!ensureSvejsningDirectory())
    {
        return;
    }

    File dir = SD.open(SVEJSE_LOG_DIR);
    if (!dir || !dir.isDirectory())
    {
        if (dir)
        {
            dir.close();
        }
        return;
    }

    int removedCount = 0;
    File entry;
    while ((entry = dir.openNextFile()))
    {
        String entryName = entry.name();
        bool isDirectory = entry.isDirectory();
        entry.close();

        String name = fileNameOnly(entryName);
        if (!isDirectory && name.indexOf("SDTEST") != -1)
        {
            String path = fullSvejsningPath(entryName);
            if (SD.remove(path.c_str()))
            {
                removedCount++;
                Serial.print("Slettede test-svejsning: ");
                Serial.println(path);
            }
            else
            {
                Serial.print("Kunne ikke slette test-svejsning: ");
                Serial.println(path);
            }
        }
    }

    dir.close();

    if (removedCount > 0)
    {
        Serial.print("Slettede SDTEST svejsefiler i alt: ");
        Serial.println(removedCount);
    }
}

void LogSvejseData()
{
    String svejsningResult = wasApproved() ? "GODKENDT" : (hasWeldFault() ? String(weldFaultText()) : "IKKE GODKENDT");
    String calculatedEnergy = String(getTotalJoule(), 2);
    String avgTemp = String(AVG_TEMP, 2);
    float svejsningTime = getSvejseElapsedTime() / 1000.0f;

    writeSvejseLog(svejsningResult, calculatedEnergy, avgTemp, svejsningTime);
}

bool writeSvejseLog(const String &svejsningResult, const String &calculatedEnergy, const String &avgTemp, float svejsningTime)
{
    if (currentWeldSummarySaved)
    {
        Serial.println("Svejselog er allerede gemt - springer dublet over");
        return true;
    }

    if (svejsningResult.length() == 0 || calculatedEnergy.length() == 0 || avgTemp.length() == 0 || svejsningTime <= 0.0f)
    {
        return false;
    }
    if (!ensureCsvFile(SVEJSE_SUMMARY_FILE, SVEJSE_LOG_COLUMNS))
    {
        return false;
    }

    String user = currentUserName();
    String uid = currentUserUID();
    String line = user + "," + uid + "," + svejsningResult + "," + calculatedEnergy + "," + String(getSvejseTargetCurrentMA(), 0) + "," + avgTemp + "," + String(svejsningTime, 3) + "," + String(getSvejseTime() / 1000.0f, 3);
    bool written = appendLineToFile(SVEJSE_SUMMARY_FILE, line);
    finalizeSvejsningLog(svejsningResult, getTotalJoule(), getSvejseTargetCurrentMA(), avgTemp, svejsningTime, getSvejseTime());
    currentWeldSummarySaved = written;
    return written;
}
