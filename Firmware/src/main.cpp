#include <Arduino.h>
#include <SCServo.h>

SMS_STS servos;

int degreesToSteps(float degrees) {
    return 2048 + (int)(degrees * 4096.0f / 360.0f);
}

void setup() {
    Serial.begin(115200);
    Serial2.begin(1000000, SERIAL_8N1, 22, 21);
    delay(500);
    servos.pSerial = &Serial2;
    Serial.println("Servo test starting...");
}

void loop() {
    float amplitude = 0.5;
    float frequency = 0.8;
    float phaseOffset = PI;

    float t = millis() / 1000.0;

    for (int i = 0; i < 2; i++) {
        float angle = amplitude * sin(2 * PI * frequency * t + i * phaseOffset);
        float degrees = angle * 180.0 / 3.14159;
        int steps = degreesToSteps(degrees);
        servos.WritePosEx(i, steps, 0, 0);
    }

    delay(20);

    int pos = servos.ReadPos(1);
    Serial.print("Servo 1 position: ");
    Serial.println(pos);
}