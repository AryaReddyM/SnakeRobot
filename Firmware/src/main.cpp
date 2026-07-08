#include <Arduino.h>
#include <Bluepad32.h>
#include <SCServo.h>

SMS_STS servos;

int degreesToSteps(float degrees) {
    return (round(degrees * 4096.0f / 360.0f));
}

void setup() {
    Serial.begin(115200);
    Serial2.begin(1000000);

    delay(500);
    servos.pSerial = &Serial2;

    Serial.println("Initial Print");

    servos.EnableTorque(1, 1);
}

void loop() {
    servos.WritePosEx(1, degreesToSteps(25), 1500, 50);
    delay(3000);

    servos.WritePosEx(1, degreesToSteps(0), 1500, 50);
    delay(3000);
}