#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>
#include <WiFi.h>
#include <variabler.h>

WebServer server(80);

void handleRoot() {
  String html = "<html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"; // Vigtigt for mobil!
  html += "<meta charset='UTF-8'>";
  
  html += "<style>";
  // Generel styling af siden
  html += "body { font-family: sans-serif; text-align: center; background-color: #f0f2f5; margin: 20px; }";
  
  // Styling af den store knap
  html += ".btn {";
  html += "  display: block;";
  html += "  width: 100%;";            // Fyld hele bredden
  html += "  max-width: 300px;";       // Men ikke mere end 300px
  html += "  margin: 15px auto;";      // Centrer knappen og giv luft
  html += "  padding: 20px;";          // Gør den tyk og stor
  html += "  background-color: #007bff;"; // Blå farve
  html += "  color: white;";           // Hvid tekst
  html += "  text-decoration: none;";  // Fjern understregning fra link
  html += "  font-size: 22px;";        // Stor tekst
  html += "  font-weight: bold;";
  html += "  border-radius: 12px;";    // Bløde hjørner
  html += "  border: none;";
  html += "  box-shadow: 0 4px 6px rgba(0,0,0,0.1);"; // Lidt skygge
  html += "}";

  // En rød variant til f.eks. "Slet" eller "Nulstil"
  html += ".btn-red { background-color: #dc3545; }";
  
  html += "h1 { color: #333; }";
  html += "p { font-size: 18px; color: #666; }";
  html += "</style></head><body>";

  html += "<h1>Svejse System</h1>";
  html += "<div>";
  html += "  <p>Operatør: <b>" + workerID + "</b></p>";
  html += "  <p>Status: <span style='color:green'>FORBUNDET</span></p>";
  html += "</div>";

  // De store knapper
  html += "<a href='/download' class='btn'>Hent Logfil</a>";
  html += "<a href='/reset' class='btn btn-red'>Nulstil Data</a>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

#endif