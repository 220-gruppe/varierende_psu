#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>
#include <WiFi.h>
#include <Arduino.h>

WebServer server(80);

const char *SSID = "Spider-feet";
const char *PASSWORD = "";

String tempNavn;
String tempPin;
bool waitforChip;
bool manglerPin;
String indtastet;

const String STYLE = "<style>"
                     "*{box-sizing: border-box;}"
                     "body{overflow: hidden; width: 100%; height: 100%; touch-action: manipulation; user-select: none; font-family:sans-serif; text-align:center; padding:15px; background:#0f454e; margin:0;}"
                     ".card{background:white; padding:20px; border-radius:15px; shadow:0 4px 8px rgba(0,0,0,0.1); "
                     "max-width:350px; margin:20px auto; width: 95%;}"
                     "h1{color:#050854; font-size: 1.8em;}"
                     ".status{font-size:1.1em; color:#e67e22; margin:15px 0; font-weight:bold; padding:10px; border-radius:8px; background:#fff3e0;}"
                     "input{margin:10px 0; padding:12px; width:100%; border:1px solid #ccc; border-radius:8px; font-size:16px;}"
                     ".btn{background:#3498db; color:white; border:none; padding:15px; width:100%; border-radius:8px; "
                     "font-weight:bold; cursor:pointer; text-decoration:none; display:inline-block; font-size:18px; margin-top:10px;}"
                     ".btn-green{background:#27ae60;}"
                     "hr{border:0; border-top:1px solid #eee; margin:20px 0;}"
                     "</style>";

void handleRoot();
void handleOpretSide();
void handleGemLogin();
void handleVerificerPin();
void handleCheckStatus();
void handleLogout();
void handleResetFile();

#endif