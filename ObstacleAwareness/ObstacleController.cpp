#include "ObstacleController.h"

ObstacleController::ObstacleController(SideDetector& leftDetector, SideDetector& rightDetector, 
                                       Vibrator& leftVibrator, Vibrator& rightVibrator)
    : _leftDetector(leftDetector), _rightDetector(rightDetector),
      _leftVibrator(leftVibrator), _rightVibrator(rightVibrator),
      _leftLastClearedTime(0), _rightLastClearedTime(0),
      _leftAlertState(false), _rightAlertState(false) {}

void ObstacleController::init() {
    _leftDetector.init();
    _rightDetector.init();
    _leftVibrator.init();
    _rightVibrator.init();
}

void ObstacleController::update() {
    bool leftDetectedRaw = _leftDetector.isDetected();
    bool rightDetectedRaw = _rightDetector.isDetected();

    unsigned long now = millis();

    // Left state machine
    if (leftDetectedRaw) {
        _leftAlertState = true;
        _leftLastClearedTime = 0;
    } else {
        if (_leftAlertState) {
            if (_leftLastClearedTime == 0) {
                _leftLastClearedTime = now; // Start cooldown
            } else if (now - _leftLastClearedTime >= COOLDOWN_MS) {
                _leftAlertState = false;
                _leftLastClearedTime = 0;
            }
        }
    }

    // Right state machine
    if (rightDetectedRaw) {
        _rightAlertState = true;
        _rightLastClearedTime = 0;
    } else {
        if (_rightAlertState) {
            if (_rightLastClearedTime == 0) {
                _rightLastClearedTime = now; // Start cooldown
            } else if (now - _rightLastClearedTime >= COOLDOWN_MS) {
                _rightAlertState = false;
                _rightLastClearedTime = 0;
            }
        }
    }

    // Apply decision logic
    if (_leftAlertState && _rightAlertState) {
        _leftVibrator.activate();
        _rightVibrator.activate();
    } else if (_leftAlertState && !_rightAlertState) {
        _leftVibrator.activate();
        _rightVibrator.deactivate();
    } else if (!_leftAlertState && _rightAlertState) {
        _leftVibrator.deactivate();
        _rightVibrator.activate();
    } else {
        _leftVibrator.deactivate();
        _rightVibrator.deactivate();
    }
}
