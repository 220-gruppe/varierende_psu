#ifndef AUTH_H
#define AUTH_H

#include <Arduino.h>
#include "database.h"

enum class AuthState
{
    Ready,
    WaitingForChip,
    WaitingForPin,
    LoggedIn
};

void setupAuth();
bool beginPendingUserCreation(const String &newUser, const String &newPassword);
bool completePendingUserCreation(const String &newUid);
bool loadUserByUID(const String &searchUid);
bool authUser(const String &enteredPin);
bool authTimedOut(unsigned long timeoutMs);
AuthState authState();
String authStateName();
String currentUserName();
String currentUserUID();
void resetAuthTimer();
void logout();

#endif
