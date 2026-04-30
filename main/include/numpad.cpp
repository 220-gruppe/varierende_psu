#include "numpad.h"
#include "screen.h"

namespace
{
Adafruit_MPR121 numpad = Adafruit_MPR121();
String typed = "";
bool userDone = false;
}

void setupNumpad()
{
    if (!numpad.begin(0x5A, &Wire))
    {
        Serial.println("STOP! MPR121 ikke fundet.");
        while (1)
        {
            delay(10);
        }
    }

    Serial.println("Den er fundet makker");
    numpad.setAutoconfig(true);
    screenPinPreview(typed);
}

String getTyped()
{
    return typed;
}

bool isUserDone()
{
    return userDone;
}

void resetUserDone()
{
    userDone = false;
}

void clearTyped()
{
    typed = "";
    userDone = false;
    screenPinPreview(typed);
}

void resetNumpad()
{
    clearTyped();
}

void numpadLogik()
{
    uint16_t numpadValue = numpad.touched();
    if (numpadValue == 0)
    {
        return;
    }

    int activeBit = -1;
    for (uint8_t i = 0; i < 12; i++)
    {
        if (numpadValue & _BV(i))
        {
            activeBit = i;
            break;
        }
    }

    if (activeBit == -1)
    {
        return;
    }

    String value = "";
    bool clearRequested = false;
    bool enterRequested = false;

    switch (activeBit)
    {
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
        clearRequested = true;
        break;
    case 11:
        enterRequested = true;
        break;
    default:
        return;
    }

    if (clearRequested)
    {
        clearTyped();
        Serial.print("SLET");
    }
    else if (value.length() > 0)
    {
        if (typed.length() < 4)
        {
            typed += value;
            userDone = false;
            screenPinPreview(typed);
            Serial.print(value + " ");
        }
    }

    if (enterRequested && typed.length() > 0)
    {
        userDone = true;
        Serial.println("#");
    }

    delay(250);
}