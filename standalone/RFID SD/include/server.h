#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>
#include <WiFi.h>
#include <variabler.h>

WebServer server(80);

void createFile();

extern WebServer server;
extern String tempNavn;
extern String tempPin;
extern bool waitforChip;
extern bool manglerPin;
extern String indtastet;

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

void handleRoot()
{
  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><meta charset='UTF-8'>";
  html += STYLE;

  // java, reloader siden
  html += "<script>";
  html += "let currentStatus = '" + String(isLoggedIn ? "LOGGED_IN" : (manglerPin ? "WAITING_FOR_PIN" : (waitforChip ? "WAITING_FOR_CHIP" : "READY"))) + "';";
  html += "setInterval(function() {";
  html += "  fetch('/checkStatus').then(response => response.text()).then(data => {";
  html += "    if (data !== currentStatus) { location.reload(); }";
  html += "  });";
  html += "}, 1000);";
  html += "</script></head><body><div class='card'>";

  if (isLoggedIn)
  {
    html += "<h1>Spider-feet</h1><hr>";
    html += "<h2>Velkommen, " + workerID + "</h2>";
    html += "<div class='status' style='background:#d4edda; color:#155724;'>Systemet er klar til svejsning</div>";
    html += "<p>Data logges automatisk under arbejde.</p>";
    html += "<br><br>";
    html += "<a href='/logout' class='btn' style='background:#dc3545;'>Log ud</a>";
  }
  else
  {
    html += "<h1>Spider-feet</h1><hr>";

    if (manglerPin)
    {
      html += "<div id='pinInput'>";
      html += "<div class='status'>Chip OK: " + workerID + "</div>";
      html += "<p>Indtast din 4-cifrede PIN:</p>";
      html += "<form action='/verificerPin' method='POST'>";
      html += "  <input type='password' name='indtastetPin' pattern='[0-9]*' inputmode='numeric' maxlength='4' required autofocus><br>";
      html += "  <input type='submit' class='btn btn-green' value='Log ind'>";
      html += "</form></div>";
    }
    else if (waitforChip)
    {
      html += "<div class='status' style='color:#e67e22;'>VENTER PÅ NY CHIP...</div>";
    }
    else
    {
      html += "<div class='status' style='color:#27ae60;'>Scan chip for at starte</div>";
      html += "<br><a href='/opret' class='btn'>Opret ny bruger</a>";
      html += "<br><a href='/reset' class='btn' style='background:#FF0000;'>Slet logins(DEBUG)</a>";
    }
  }

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

void handleVerificerPin()
{
  if (server.hasArg("indtastetPin"))
  {
    indtastet = server.arg("indtastetPin");

    if (indtastet == korrektPin)
    {
      manglerPin = false;
      isLoggedIn = true;

      server.sendHeader("Location", "/");
      server.send(303);
    }
    else
    {
      server.send(200, "text/html", "<h1>Forkert kode!</h1><a href='/'>Prøv igen</a>");
    }
  }
}

void handleCheckStatus()
{
  if (isLoggedIn)
  {
    server.send(200, "text/plain", "LOGGED_IN");
  }
  else if (manglerPin)
  {
    server.send(200, "text/plain", "WAITING_FOR_PIN");
  }
  else if (waitforChip)
  {
    server.send(200, "text/plain", "WAITING_FOR_CHIP");
  }
  else
  {
    server.send(200, "text/plain", "READY");
  }
}

void handleLogout()
{
  isLoggedIn = false;
  workerID = "";
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleResetFile()
{
  if (SD.exists("/logins.csv"))
  {
    SD.remove("/logins.csv");
    createFile();
  }
  Serial.println("logins slettet");
  server.sendHeader("Location", "/");
  server.send(303);
}

#endif