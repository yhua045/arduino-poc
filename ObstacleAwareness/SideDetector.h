#ifndef SIDE_DETECTOR_H
#define SIDE_DETECTOR_H

#include "PirSensor.h"
#include "UltrasonicSensor.h"
#include "Config.h"

class SideDetector {
public:
    SideDetector(const char* name, PirSensor& pir, UltrasonicSensor& ultrasonic);
    void init();
    bool isDetected();

private:
    const char* _name;
    PirSensor& _pir;
    UltrasonicSensor& _ultrasonic;
    bool _lastDetected;
    unsigned long _lastPrintTime;
};

#endif
