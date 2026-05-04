#include "UltrasonicSensor.h"

UltrasonicSensor::UltrasonicSensor(uint8_t txPinArduino, uint8_t rxPinArduino)
    : _txPin(txPinArduino), _rxPin(rxPinArduino), _serial(rxPinArduino, txPinArduino) {}

void UltrasonicSensor::init() {
    _serial.begin(9600);
}

float UltrasonicSensor::getDistanceCm() {
    _serial.listen();
    delay(5);

    while (_serial.available()) {
        _serial.read();
    }

    _serial.write(0x55);

    uint8_t bytes[10];
    int count = 0;
    unsigned long start = millis();
    while (millis() - start < 220) {
        while (_serial.available()) {
            uint8_t b = (uint8_t) _serial.read();
            if (count < 10) {
                bytes[count] = b;
            }
            count++;
        }
    }

    if (count == 0) {
        Serial.println("UART timeout: no bytes");
        return 999.0;
    }

    Serial.print("UART raw bytes: ");
    int maxPrint = count < 10 ? count : 10;
    for (int i = 0; i < maxPrint; i++) {
        if (bytes[i] < 0x10) {
            Serial.print('0');
        }
        Serial.print(bytes[i], HEX);
        Serial.print(' ');
    }
    Serial.print("(count=");
    Serial.print(count);
    Serial.println(')');

    // Empirical SR04M-2 UART frames observed here are 3 bytes with many 0x00 paddings.
    // The most reliable signal in logs is the last non-zero byte after 0x55 trigger.
    int lastNonZero = -1;
    for (int i = maxPrint - 1; i >= 0; i--) {
        if (bytes[i] != 0x00) {
            lastNonZero = bytes[i];
            break;
        }
    }

    if (lastNonZero < 0) {
        Serial.println("UART decode: only zero bytes");
        return 999.0;
    }

    float cm = lastNonZero / 10.0f;
    Serial.print("UART decoded distance: ");
    Serial.print(cm);
    Serial.println(" cm");
    return cm;
}
