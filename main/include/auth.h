#ifndef AUTH_H
#define AUTH_H
 
#include <Arduino.h>
#include "database.h"
#include "rfid.h"

void setupAuth();
bool createUser();
bool authStatus();
bool logout();
bool authStatus();
bool findUID(String &userLine);
bool authUser();

#endif
