#ifndef PIR_SENSOR_H
#define PIR_SENSOR_H

#include <Arduino.h>

class PirSensor {
public:
    PirSensor(uint8_t pin);
    void init();
    bool isTriggered();

private:
    uint8_t _pin;
};

#endif
