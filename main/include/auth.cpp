#include "auth.h"

String uid;
String user;
String password;
bool authStatus = false;

void setupAuth(){
   DB("users", "UID,USER,PASSWORD");
}

bool createUser()
{
    write((uid + "," + user, "," + password))     
}

bool findUID(String &user){
    if(!uid){
        return false;
    }
    
    search(uid, user);

    return true;
}


bool authUser()
{
    if (findUID()) // matcher fundet UID med data
    {
        manglerPin = true;
        Serial.println("Chip fundet. Venter på kode...");
    }
    else
    {
        ikkeKodet = true;
        Serial.println("CHIP FINDES IKKE!");
    }
}