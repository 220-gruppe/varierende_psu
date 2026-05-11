#include "interface_state.h"

#include <Arduino.h>

#include "auth.h"
#include "numpad.h"
#include "rfid.h"
#include "screen.h"

namespace
{
constexpr unsigned long INACTIVITY_TIMEOUT_MS = 30000;

void showUnknownChip()
{
    setScreenState(ScreenState::Ready);
    showTemporaryScreen(ScreenState::UnknownChip);
}

void handleInactivity()
{
    if (!authTimedOut(INACTIVITY_TIMEOUT_MS))
    {
        return;
    }

    logout();
    resetNumpad();
    setScreenState(ScreenState::Ready);
    showTemporaryScreen(ScreenState::InactiveLogout);
    Serial.println("logget ud pga inaktivitet");
}

void handleWaitingForChipState(const String &scannedUID)
{
    setScreenState(ScreenState::ScanNewChip);

    if (scannedUID.length() == 0)
    {
        return;
    }

    if (completePendingUserCreation(scannedUID))
    {
        resetNumpad();
        setScreenState(ScreenState::Ready);
        Serial.println("Ny bruger oprettet");
        return;
    }

    showUnknownChip();
}

void handleWaitingForPinState()
{
    setScreenState(ScreenState::EnterPin);
    numpadLogik();

    if (!isUserDone())
    {
        return;
    }

    if (authUser(getTyped()))
    {
        setScreenState(ScreenState::LoggedIn);
        resetNumpad();
        Serial.println("ADGANG GODKENDT");
        return;
    }

    setScreenState(ScreenState::EnterPin);
    showTemporaryScreen(ScreenState::WrongPin);
    resetNumpad();
    Serial.println("FORKERT KODE!");
}

void handleLoggedInState()
{
    setScreenState(ScreenState::LoggedIn);
    handleInactivity();
}

void handleReadyState(const String &scannedUID)
{
    if (scannedUID.length() == 0)
    {
        setScreenState(ScreenState::Ready);
        return;
    }

    if (loadUserByUID(scannedUID))
    {
        resetNumpad();
        setScreenState(ScreenState::EnterPin);
        return;
    }

    showUnknownChip();
}
}

void processInterfaceState()
{
    String scannedUID = scanUID();

    switch (authState())
    {
    case AuthState::WaitingForChip:
        handleWaitingForChipState(scannedUID);
        break;
    case AuthState::WaitingForPin:
        handleWaitingForPinState();
        break;
    case AuthState::LoggedIn:
        handleLoggedInState();
        break;
    case AuthState::Ready:
    default:
        handleReadyState(scannedUID);
        break;
    }

    drawScreen();
}
