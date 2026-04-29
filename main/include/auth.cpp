#include "auth.h"

String uid = "";
String user = "";
String password = "";
bool isAuth = false;

void setupAuth()
{
    DB("users", "UID,USER,PASSWORD");
}

bool createUser()
{

    if (uid.length() == 0 || user.length() == 0 || password.length() == 0)
    {
        return false;
    }

    return databaseWrite(uid + "," + user + "," + password);
}

bool authStatus(){
    return isAuth;
}

void logout(){
    uid = "";
    user = "";
    password = "";
    isAuth = false;
}

bool findUID(String &userLine)
{
    if (uid.length() == 0)
    {
        return false;
    }

    return databaseSearch(uid, userLine);
}

bool authUser(String &username, String userPassword)
{
    uid = scanUID();

    String userLine;
    if (!findUID(userLine))
    {
        isAuth = false;
        Serial.println("CHIP FINDES IKKE!");
        return false;
    }

    int firstComma = userLine.indexOf(',');
    int secondComma = userLine.indexOf(',', firstComma + 1);

    if (firstComma == -1 || secondComma == -1)
    {
        isAuth = false;
        Serial.println("Brugerdata er ugyldige");
        return false;
    }

    uid = userLine.substring(0, firstComma);
    user = userLine.substring(firstComma + 1, secondComma);
    password = userLine.substring(secondComma + 1);
    
    if (password == userPassword){
        username = user;
        return true;
    } 
    return false;
}
