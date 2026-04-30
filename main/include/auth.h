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
bool createUser(const String &newUid, const String &newUser, const String &newPassword);
bool beginPendingUserCreation(const String &newUser, const String &newPassword);
bool completePendingUserCreation(const String &newUid);
bool findUID(const String &searchUid, String &userLine);
bool loadUserByUID(const String &searchUid);
bool authUser(const String &enteredPin);
bool authStatus();
AuthState authState();
String authStateName();
String currentUserName();
void logout();

#endif
