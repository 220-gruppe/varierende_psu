#include "screen.h"

void setupScreen(){
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(0xE6D6);
    tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
    tft.setTextSize(3);
    tft.setSwapBytes(true);
    tft.pushImage(35, 10, 250, 77, logo);
}

void opdaterScreen()
{
    if (isLoggedIn)
        nuStatus = "Logget ind: " + workerID;
    else if (manglerPin)
        nuStatus = "INDTAST PIN:";
    else if (waitforChip)
        nuStatus = "SCAN NY CHIP...";
    else if (ikkeKodet)
    {
        nuStatus = "CHIP FINDES IKKE!";
    }
    else
        nuStatus = "KLAR TIL SCAN";

    if (nuStatus != sidsteStatus)
    {

        tft.fillRect(0, 97, 320, 222, SPIDER_BG);

        if (nuStatus == "CHIP FINDES IKKE!")
        {
            tft.setTextColor(TFT_RED, SPIDER_BG);
            tft.setTextDatum(MC_DATUM);
            tft.drawString(nuStatus, 160, 110, 1);
            delay(2000);
            ikkeKodet = false;
        }
        else
        {
            tft.setTextColor(SPIDER_BLUE, SPIDER_BG);
            tft.setTextDatum(MC_DATUM);
            tft.drawString(nuStatus, 160, 110, 1);
        }
        sidsteStatus = nuStatus;
    }
}