#ifndef TIMER_CONTROL_H
#define TIMER_CONTROL_H

#pragma once
#include <Arduino.h>
#include "TimerController.h"

// Define states as an enum
enum State {
    IDLE,
    PRE_COUNTDOWN,
    RUNNING,
    PAUSED
};

class StateMachine {
public:
    StateMachine();
    
    void begin();
    void update();
    
    void handleButtonPress(); // to handle button interactions
    
private:
    State _currentState;
    TimerController _timer;
};

#endif