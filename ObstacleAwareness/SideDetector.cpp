#include "SideDetector.h"

SideDetector::SideDetector(const char* name, PirSensor& pir, UltrasonicSensor& ultrasonic)
    : _name(name), _pir(pir), _ultrasonic(ultrasonic), _lastDetected(false), _lastPrintTime(0) {}

void SideDetector::init() {
    _pir.init();
    _ultrasonic.init();
}

bool SideDetector::isDetected() {
    bool pirTriggered = _pir.isTriggered();
    float distance = _ultrasonic.getDistanceCm();

    // Sensor Fusion: Obstacle if PIR triggers OR distance is below threshold
    bool detected = pirTriggered || (distance < DISTANCE_THRESHOLD_CM);

    unsigned long now = millis();
    // Log on state change OR every 1000ms for telemetry visibility
    if (detected != _lastDetected || (now - _lastPrintTime > 1000)) {
        Serial.print(F("SENSOR [")); Serial.print(_name); Serial.print(F("] "));
        Serial.print(F("PIR: ")); Serial.print(pirTriggered ? "MOTION   " : "NO_MOTION");
        Serial.print(F(" | Dist: ")); Serial.print(distance);
        Serial.print(F(" cm -> State: "));
        Serial.println(detected ? F("DETECTED") : F("CLEAR"));
        
        _lastDetected = detected;
        _lastPrintTime = now;
    }

    return detected;
}
