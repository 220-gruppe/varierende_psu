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

void setupScreen();
void setScreenState(ScreenState state);
void showTemporaryScreen(ScreenState state);
void screenPinPreview(const String &typedPin);
void drawScreen();

#endif
