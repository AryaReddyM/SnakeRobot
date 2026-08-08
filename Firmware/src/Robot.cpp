#include <Arduino.h>
#include <SCServo.h>
#include <Bluepad32.h>

#define frequency 0.6
#define segments 4
#define phaseOffset (2 * PI / segments)

#define servoMax 1758
#define servoMin 1092

SMS_STS servos;
GamepadPtr gamepad;

int servoCenters[segments];

int DegreesToSteps(float degrees) {
    return (int)(degrees * 4096.0f / 360.0f);
}

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
        servoCenters[i] = (servoMax + servoMin) / 2;
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

            float turn = mapFloat(leftX, -512, 512, -0.2f, 0.2f);\
            float turnSteps = DegreesToSteps(turn * 180.0 / PI);

            if (leftY > 150 || leftX > 350 || leftX < -350) {
                float t = millis() / 1000.0;

                for (int i = 0; i < segments; i++) {
                    float wave = sin(2 * PI * frequency * t + i * phaseOffset);
                    float waveSteps = mapFloat(wave, -1.0f, 1.0f, (float)servoMin, (float)servoMax);

                    int target = constrain(waveSteps, servoMin, servoMax);

                    servos.WritePosEx(i, target, 0, 0);
                }
                
                Serial.println(servos.ReadPos(0));
            }
        }
    }
}