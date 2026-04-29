#ifndef SCREEN_H
#define SCREEN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "logo.h"
#include "auth.h"

constexpr uint16_t SPIDER_BG = 0xE6D6;
constexpr uint16_t SPIDER_BLUE = TFT_BLUE;

extern TFT_eSPI tft;
extern String nuStatus;
extern String sidsteStatus;
extern bool ikkeKodet;

void setupScreen();
void opdaterScreen();
void tegnStatus(const String &tekst, uint16_t farve = SPIDER_BLUE, int y = 110);

#endif
