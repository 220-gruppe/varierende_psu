#include "Arduino.h"

void setup() {
    Serial.begin(115200);
}

void loop() {
    Serial.print("If this is shows up, something messed up!!");
}