#include "numpad.h"

Adafruit_MPR121 numpad = Adafruit_MPR121();
String typed = "";
bool enter = false; 

void setupNumpad()
{
    if (!numpad.begin(0x5A, &Wire))
    {
        Serial.println("STOP! MPR121 ikke fundet.");
        while (1)
            delay(10);
    }
    Serial.println("Den er fundet makker");
    numpad.setAutoconfig(true);
}

String getTyped()
{
    return typed;
}

void callClear(){
    typed = "";
}

void callEnter(){
    enter = true;
}

void numpadLogik()
{
    uint16_t numpadValue = numpad.touched();
    if (!(numpadValue > 0))
    {
        return;
    }

    int aktivtBit = -1;
    for (uint8_t i = 0; i < 12; i++)
    {
        if (numpadValue & _BV(i))
        {
            aktivtBit = i;
            break;
        }
    }

    if (aktivtBit == -1)
    {
        return;
    }

    String value = "";

    switch (aktivtBit) {
        case 7:
            value = "0";
            break;
        case 0:
            value = "1";
            break;
        case 4:
            value = "2";
            break;
        case 8:
            value = "3";
            break;
        case 1:
            value = "4";
            break;
        case 5:
            value = "5";
            break;
        case 9:
            value = "6";
            break;
        case 2:
            value = "7";
            break;
        case 6:
            value = "8";
            break;
        case 10:
            value = "9";
            break;
        case 3:
            callClear();
            break; // SLET
        case 11:
            callEnter();
            break; // ENTER
        default:
            return;
    }

    typed += value;

    if (slet)
    {
        indtastet = "";
        tastet = "";
        tft.fillRect(0, 120, 320, 50, SPIDER_BG);
        Serial.print("SLET");
    }

    if (indtastet.length() > 4 && indtastet != korrektPin)
    {
        tft.fillRect(0, 120, 320, 50, SPIDER_BG);
        indtastet = "";
        tastet = "";
    }

    if (enter)
    {
        Serial.println("#");
        if (indtastet == korrektPin)
        {
            isLoggedIn = true;
            manglerPin = false;
            indtastet = "";
            tastet = "";
            Serial.println("ADGANG GODKENDT");
            tidStart = millis();
        }
        else
        {
            Serial.println("FORKERT KODE!");
            tft.fillRect(0, 120, 320, 50, SPIDER_BG);
            tft.drawString("FORKERT KODE!", 160, 140, 1);
            delay(1000);
            indtastet = "";
            tastet = "";
            tft.fillRect(0, 120, 320, 50, SPIDER_BG);
        }
    }

    if (manglerPin && !slet)
    {
        tft.setTextColor(TFT_RED, SPIDER_BG);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(tastet, 160, 140, 1);
    }
    delay(250);
}

