#include "screen.h"

TFT_eSPI tft = TFT_eSPI();

namespace
{
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
ScreenState lastDrawnState = ScreenState::Ready;
ScreenState returnState = ScreenState::Ready;

String currentName = "";
String lastDrawnName = "";
String pinText = "";
String lastDrawnPin = "";

bool needsRedraw = true;
unsigned long stateTimeout = 0;

int screenCenterX()
{
    return tft.width() / 2;
}

int contentHeight()
{
    int height = tft.height() - CONTENT_TOP;
    return height > 0 ? height : 0;
}

bool isTemporaryState(ScreenState state)
{
    return state == ScreenState::UnknownChip ||
           state == ScreenState::WrongPin ||
           state == ScreenState::InactiveLogout;
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

String maskPin(const String &typedPin)
{
    String masked = "";

    for (size_t i = 0; i < typedPin.length(); i++)
    {
        masked += "*";
    }

    return masked;
}

void drawReady()
{
    drawCenteredStatus("KLAR TIL SCAN", SPIDER_BLUE);
}

void drawLoggedIn()
{
    drawCenteredStatus("Logget ind: " + currentName, SPIDER_BLUE);
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

void changeState(ScreenState state, const String &name = "")
{
    bool changed = currentState != state || currentName != name;

    currentState = state;
    currentName = name;
    stateTimeout = 0;

    if (state != ScreenState::EnterPin)
    {
        pinText = "";
    }

    if (changed)
    {
        needsRedraw = true;
    }
}

void changeTemporaryState(ScreenState state, unsigned long showTimeMs)
{
    if (!isTemporaryState(currentState))
    {
        returnState = currentState;
    }

    currentState = state;
    stateTimeout = millis() + showTimeMs;
    needsRedraw = true;
}

void handleTimeout()
{
    if (stateTimeout == 0)
    {
        return;
    }

    if ((long)(millis() - stateTimeout) < 0)
    {
        return;
    }

    currentState = returnState;
    stateTimeout = 0;
    needsRedraw = true;
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

void clearScreen()
{
    tft.fillRect(0, CONTENT_TOP, tft.width(), contentHeight(), SPIDER_BG);
}

void clearPinArea()
{
    tft.fillRect(0, 120, tft.width(), tft.height() - 120, SPIDER_BG);
}

void screenReady()
{
    changeState(ScreenState::Ready);
}

void screenLoggedIn(const String &name)
{
    changeState(ScreenState::LoggedIn, name);
}

void screenEnterPin()
{
    changeState(ScreenState::EnterPin);
}

void screenScanNewChip()
{
    changeState(ScreenState::ScanNewChip);
}

void screenUnknownChip()
{
    changeTemporaryState(ScreenState::UnknownChip, ERROR_TIME_MS);
}

void screenWrongPin()
{
    changeTemporaryState(ScreenState::WrongPin, ERROR_TIME_MS);
}

void screenInactiveLogout()
{
    changeTemporaryState(ScreenState::InactiveLogout, LOGOUT_TIME_MS);
}

void setScreenState(ScreenState state)
{
    changeState(state);
}

void screenPinPreview(const String &typedPin)
{
    drawPinInput(maskPin(typedPin));
}

void drawPinInput(const String &maskedPin)
{
    if (pinText == maskedPin)
    {
        return;
    }

    pinText = maskedPin;

    if (currentState == ScreenState::EnterPin)
    {
        needsRedraw = true;
    }
}

void drawScreen()
{
    handleTimeout();

    bool sameState = currentState == lastDrawnState;
    bool sameName = currentName == lastDrawnName;
    bool samePin = pinText == lastDrawnPin;

    if (!needsRedraw && sameState && sameName && samePin)
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
    lastDrawnName = currentName;
    lastDrawnPin = pinText;
    needsRedraw = false;
}