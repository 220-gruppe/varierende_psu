#include "interface_state.h"

#include <Arduino.h>

#include "auth.h"
#include "numpad.h"
#include "rfid.h"
#include "screen.h"
#include "programs.h"
#include "tempsensor.h"
#include "svejse_logs.h"

namespace
{
    enum class UserInterfaceState
    {
        Idle,
        Measure,
        Menu,
        Svejse,
        Result,
        Data,
        Log,
        Choice
    };

    constexpr unsigned long INACTIVITY_TIMEOUT_MS = 240000;
    constexpr unsigned long TEMP_DISPLAY_MS = 3000;
    unsigned long stateTimer = 0;

    UserInterfaceState currentUserInterfaceState = UserInterfaceState::Idle;

    bool UserInterfaceNumpadOverride(int activeBit)
    {
        resetAuthTimer();
        Serial.println("NUMPAD IS OVERRIDDEN YO" + String(activeBit));
        switch (currentUserInterfaceState)
        {
        case UserInterfaceState::Idle:
            if (activeBit == 3)
            {
                currentUserInterfaceState = UserInterfaceState::Measure;
                setScreenState(ScreenState::Measuring);
                return true;
            }
            break;
        case UserInterfaceState::Menu:
            if (activeBit == 7)
            {
                cycleProgram();
                forceRedraw();
                return true;
            }
            if (activeBit == 11)
            {
                if (confirmProgram())
                {
                    showTemporaryScreen(ScreenState::ProgramConfirmation);
                    startSvejse();
                    currentUserInterfaceState = UserInterfaceState::Svejse;
                }
                return true;
            }
            if (activeBit == 3)
            {
                currentUserInterfaceState = UserInterfaceState::Idle;
                return true;
            }
            break;
        case UserInterfaceState::Result:
            if (activeBit == 11)
            {
                startSvejse();
                currentUserInterfaceState = UserInterfaceState::Svejse;
                return true;
            }
            if (activeBit == 0)
            {
                currentUserInterfaceState = UserInterfaceState::Data;
                return true;
            }
            break;
        case UserInterfaceState::Data:
            if (activeBit == 11)
            {
                currentUserInterfaceState = UserInterfaceState::Log;
                return true;
            }
            break;
        case UserInterfaceState::Choice:
            if (activeBit == 3)
            {
                currentUserInterfaceState = UserInterfaceState::Idle;
                return true;
            }
            if (activeBit == 0)
            {
                logout();
                resetNumpad();
                currentUserInterfaceState = UserInterfaceState::Idle;
                selectedProgram = 0;
                setScreenState(ScreenState::Ready);
                return true;
            }
            break;
        default:
            break;
        }
        return false;
    }

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
            setNumpadOverride(UserInterfaceNumpadOverride);
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

    void handleIdleState()
    {
        setScreenState(ScreenState::Idle);
        Serial.println("CALLING NUMPADLOGIK");
        numpadLogik();
    }

    void handleMeasureState()
    {
        resetAuthTimer();

        if (stateTimer == 0)
        {
            setScreenState(ScreenState::Measuring);
            AVG_TEMP = takeTempMeasurement();
            setScreenState(ScreenState::MeasurementResult);
            stateTimer = millis();
            return;
        }

        if (millis() - stateTimer >= TEMP_DISPLAY_MS)
        {
            stateTimer = 0;
            selectedProgram = 1;
            Serial.println("GOING TO MENU");
            currentUserInterfaceState = UserInterfaceState::Menu;
        }
    }

    void handleMenuState()
    {
        Serial.println("IN MENU STATE");
        setScreenState(ScreenState::ProgramSelection);
        numpadLogik();
    }

    void handleSvejseState()
    {
        unsigned long elapsed = millis() - svejseStartTime;
        unsigned long remaining = (elapsed < svejseDuration) ? (svejseDuration - elapsed) : 0;
        setRemainingTime(remaining);
        setScreenState(ScreenState::SvejseActive);

        if (svejseHandler())
        {
        Serial.println("svejseHandler returned true");
        stopSvejse();
        Serial.println("stopSvejse done");
        heatInput = calculatedOutputEnergy();
        Serial.println("calculatedOutputEnergy done");
        saveSvejsningResult();
        Serial.println("saveSvejsningResult done");
        currentUserInterfaceState = UserInterfaceState::Result;
        Serial.println("Transitioning to Result");
        }
    }

    void handleResultState()
    {
        if (wasApproved())
        {
            setScreenState(ScreenState::SvejsningApproved);
            numpadLogik();
            currentUserInterfaceState = UserInterfaceState::Data;
        }
        else
        {
            setScreenState(ScreenState::SvejsningNotApproved);
            numpadLogik();
        }
    }

    void handleDataState()
    {
        setScreenState(ScreenState::Data);
        numpadLogik();
    }

    void handleLogDataState()
    {
        LogSvejseData();
        showTemporaryScreen(ScreenState::LogData);
        selectedProgram = 0;
        currentUserInterfaceState = UserInterfaceState::Choice;
    }

    void handleChoiceState()
    {
        setScreenState(ScreenState::Choice);
        numpadLogik();
    }

    void handleUserInactivity()
    {
        if (!authTimedOut(INACTIVITY_TIMEOUT_MS))
            return;

        logout();
        resetNumpad();
        currentUserInterfaceState = UserInterfaceState::Idle;
        setScreenState(ScreenState::Ready);
        showTemporaryScreen(ScreenState::InactiveLogout);
    }
}

void processAuthenticationInterfaceState()
{
    String scannedUID = scanUID();

    // // DEBUGGING=====================================
    // Serial.print("AUTH STATE: ");
    // Serial.println(int(authState()));
    // //===============================================

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
}

void processUserInterfaceState()
{
    // // DEBUGGING=====================================
    // Serial.print("USER STATE: ");
    // Serial.println(int(currentUserInterfaceState));
    // //===============================================

    if (authState() != AuthState::LoggedIn)
    {
        return;
    }
    handleUserInactivity();

    switch (currentUserInterfaceState)
    {
    case UserInterfaceState::Idle:
        handleIdleState();
        break;
    case UserInterfaceState::Measure:
        handleMeasureState();
        break;
    case UserInterfaceState::Menu:
        handleMenuState();
        break;
    case UserInterfaceState::Svejse:
        handleSvejseState();
        break;
    case UserInterfaceState::Result:
        handleResultState();
        break;
    case UserInterfaceState::Data:
        handleDataState();
        break;
    case UserInterfaceState::Log:
        handleLogDataState();
        break;
    case UserInterfaceState::Choice:
        handleChoiceState();
        break;
    default:
        handleIdleState();
        break;
    }
}
