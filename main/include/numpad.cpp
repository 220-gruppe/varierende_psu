#include "numpad.h"

void setupNumpad(){
    if (!numpad.begin(0x5A, &Wire))
    {
        Serial.println("STOP! MPR121 ikke fundet.");
        while (1)
            delay(10);
    }
    Serial.println("Den er fundet makker");
    numpad.setAutoconfig(true);
}
void numpadLogik()
{
    uint16_t talTrykket = numpad.touched();
    if (talTrykket > 0)
    {
        int aktivtBit = -1;
        for (uint8_t i = 0; i < 12; i++)
        {
            if (talTrykket & _BV(i))
            {
                aktivtBit = i;
                break;
            }
        }

        if (aktivtBit != -1)
        {
            String tal = "";
            bool slet = false;
            bool enter = false;

            switch (aktivtBit)
            {
            case 7:
                tal = "0";
                break;
            case 0:
                tal = "1";
                break;
            case 4:
                tal = "2";
                break;
            case 8:
                tal = "3";
                break;
            case 1:
                tal = "4";
                break;
            case 5:
                tal = "5";
                break;
            case 9:
                tal = "6";
                break;
            case 2:
                tal = "7";
                break;
            case 6:
                tal = "8";
                break;
            case 10:
                tal = "9";
                break;
            case 3:
                slet = true;
                break; // SLET
            case 11:
                enter = true;
                break; // ENTER
            }

            if (tal != "")
            {
                indtastet += tal;
                tastet += "*";
                Serial.print(tal + " ");
            }

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
    }
}