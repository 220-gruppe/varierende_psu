#ifndef SCREEN_H
#define SCREEN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "logo.h"
#include "programs.h"

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
    InactiveLogout,
    Idle,
    Measuring,
    MeasurementResult,
    ProgramSelection,
    ProgramConfirmation,
    SvejseActive,
    SvejsningApproved,
    SvejsningNotApproved,
    Data,
    LogData,
    Choice
};

void forceRedraw();
void setupScreen();
void setScreenState(ScreenState state);
void showTemporaryScreen(ScreenState state);
void screenPinPreview(const String &typedPin);
void setRemainingTime(unsigned long ms);
void drawScreen();


#endif
