#ifndef VIBRATOR_H
#define VIBRATOR_H

#include <Arduino.h>

class Vibrator {
public:
    Vibrator(const char* name, uint8_t pin);
    void init();
    void activate();
    void deactivate();
    void setIntensity(uint8_t intensity); // 0-255

private:
    const char* _name;
    uint8_t _pin;
    bool _isOn;
};

#endif
