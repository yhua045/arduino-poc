#ifndef OBSTACLE_CONTROLLER_H
#define OBSTACLE_CONTROLLER_H

#include <Arduino.h>
#include "SideDetector.h"
#include "Vibrator.h"
#include "Config.h"

class ObstacleController {
public:
    ObstacleController(SideDetector& leftDetector, SideDetector& rightDetector, 
                       Vibrator& leftVibrator, Vibrator& rightVibrator);
    void init();
    void update();

private:
    SideDetector& _leftDetector;
    SideDetector& _rightDetector;
    Vibrator& _leftVibrator;
    Vibrator& _rightVibrator;

    unsigned long _leftLastClearedTime;
    unsigned long _rightLastClearedTime;

    bool _leftAlertState;
    bool _rightAlertState;
};

#endif
