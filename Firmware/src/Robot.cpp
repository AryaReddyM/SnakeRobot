#include <Arduino.h>
#include <SCServo.h>
#include <Bluepad32.h>

#define frequency 0.8
#define segments 6
#define phaseOffset (2 * PI / segments)

#define servoMax 1650
#define servoMin 1250
#define range ((servoMax - servoMin) / 2)

SMS_STS servos;
GamepadPtr gamepad;

float mapFloat(float x, float inMin, float inMax, float outMin, float outMax) {
    return (x - inMin) / (inMax - inMin) * (outMax - outMin) + outMin;
}

void OnConnectedGamepad(GamepadPtr gp) {
    Serial.println("Gamepad connected");
    gamepad = gp;
}

void OnDisconnectedGamepad(GamepadPtr gp) {
    Serial.println("Gamepad disconnected");
    gamepad = nullptr;
}

void calibrateCenters() {
    for (int i = 0; i < segments; i++) {
        servos.WritePosEx(i, (servoMax + servoMin) / 2, 0, 0);
        delay(500);
    }
}

void setup() {
    Serial.begin(115200);
    Serial2.begin(1000000, SERIAL_8N1, 22, 21);
    delay(500);
    servos.pSerial = &Serial2;

    calibrateCenters();

    BP32.setup(&OnConnectedGamepad, &OnDisconnectedGamepad);
}

unsigned long lastUpdate = 0;
unsigned long lastRead = 0;
void loop() {
    BP32.update();

    if (gamepad && gamepad->isConnected()) {
        if (gamepad->a()) {
            calibrateCenters();
        }

        if (millis() - lastUpdate >= 20) {
            lastUpdate = millis();

            int leftY = -(gamepad->axisY());
            int leftX = gamepad->axisX();

            float offset = 0;
            if (abs(leftX) > 250) {
                offset = mapFloat(leftX, -512, 512, -120, 120);
            }

            if (leftY > 150 || leftX > 350 || leftX < -350) {
                float t = millis() / 1000.0;

                for (int i = 0; i < segments; i++) {
                    float wave = sin(2 * PI * frequency * t + i * phaseOffset);

                    float jointOffset = offset * (float)(segments - i) / segments;
                    float amp = range - fabs(jointOffset);
                    int target = (int)((servoMin + servoMax) / 2 + jointOffset + wave * amp);

                    servos.WritePosEx(i, target, 2000, 200);
                }
            }
        }

        if (millis() - lastRead >= 503) {
            lastRead = millis();

            if (millis() - lastUpdate > 5) {
                int pos = servos.ReadPos(0);
                if (pos >= 0) Serial.println(pos);
            }
        }
    }
}