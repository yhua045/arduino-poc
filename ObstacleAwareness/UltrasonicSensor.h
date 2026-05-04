#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class UltrasonicSensor {
public:
    UltrasonicSensor(uint8_t txPinArduino, uint8_t rxPinArduino);
    void init();
    float getDistanceCm();

private:
    uint8_t _txPin;
    uint8_t _rxPin;
    SoftwareSerial _serial;
};

#endif
