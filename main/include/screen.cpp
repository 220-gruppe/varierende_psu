#include "screen.h"
#include "auth.h"

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
    default:
        drawReady();
        break;
    }

    lastDrawnState = currentState;
    lastDrawnPin = pinText;
    needsRedraw = false;
}
