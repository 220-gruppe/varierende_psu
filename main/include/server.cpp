#include "server.h"
#include "auth.h"
#include "database.h"
#include <WebServer.h>
#include <WiFi.h>

namespace
{
WebServer server(80);
constexpr char SERVER_SSID[] = "Spider-feet";
constexpr char SERVER_PASSWORD[] = "";

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
}

static void handleRoot();
static void handleOpretSide();
static void handleGemLogin();
static void handleVerificerPin();
static void handleCheckStatus();
static void handleLogout();
static void handleResetFile();

void setupServer()
{
    WiFi.softAP(SERVER_SSID, SERVER_PASSWORD);
    IPAddress ip = WiFi.softAPIP();

    server.on("/gemLogin", HTTP_POST, handleGemLogin);
    server.on("/", handleRoot);
    server.on("/verificerPin", HTTP_POST, handleVerificerPin);
    server.on("/opret", handleOpretSide);
    server.on("/checkStatus", handleCheckStatus);
    server.on("/reset", handleResetFile);
    server.on("/logout", handleLogout);
    server.begin();

    Serial.print("Server er oppe paa: ");
    Serial.println(ip);
}

static void handleRoot()
{
    String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><meta charset='UTF-8'>";
    html += STYLE;

    html += "<script>";
    html += "let currentStatus = '" + authStateName() + "';";
    html += "setInterval(function() {";
    html += "  fetch('/checkStatus').then(response => response.text()).then(data => {";
    html += "    if (data !== currentStatus) { location.reload(); }";
    html += "  });";
    html += "}, 1000);";
    html += "</script></head><body><div class='card'>";

    if (authState() == AuthState::LoggedIn)
    {
        html += "<h1>Spider-feet</h1><hr>";
        html += "<h2>Velkommen, " + currentUserName() + "</h2>";
        html += "<div class='status' style='background:#d4edda; color:#155724;'>Systemet er klar til svejsning...</div>";
        html += "<p>Data logges automatisk under arbejde.</p>";
        html += "<br><br>";
        html += "<a href='/logout' class='btn' style='background:#dc3545;'>Log ud</a>";
    }
    else
    {
        html += "<h1>Spider-feet</h1><hr>";

        if (authState() == AuthState::WaitingForPin)
        {
            html += "<div id='pinInput'>";
            html += "<div class='status'>Chip OK: " + currentUserName() + "</div>";
            html += "<p>Indtast din 4-cifrede PIN:</p>";
            html += "<form action='/verificerPin' method='POST'>";
            html += "  <input type='password' name='indtastetPin' pattern='[0-9]*' inputmode='numeric' maxlength='4' required autofocus><br>";
            html += "  <input type='submit' class='btn btn-green' value='Log ind'>";
            html += "</form></div>";
        }
        else if (authState() == AuthState::WaitingForChip)
        {
            html += "<div class='status' style='color:#e67e22;'>VENTER PAA NY CHIP...</div>";
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

static void handleOpretSide()
{
    String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><meta charset='UTF-8'>";
    html += STYLE;
    html += "</head><body>";
    html += "<div class='card'>";
    html += "<h1>Ny Bruger</h1>";
    html += "<form action='/gemLogin' method='POST'>";
    html += "  <input type='text' name='brugerNavn' placeholder='Navn paa medarbejder' required><br>";
    html += "  <input type='number' name='pinKode' placeholder='4-cifret PIN' required><br>";
    html += "  <input type='submit' class='btn btn-green' value='Start oprettelse'>";
    html += "</form>";
    html += "<a href='/' style='display:block; margin-top:15px; color:#7f8c8d;'>Annuller</a>";
    html += "</div></body></html>";

    server.send(200, "text/html", html);
}

static void handleGemLogin()
{
    if (server.hasArg("brugerNavn") && server.hasArg("pinKode"))
    {
        beginPendingUserCreation(server.arg("brugerNavn"), server.arg("pinKode"));
        server.sendHeader("Location", "/");
        server.send(303);
    }
}

static void handleVerificerPin()
{
    if (server.hasArg("indtastetPin"))
    {
        const String enteredPin = server.arg("indtastetPin");

        if (authUser(enteredPin))
        {
            server.sendHeader("Location", "/");
            server.send(303);
        }
        else
        {
            server.send(200, "text/html", "<h1>Forkert kode!</h1><a href='/'>Proev igen</a>");
        }
    }
}

static void handleCheckStatus()
{
    server.send(200, "text/plain", authStateName());
}

static void handleLogout()
{
    logout();

    server.sendHeader("Location", "/");
    server.send(303);
}

static void handleResetFile()
{
    if (SD.exists("/users.csv"))
    {
        SD.remove("/users.csv");
    }

    ensureCsvFile("/users.csv", "UID,USER,PASSWORD");
    logout();

    Serial.println("users slettet");
    server.sendHeader("Location", "/");
    server.send(303);
}

void processServer()
{
    server.handleClient();
}
