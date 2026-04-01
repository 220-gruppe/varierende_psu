#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>
#include <WiFi.h>
#include <variabler.h>

extern WebServer server;
extern String tempNavn;
extern String tempPin;
extern bool waitforChip;

const String STYLE = "<style>"
                     "*{box-sizing: border-box;}"
                     "body{font-family:sans-serif; text-align:center; padding:15px; background:#f0f2f5; margin:0;}"
                     ".card{background:white; padding:20px; border-radius:15px; shadow:0 4px 8px rgba(0,0,0,0.1); "
                     "max-width:350px; margin:20px auto; width: 95%;}" 
                     "h1{color:#2c3e50; font-size: 1.8em;}"
                     ".status{font-size:1.1em; color:#e67e22; margin:15px 0; font-weight:bold; padding:10px; border-radius:8px; background:#fff3e0;}"
                     "input{margin:10px 0; padding:12px; width:100%; border:1px solid #ccc; border-radius:8px; font-size:16px;}"
                     ".btn{background:#3498db; color:white; border:none; padding:15px; width:100%; border-radius:8px; "
                     "font-weight:bold; cursor:pointer; text-decoration:none; display:inline-block; font-size:18px; margin-top:10px;}"
                     ".btn-green{background:#27ae60;}"
                     "hr{border:0; border-top:1px solid #eee; margin:20px 0;}"
                     "</style>";

void handleRoot()
{
  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><meta charset='UTF-8'>";
  html += STYLE;
  html += "</head><body>";
  html += "<div class='card'>";
  html += "<h1>Spider-feet</h1>";
  html += "<hr>";

  if (waitforChip)
  {
    html += "<div class='status'>VENTER PÅ TAG...<br><small>(Navn: " + tempNavn + ")</small></div>";
  }
  else
  {
    html += "<div class='status' style='color:#27ae60;'>System Klar</div>";
  }

  html += "<p>Scan venligst din chip på maskinen for at logge data.</p>";
  html += "<br>";

  // Knap til at gå til opret-siden
  html += "<a href='/opret' class='btn'>Opret ny bruger</a>";
  html += "</div></body></html>";

  server.send(200, "text/html", html);
}

void handleOpretSide()
{
  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><meta charset='UTF-8'>";
  html += STYLE;
  html += "</head><body>";
  html += "<div class='card'>";
  html += "<h1>Ny Bruger</h1>";
  html += "<form action='/gemLogin' method='POST'>";
  html += "  <input type='text' name='brugerNavn' placeholder='Navn på medarbejder' required><br>";
  html += "  <input type='number' name='pinKode' placeholder='4-cifret PIN' required><br>";
  html += "  <input type='submit' class='btn btn-green' value='Start oprettelse'>";
  html += "</form>";
  html += "<a href='/' style='display:block; margin-top:15px; color:#7f8c8d;'>Annuller</a>";
  html += "</div></body></html>";

  server.send(200, "text/html", html);
}

void handleGemLogin()
{
  if (server.hasArg("brugerNavn") && server.hasArg("pinKode"))
  {
    tempNavn = server.arg("brugerNavn");
    tempPin = server.arg("pinKode");
    waitforChip = true;

    server.sendHeader("Location", "/");
    server.send(303);
  }
}

#endif