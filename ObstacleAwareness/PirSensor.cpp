#include "PirSensor.h"

PirSensor::PirSensor(uint8_t pin) : _pin(pin) {}

void PirSensor::init() {
    pinMode(_pin, INPUT);
}

bool PirSensor::isTriggered() {
    return digitalRead(_pin) == HIGH;
}
