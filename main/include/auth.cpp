#include "auth.h"

namespace
{
    String uid = "";
    String user = "";
    String password = "";
    String pendingUser = "";
    String pendingPassword = "";
    unsigned long lastAuthActivity = 0;
    AuthState currentState = AuthState::Ready;

    constexpr char USERS_FILE[] = "/users.csv";
    constexpr char USERS_COLUMNS[] = "UID,USER,PASSWORD";

    void clearLoadedUser()
    {
        uid = "";
        user = "";
        password = "";
    }

    void clearPendingUser()
    {
        pendingUser = "";
        pendingPassword = "";
    }

    void setAuthState(AuthState state)
    {
        currentState = state;
    }

    String stateName(AuthState state)
    {
        switch (state)
        {
        case AuthState::WaitingForChip:
            return "WAITING_FOR_CHIP";
        case AuthState::WaitingForPin:
            return "WAITING_FOR_PIN";
        case AuthState::LoggedIn:
            return "LOGGED_IN";
        case AuthState::Ready:
        default:
            return "READY";
        }
    }

    bool parseUserLine(const String &userLine)
    {
        int firstComma = userLine.indexOf(',');
        int secondComma = userLine.indexOf(',', firstComma + 1);

        if (firstComma == -1 || secondComma == -1)
        {
            Serial.println("Brugerdata er ugyldige");
            return false;
        }

        uid = userLine.substring(0, firstComma);
        user = userLine.substring(firstComma + 1, secondComma);
        password = userLine.substring(secondComma + 1);
        password.trim();
        return true;
    }

    bool writeUserRecord(const String &newUid, const String &newUser, const String &newPassword)
    {
        if (newUid.length() == 0 || newUser.length() == 0 || newPassword.length() == 0)
        {
            return false;
        }

        uid = newUid;
        user = newUser;
        password = newPassword;

        if (!ensureCsvFile(USERS_FILE, USERS_COLUMNS))
        {
            return false;
        }

        return appendLineToFile(USERS_FILE, uid + "," + user + "," + password);
    }

    bool findUserLineByUid(const String &searchUid, String &userLine)
    {
        if (searchUid.length() == 0)
        {
            return false;
        }

        if (!ensureCsvFile(USERS_FILE, USERS_COLUMNS))
        {
            return false;
        }

        return findLineByFirstCsvField(USERS_FILE, searchUid, userLine);
    }
}

void setupAuth()
{
    ensureCsvFile(USERS_FILE, USERS_COLUMNS);
}

bool beginPendingUserCreation(const String &newUser, const String &newPassword)
{
    if (newUser.length() == 0 || newPassword.length() == 0)
    {
        return false;
    }

    clearLoadedUser();
    pendingUser = newUser;
    pendingPassword = newPassword;
    setAuthState(AuthState::WaitingForChip);
    return true;
}

bool completePendingUserCreation(const String &newUid)
{
    if (currentState != AuthState::WaitingForChip)
    {
        return false;
    }

    if (!writeUserRecord(newUid, pendingUser, pendingPassword))
    {
        return false;
    }

    clearLoadedUser();
    clearPendingUser();
    setAuthState(AuthState::Ready);
    return true;
}

bool loadUserByUID(const String &searchUid)
{
    String userLine;

    if (!findUserLineByUid(searchUid, userLine))
    {
        clearLoadedUser();
        setAuthState(AuthState::Ready);
        return false;
    }

    if (!parseUserLine(userLine))
    {
        clearLoadedUser();
        setAuthState(AuthState::Ready);
        return false;
    }

    setAuthState(AuthState::WaitingForPin);
    return true;
}

bool authUser(const String &enteredPin)
{
    if (password.length() == 0)
    {
        return false;
    }

    if (enteredPin != password)
    {
        setAuthState(AuthState::WaitingForPin);
        return false;
    }

    setAuthState(AuthState::LoggedIn);
    lastAuthActivity = millis();
    return true;
}

bool authTimedOut(unsigned long timeoutMs)
{
    return currentState == AuthState::LoggedIn &&
           lastAuthActivity != 0 &&
           millis() - lastAuthActivity > timeoutMs;
}

AuthState authState()
{
    return currentState;
}

String authStateName()
{
    return stateName(currentState);
}

String currentUserName()
{
    return user;
}

String currentUserUID()
{
    return uid;
}

void resetAuthTimer()
{
    lastAuthActivity = millis();
}

void logout()
{
    clearLoadedUser();
    clearPendingUser();
    lastAuthActivity = 0;
    setAuthState(AuthState::Ready);
}
