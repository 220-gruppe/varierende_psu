#ifndef SCREEN_H
#define SCREEN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "logo.h"

constexpr uint16_t SPIDER_BG = 0xE6D6;
constexpr uint16_t SPIDER_BLUE = TFT_BLUE;

enum class ScreenState
{
    Ready,
    LoggedIn,
    EnterPin,
    ScanNewChip,
    UnknownChip,
    WrongPin,
    InactiveLogout
};

extern TFT_eSPI tft;

void setupScreen();
void clearScreen();
void clearPinArea();

void screenReady();
void screenLoggedIn(const String &name);
void screenEnterPin();
void screenScanNewChip();
void screenUnknownChip();
void screenWrongPin();
void screenInactiveLogout();

void setScreenState(ScreenState state);
void screenPinPreview(const String &typedPin);
void drawPinInput(const String &maskedPin);
void drawScreen();
void opdaterScreen();

#endif
