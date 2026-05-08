#include "screen.h"
#include "auth.h"
#include "tempsensor.h"
#include "programs.h"
#include "svejse_logs.h"
#include "pwm.h"

namespace
{
    TFT_eSPI tft = TFT_eSPI();

    constexpr int LOGO_X = 35;
    constexpr int LOGO_Y = 10;
    constexpr int LOGO_WIDTH = 250;
    constexpr int LOGO_HEIGHT = 77;
    constexpr int CONTENT_TOP = LOGO_Y + LOGO_HEIGHT + 10;
    constexpr int STATUS_Y = 110;
    constexpr int PIN_Y = 140;

    constexpr unsigned long ERROR_TIME_MS = 1500;
    constexpr unsigned long LOGOUT_TIME_MS = 2000;

    ScreenState currentState = ScreenState::Ready;
    ScreenState baseState = ScreenState::Ready;
    ScreenState lastDrawnState = ScreenState::Ready;

    String pinText = "";
    String lastDrawnPin = "";

    bool needsRedraw = true;
    unsigned long temporaryStateUntil = 0;
    unsigned long remainingMs = 0;

    bool isLogoLess(ScreenState state)
    {
        return state == ScreenState::ProgramSelection || state == ScreenState::Data;
    }

    int screenCenterX()
    {
        return tft.width() / 2;
    }

    int contentHeight()
    {
        int height = tft.height() - CONTENT_TOP;
        return height > 0 ? height : 0;
    }

    unsigned long temporaryDuration(ScreenState state)
    {
        switch (state)
        {
        case ScreenState::InactiveLogout:
            return LOGOUT_TIME_MS;
        case ScreenState::UnknownChip:
        case ScreenState::WrongPin:
            return ERROR_TIME_MS;
        case ScreenState::ProgramConfirmation:
            return 2000;
        case ScreenState::LogData:
            return 2000;
        default:
            return 0;
        }
    }

    void drawLogo()
    {
        tft.setSwapBytes(true);
        tft.pushImage(LOGO_X, LOGO_Y, LOGO_WIDTH, LOGO_HEIGHT, logo);
    }

    void drawCenteredStatus(const String &text, uint16_t color, int y = STATUS_Y)
    {
        tft.setTextColor(color, SPIDER_BG);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(text, screenCenterX(), y, 1);
    }

    // AUTH SCREENS ========================================================================================
    void drawReady()
    {
        drawCenteredStatus("KLAR TIL SCAN", SPIDER_BLUE);
    }

    void drawLoggedIn()
    {
        drawCenteredStatus("Logget ind: " + currentUserName(), SPIDER_BLUE);
    }

    void drawEnterPin()
    {
        drawCenteredStatus("INDTAST PIN:", SPIDER_BLUE);
        drawCenteredStatus(pinText, TFT_RED, PIN_Y);
    }

    void drawScanNewChip()
    {
        drawCenteredStatus("SCAN NY CHIP...", SPIDER_BLUE);
    }

    void drawUnknownChip()
    {
        drawCenteredStatus("CHIP FINDES IKKE!", TFT_RED);
    }

    void drawWrongPin()
    {
        drawCenteredStatus("FORKERT KODE!", TFT_RED);
    }

    void drawInactiveLogout()
    {
        drawCenteredStatus("INAKTIV: LOGGET UD", TFT_RED);
    }

    // USER SCREENS WITH LOGO ========================================================================================
    void drawIdle()
    {
        drawCenteredStatus("TRYK (*) FOR AT", SPIDER_BLUE, STATUS_Y);
        drawCenteredStatus("MAALE TEMPERATUR", SPIDER_BLUE, STATUS_Y + 25);
    }

    void drawMeasuring()
    {
        drawCenteredStatus("MAALER", SPIDER_BLUE);
        drawCenteredStatus("TEMPERATUR...", SPIDER_BLUE, STATUS_Y + 25);
    }

    void drawMeasurementResult(float temp)
    {
        drawCenteredStatus("TEMP: " + String(temp, 1) + " C", SPIDER_BLUE);
    }

    void drawProgramConfirmation()
    {
        tft.setTextSize(2);
        drawCenteredStatus(String(programName(selectedProgram)), SPIDER_BLUE);
    }

    void drawSvejseActive()
    { // maybe add graph with mA and mV?
        tft.setTextSize(2);
        tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("SVEJSER...", screenCenterX(), STATUS_Y, 2);
        tft.drawString(String(remainingMs / 1000.0f, 1) + "S TILBAGE", screenCenterX(), PIN_Y, 2);
    }

    void drawSvejsningApproved()
    {
        tft.setTextColor(TFT_GREEN, SPIDER_BG);
        drawCenteredStatus("SVEJSNING GODKENDT", SPIDER_BLUE);
        drawCenteredStatus("(LOGGER AUTOMATISK)", SPIDER_BLUE, STATUS_Y + 25);
    }

    void drawSvejsningNotApproved()
    {
        tft.setTextSize(2);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_RED, SPIDER_BG);
        tft.drawString("IKKE GODKENDT", screenCenterX(), STATUS_Y, 2);
        tft.setTextSize(1);
        tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
        tft.drawString("(#)PROEV IGEN ", screenCenterX(), STATUS_Y + 30, 1);
        tft.drawString("(1)GEM OG LOG UD", screenCenterX(), STATUS_Y + 50, 1);
    }

    void drawLogData()
    {
        drawCenteredStatus("DATA GEMT!", SPIDER_BLUE);
    }

    void drawChoice()
    {
        tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
        tft.setTextSize(2);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("(*)NY SVEJSNING", screenCenterX(), STATUS_Y);
        tft.drawString("(1)AFSLUT OG LOGUD", screenCenterX(), STATUS_Y + 30);
    }

    // USER SCREENS WITHOUT LOGO ========================================================================================
    void drawProgramSelection()
    {
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(1);

        tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
        tft.drawString("TEMP: " + String(AVG_TEMP, 1) + "C", screenCenterX(), 10, 2);
        tft.drawString("(0)SKIFT  (#)OK  (*)TILBAGE", screenCenterX(), 30, 1);

        int startY = 60;

        for (int p = 1; p <= 4; p++)
        {
            uint16_t col = (p == selectedProgram) ? TFT_RED : SPIDER_BLUE;
            tft.setTextColor(col, SPIDER_BG);

            tft.drawString(programName(p), 160, startY + (p - 1) * 22, 1);
        }
    }

    void drawData()
    {
        tft.setTextSize(1);
        tft.setTextDatum(MC_DATUM); 
        tft.drawString("//////SVEJSE DATA/////: ", screenCenterX(), 10, 1);
        tft.drawString("C: " + String(AVG_TEMP, 1) + " C", screenCenterX(), 30, 1);
        tft.drawString("P: " + String(programName(selectedProgram)), screenCenterX(), 45, 1);
        tft.drawString("T: " + String(svejseDuration / 1000) + " s", screenCenterX(), 60, 1);
        tft.drawString("E: " + String(calculatedOutputEnergy(), 1) + " J", screenCenterX(), 75, 1);
        tft.drawString("M: " + String(getTargetEnergy(), 1) + " J", screenCenterX(), 90, 1);
        bool approved = wasApproved();
        tft.setTextColor(approved ? TFT_GREEN : TFT_RED, SPIDER_BG);
        tft.drawString(approved ? "GODKENDT" : "IKKE GODKENDT", screenCenterX(), 110, 1);
        tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
        tft.setTextSize(2);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("(#)GEM OG FORTSAET", screenCenterX(), 140, 1);
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(2);
    }

    // HELPER FUNCTIONS ========================================================================================
    void clearScreen(bool clearLogo = false)
    {
        if (clearLogo)
        {
            tft.fillScreen(SPIDER_BG);
        }
        else
        {
            tft.fillRect(0, CONTENT_TOP, tft.width(), contentHeight(), SPIDER_BG);
        }
    }

    void applyState(ScreenState state)
    {
        bool changedBaseState = baseState != state;
        baseState = state;

        if (state != ScreenState::EnterPin)
        {
            pinText = "";
        }

        if (temporaryStateUntil != 0)
        {
            return;
        }

        bool changedCurrentState = currentState != baseState;
        currentState = baseState;
        temporaryStateUntil = 0;

        if (changedCurrentState || changedBaseState)
        {
            needsRedraw = true;
        }
    }

    void updatePinPreview(const String &typedPin)
    {
        String hiddenPin = "";

        for (size_t i = 0; i < typedPin.length(); i++)
        {
            hiddenPin += "*";
        }

        if (pinText == hiddenPin)
        {
            return;
        }

        pinText = hiddenPin;

        if (currentState == ScreenState::EnterPin || baseState == ScreenState::EnterPin)
        {
            needsRedraw = true;
        }
    }
}

void forceRedraw()
{
    needsRedraw = true;
}

void setupScreen()
{
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(SPIDER_BG);
    tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
    tft.setTextSize(3);

    drawLogo();
    needsRedraw = true;
    drawScreen();
}

void setScreenState(ScreenState state)
{
    applyState(state);
}

void showTemporaryScreen(ScreenState state)
{
    unsigned long duration = temporaryDuration(state);

    if (duration == 0)
    {
        applyState(state);
        return;
    }

    currentState = state;
    temporaryStateUntil = millis() + duration;
    needsRedraw = true;
}

void screenPinPreview(const String &typedPin)
{
    updatePinPreview(typedPin);
}

void setRemainingTime(unsigned long ms)
{
    remainingMs = ms;
    if (currentState == ScreenState::SvejseActive)
    {
        tft.fillRect(0, CONTENT_TOP, tft.width(), contentHeight(), SPIDER_BG);
        tft.setTextSize(2);
        tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("SVEJSER...", screenCenterX(), STATUS_Y, 2);
        tft.drawString(String(remainingMs / 1000.0f, 1) + "S TILBAGE", screenCenterX(), PIN_Y, 2);
    }
}

// MAIN SCREEN LOGIC ========================================================================================
void drawScreen()
{
    if (temporaryStateUntil != 0 && (long)(millis() - temporaryStateUntil) >= 0)
    {
        currentState = baseState;
        temporaryStateUntil = 0;
        needsRedraw = true;
    }

    if (!needsRedraw && currentState == lastDrawnState && pinText == lastDrawnPin)
    {
        return;
    }

    bool wasLogoLess = isLogoLess(lastDrawnState);
    bool nowLogoLess = isLogoLess(currentState);

    if (nowLogoLess)
    {
        tft.fillScreen(SPIDER_BG);
    }
    else 
    {
        tft.fillScreen(SPIDER_BG);
        drawLogo();
    }


    Serial.println("State changed: " + String((int)lastDrawnState) + " -> " + String((int)currentState));

    switch (currentState)
    {
    case ScreenState::LoggedIn:
        drawLoggedIn();
        break;
    case ScreenState::EnterPin:
        drawEnterPin();
        break;
    case ScreenState::ScanNewChip:
        drawScanNewChip();
        break;
    case ScreenState::UnknownChip:
        drawUnknownChip();
        break;
    case ScreenState::WrongPin:
        drawWrongPin();
        break;
    case ScreenState::InactiveLogout:
        drawInactiveLogout();
        break;
    case ScreenState::Ready:
        drawReady();
        break;
    case ScreenState::Idle:
        drawIdle();
        break;
    case ScreenState::Measuring:
        drawMeasuring();
        break;
    case ScreenState::MeasurementResult:
        drawMeasurementResult(AVG_TEMP);
        break;
    case ScreenState::ProgramSelection:
        drawProgramSelection();
        break;
    case ScreenState::ProgramConfirmation:
        drawProgramConfirmation();
        break;
    case ScreenState::SvejseActive:
        drawSvejseActive();
        break;
    case ScreenState::SvejsningApproved:
        drawSvejsningApproved();
        break;
    case ScreenState::SvejsningNotApproved:
        drawSvejsningNotApproved();
        break;
    case ScreenState::Data:
        drawData();
        break;
    case ScreenState::LogData:
        drawLogData();
        break;
    case ScreenState::Choice:
        drawChoice();
        break;
    default:
        drawReady();
    }

    lastDrawnState = currentState;
    lastDrawnPin = pinText;
    needsRedraw = false;
}
