#include <Arduino.h>
#include <SCServo.h>
#include <Bluepad32.h>

#define amplitude 0.5
#define frequency 0.8
#define phaseOffset PI
#define segments 2

SMS_STS servos;
GamepadPtr gamepad;

int DegreesToSteps(float degrees) {
    return 2048 + (int)(degrees * 4096.0f / 360.0f);
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

void setup() {
    Serial.begin(115200);
    Serial2.begin(1000000, SERIAL_8N1, 22, 21);
    delay(500);
    servos.pSerial = &Serial2;
    
    BP32.setup(&OnConnectedGamepad, &OnDisconnectedGamepad);
}

void loop() {
    BP32.update();

    if (gamepad && gamepad->isConnected()) {
        int leftY = -(gamepad->axisY());
        int leftX = gamepad->axisX();
        
        float turn = mapFloat(leftX, -512, 512, -0.2f, 0.2f);

        if (leftY > 150 || leftX > 50 || leftX < -50) {
            float t = millis() / 1000.0;

            for (int i = 0; i < segments; i++) {
                float angle = amplitude * sin(2 * PI * frequency * t + i * phaseOffset) + turn;
                float degrees = angle * 180.0 / 3.14159;
                int steps = DegreesToSteps(degrees);
                servos.WritePosEx(i, steps, 0, 0);
            }
        }

        Serial.print(leftX);
        Serial.print(", ");
        Serial.println(turn);
    }

    delay(20);
}