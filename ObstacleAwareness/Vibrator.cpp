#include "Vibrator.h"

Vibrator::Vibrator(const char* name, uint8_t pin) : _name(name), _pin(pin), _isOn(false) {}

void Vibrator::init() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void Vibrator::activate() {
    if (!_isOn) {
        Serial.print(F("ACTUATOR [")); Serial.print(_name); Serial.println(F("] Vibrator: ON"));
        digitalWrite(_pin, HIGH);
        _isOn = true;
    }
}

void Vibrator::deactivate() {
    if (_isOn) {
        Serial.print(F("ACTUATOR [")); Serial.print(_name); Serial.println(F("] Vibrator: OFF"));
        digitalWrite(_pin, LOW);
        _isOn = false;
    }
}

void Vibrator::setIntensity(uint8_t intensity) {
    if (!_isOn || intensity > 1) { // basic logging change
        Serial.print(F("ACTUATOR [")); Serial.print(_name); Serial.print(F("] Vibrator Intensity: ")); Serial.println(intensity);
    }
    analogWrite(_pin, intensity);
    _isOn = (intensity > 0);
}
