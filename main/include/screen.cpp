#include "screen.h"
#include "auth.h"
#include "tempsensor.h"
#include "programs.h"

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

    void drawIdle()
    {
        drawCenteredStatus("TRYK (*) FOR AT MAALE TEMPERATUR", SPIDER_BLUE);
    }

    void drawMeasuring()
    {
        drawCenteredStatus("MAALER TEMPERATUR...", SPIDER_BLUE);
    }

    void drawMeasurementResult(float temp)
    {
        drawCenteredStatus("TEMPERATUR: " + String(temp, 1) + " C", SPIDER_BLUE);
    }

    void drawProgramSelection()
    {
        tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("Temperatur: " + String(AVG_TEMP, 1) + " C", screenCenterX(), STATUS_Y, 1);
        tft.drawString("Vælg program med (0)", screenCenterX(), PIN_Y, 1);
        tft.drawString("Godkend med (#)", screenCenterX(), PIN_Y + 30, 1);
        int startY = PIN_Y + 50;
        int spacing = 20;

        for (int p = 1; p <= 4; p++)
        {
            uint16_t col = (p == selectedProgram) ? TFT_RED : SPIDER_BLUE;
            tft.setTextColor(col, SPIDER_BG);
            tft.drawString(programName(p), screenCenterX(), startY + (p - 1) * spacing);
        }
    }

    void drawProgramConfirmation()
    {
        drawCenteredStatus("VALGT PROGRAM: " + String(programName(selectedProgram)), SPIDER_BLUE);
    }

    void drawSvejseActive()
    {
        tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("SVEJSER...", screenCenterX(), STATUS_Y, 1);
        tft.drawString(String(remainingMs / 1000.0f, 1) + "s tilbage", screenCenterX(), PIN_Y, 1);
    }

    void drawSvejsningApproved()
    {
        tft.setTextColor(TFT_GREEN, SPIDER_BG);
        drawCenteredStatus("SVEJSNING GODKENDT (LOGGER AUTOMATISK)", SPIDER_BLUE);
    }

    void drawSvejsningNotApproved()
    {
        tft.setTextColor(TFT_RED, SPIDER_BG);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("SVEJSNING IKKE GODKENDT", screenCenterX(), STATUS_Y, 1);
        tft.drawString("TRYK (#) FOR AT PROEVE IGEN", screenCenterX(), PIN_Y, 1);
        tft.drawString("ELLER 1 FOR AT GEMME OG LOGGE UD", screenCenterX(), PIN_Y + 30, 1);
    }

    void drawData()
    {

        tft.setTextSize(1);
        tft.setTextDatum(ML_DATUM); // left-align for compact layout
        tft.drawString("SVEJSE DATA: ", 10, 20);
        tft.drawString("Temp:     " + String(AVG_TEMP, 1) + " C", 10, 35);
        tft.drawString("Program:  " + String(programName(selectedProgram)), 10, 50);
        tft.drawString("Varighed: " + String(svejseDuration / 1000) + " s", 10, 65);
        tft.drawString("Energi:   " + String(calculatedOutputEnergy(), 1) + " J", 10, 80);
        tft.drawString("Maal:     " + String(getTargetEnergy(), 1) + " J", 10, 95);
        tft.setTextColor(SvejsningNotApproved ? TFT_GREEN : TFT_RED, SPIDER_BG);
        tft.drawString(SvejsningApproved ? "GODKENDT" : "IKKE GODKENDT", 10, 115);
        tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
        tft.setTextSize(2);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("# = Gem og fortsaet", 10, 140);
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
        tft.drawString("TRYK (*) FOR NY SVEJSNING", screenCenterX(), STATUS_Y);
        tft.drawString("TRYK (1) FOR AFSLUT OG LOGUD", screenCenterX(), STATUS_Y + 30);
    }

    void clearScreen()
    {
        tft.fillRect(0, CONTENT_TOP, tft.width(), contentHeight(), SPIDER_BG);
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
        needsRedraw = true;
}

void drawScreen()
{
    if (temporaryStateUntil != 0 && (long)(millis() - temporaryStateUntil) >= 0)
    {
        currentState = baseState;
        temporaryStateUntil = 0;
        needsRedraw = true;
    }

    bool sameState = currentState == lastDrawnState;
    bool samePin = pinText == lastDrawnPin;

    if (!needsRedraw && sameState && samePin)
    {
        return;
    }

    clearScreen();

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
        break;
    }

    lastDrawnState = currentState;
    lastDrawnPin = pinText;
    needsRedraw = false;
}
