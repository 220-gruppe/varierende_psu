#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <DFRobot_MLX90614.h>
#include <variabler.h>
#include <programs.h>
#include <functions.h>
#include <config.h>


// * = bit 3  → start measurement / back
// 0 = bit 7  → cycle program
// # = bit 11 → confirm / OK
// 1 = bit 0  → force log (on reject screen)

const int buttonMeasure = 3; // * on numpad
const int buttonCycle = 7;   // 0 on numpad
const int buttonOk = 11;     // # on numpad
const int buttonLogud = 0;   // 1 on numpad

State currentState = IDLE;

float AVG_TEMP = 0.0;
bool svejseGodkendt = false;
float measSum = 0.0;
int measCount = 0;
const int TOTAL_SAMPLES = 1;

// MPR121 electrode indexes must be 0..11
// skal kigge på det her igen
bool buttonPressed(int electrode)
{
    static uint16_t lastTouched = 0;
    uint16_t touched = numpad.touched();
    bool justPressed = (touched & _BV(electrode)) && !(lastTouched & _BV(electrode));
    lastTouched = touched;
    return justPressed;
}

static bool screenDrawn = false;
static State prevState = LOG;

void showIdleScreen()
{
    if (screenDrawn)
        return; // Only draw once
    tft.fillScreen(SPIDER_BG);
    tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("TRYK * FOR AT", 160, 125);
    tft.drawString("MAALE TEMPERATUR", 160, 150);
    screenDrawn = true;
}

void showMeasuringScreen(int count)
{
    tft.fillScreen(SPIDER_BG);
    tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Maaler temperatur...", 160, 100);
}

void showTemperatureScreen()
{
    tft.fillScreen(SPIDER_BG);
    tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Maalt temperatur:", 160, 90);
    tft.drawString(String(AVG_TEMP, 1) + " C", 160, 125);
    tft.drawString("Viser program...", 160, 160);
}

void startMeasurement()
{
    if (screenDrawn)
        return;
    measSum = 0.0;
    measCount = 0;
    sensor.enterSleepMode(false);
    delay(1000);
    tft.fillScreen(SPIDER_BG);
    tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
    tft.setTextSize(3);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Maaler temperatur...", 160, 120);
    screenDrawn = true;
}

void showProgramMenu()
{
    if (screenDrawn)
        return;

    tft.fillScreen(SPIDER_BG);
    tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Temp: " + String(AVG_TEMP, 1) + " C", 160, 40);
    tft.drawString("Vaelg program med 0, godkend med #:", 160, 65);
    int startY = 95;
    int spacing = 18;

    for (int p = 1; p <= 4; p++)
    {
        uint16_t col = (p == selectedProgram) ? TFT_RED : SPIDER_BLUE;
        tft.setTextColor(col, SPIDER_BG);
        tft.drawString(programName(p), 160, startY + (p - 1) * spacing);
    }
    screenDrawn = true;
}

void showSvejseScreen(unsigned long remainingMs)
{
    static unsigned long lastDisplayed = 0xFFFFFFFF;  // force first draw
    unsigned long roundedRemaining = (remainingMs / 300) * 300;  // round to 300ms

    if (roundedRemaining == lastDisplayed) return;  // skip if unchanged
    lastDisplayed = roundedRemaining;

    tft.fillScreen(SPIDER_BG);
    tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
    tft.setTextSize(3);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Svejser...", 160, 120);
    tft.drawString(String(remainingMs / 1000.0f, 1) + "s tilbage", 160, 145);
    tft.pushImage(35, 10, 250, 77, logo);
}

void showProgramConfirmed()
{
    tft.fillScreen(SPIDER_BG);
    tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Program valgt:", 160, 120);
    tft.drawString(programName(selectedProgram), 160, 145);
    delay(3000);
}

void showGodkendt()
{
    tft.fillScreen(SPIDER_BG);
    tft.setTextColor(TFT_GREEN, SPIDER_BG);
    tft.setTextSize(3);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("GODKENDT!", 160, 100);
    tft.setTextSize(2);
    tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
    tft.drawString("Logger om 3 sek...", 160, 150);
}

void showRejected()
{
    if (screenDrawn)
        return;
    tft.fillScreen(SPIDER_BG);
    tft.setTextColor(TFT_RED, SPIDER_BG);
    tft.setTextSize(3);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IKKE GODKENDET", 160, 90);
    tft.setTextSize(2);
    tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
    tft.drawString("# = Prøv igen", 160, 125);
    tft.drawString("1 = Gem og afslut", 160, 140);
    screenDrawn = true;
}

bool measureHandler()
{
    if (measCount < TOTAL_SAMPLES)
    {
        measSum += sensor.getObjectTempCelsius();
        measCount++;
        delay(200);
        return false;
    }
    AVG_TEMP = measSum / TOTAL_SAMPLES;
    // sensor.enterSleepMode(true);
    return true;
}

void setupSensor()
{
    while (NO_ERR != sensor.begin())
    {
        Serial.println("TEMP sensor fejl – prøver igen…");
        delay(300);
    }
    // sensor.enterSleepMode(true);
    Serial.println("TEMP sensor OK");
}

void StateMachine()
{
    static unsigned long stateTimer = 0;
    if (currentState != prevState)
    {
        screenDrawn = false;
        prevState = currentState;
    }

    switch (currentState)
    {

    // IDLE: wait for operator to trigger temperature measurement
    case IDLE:
        showIdleScreen();
        if (buttonPressed(buttonMeasure))
        {
            measSum = 0.0;
            measCount = 0;
            // sensor.enterSleepMode(false);
            delay(500);
            currentState = MEASURE;
        }
        break;

    // MEASURE: collect x-samples
    case MEASURE:
        if (measCount < TOTAL_SAMPLES)
        {
            float reading = sensor.getObjectTempCelsius();
            Serial.print("Sample ");
            Serial.println(measCount);
            Serial.print("Reading: ");
            Serial.println(reading);
            measSum += reading;
            measCount++;
            showMeasuringScreen(measCount);
            delay(500);
        }
        else if (stateTimer == 0)
        {
            AVG_TEMP = measSum / TOTAL_SAMPLES;
            // sensor.enterSleepMode(true);
            Serial.print("Gns. temp: ");
            Serial.println(AVG_TEMP);
            showTemperatureScreen();
            stateTimer = millis();
        }
        else if (millis() - stateTimer >= 3000)
        {
            stateTimer = 0;
            selectedProgram = 1; // default to program 1
            currentState = MENU;
        }

        break;

    // MENU: operator cycles programs, confirms with OK
    case MENU:
        showProgramMenu();
        if (buttonPressed(buttonCycle))
        {
            cycleProgram();
            screenDrawn = false;
        }
        if (buttonPressed(buttonOk))
        {
            if (confirmProgram())
            {
                showProgramConfirmed();
                startSvejse();
                currentState = SVEJSE;
            }
        }
        if (buttonPressed(buttonMeasure))
        {
            screenDrawn = false;
            currentState = IDLE;
        }
        break;

        // SVEJSE: svejse... until countdown is up
    case SVEJSE:
    {
        unsigned long elapsed = millis() - svejseStartTime;
        unsigned long remaining = (elapsed < svejseDuration) ? (svejseDuration - elapsed) : 0;
        showSvejseScreen(remaining);
        if (svejseHandler())
        {
            stopSvejse();
            heatInput = calcLevereretEnergi();
            svejseGodkendt = energiOk();
            currentState = RESULT;
        }
        break;
    }
    // RESULT: show godkendt/ ikke godkendt; retry or proceed to log
    case RESULT:
        if (svejseGodkendt)
        {
            showGodkendt();
            delay(3000);
            currentState = DATA;
        }
        else
        {
            showRejected();
            if (buttonPressed(buttonOk))
            { // retry same program, ask Jakob what happens if svejsning ikke godkendt
                startSvejse();
                screenDrawn = false;
                currentState = SVEJSE;
            }
            if (buttonPressed(buttonLogud))
            { // force-log and return
                currentState = DATA;
            }
        }
        break;

    // DATA: show any relevant data
    case DATA:
        if (!screenDrawn)
        {
            tft.fillScreen(SPIDER_BG);
            tft.setTextColor(SPIDER_BLUE, SPIDER_BG);            
            tft.setTextSize(1);
            tft.setTextDatum(ML_DATUM); // left-align for compact layout
            tft.drawString("SVEJSE DATA: ", 10, 20);
            tft.drawString("Temp:     " + String(AVG_TEMP, 1) + " C", 10, 35);
            tft.drawString("Program:  " + String(programName(selectedProgram)), 10, 50);
            tft.drawString("Varighed: " + String(svejseDuration / 1000) + " s", 10, 65);
            tft.drawString("Energi:   " + String(calcLevereretEnergi(), 1) + " J", 10, 80);
            tft.drawString("Maal:     " + String(getTargetEnergy(), 1) + " J", 10, 95);
            tft.setTextColor(svejseGodkendt ? TFT_GREEN : TFT_RED, SPIDER_BG);
            tft.drawString(svejseGodkendt ? "GODKENDT" : "IKKE GODKENDT", 10, 115);
            tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
            tft.setTextSize(2);
            tft.setTextDatum(ML_DATUM);
            tft.drawString("# = Gem og fortsaet", 10, 140);
            screenDrawn = true;
        }
        if (buttonPressed(buttonOk))
            currentState = LOG;
        break;

    // LOG: save weld data, pause, then reset to IDLE
    case LOG:
        saveData();
        tft.fillScreen(SPIDER_BG);
        tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
        tft.setTextSize(2);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("Data gemt!", 160, 100);
        delay(3000);
        selectedProgram = 0;
        tidStart = 0;
        currentState = CHOICE;
        break;

    // CHOICE: handle any choices or confirmations needed before going back to IDLE
    case CHOICE:
        if (!screenDrawn)
        {
            tft.fillScreen(SPIDER_BG);
            tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
            tft.setTextSize(2);
            tft.setTextDatum(MC_DATUM);
            tft.drawString("Tryk * for ny svejsning, ", 160, 70);
            tft.drawString("eller 1 for at afslut og logud", 160, 110);
            screenDrawn = true;
        }
        if (buttonPressed(buttonMeasure))
        {
            currentState = IDLE;
        }
        if (buttonPressed(buttonLogud))
        {
            tft.fillScreen(SPIDER_BG);
            tft.pushImage(35, 10, 250, 77, logo);
            isLoggedIn = false;
            workerID = "";
            tidStart = 0;
            selectedProgram = 0;
            currentState = IDLE;
        }
        break;
    }
}

#endif