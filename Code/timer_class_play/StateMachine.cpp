#include "StateMachine.h"

StateMachine::StateMachine() 
    : _currentState(IDLE) {}

void StateMachine::begin() {
    _currentState = IDLE;
    _timer.begin(120); // Set initial time (120 seconds)
}

void StateMachine::handleButtonPress() {
    switch (_currentState) {
        case IDLE:
            _timer.startPreCountdown(); // Start 3-2-1 countdown before the timer starts
            _currentState = PRE_COUNTDOWN;
            break;
        case PRE_COUNTDOWN:
            // Let the countdown finish; TimerController handles countdown
            break;
        case RUNNING:
            _timer.pause();
            _currentState = PAUSED;
            break;
        case PAUSED:
            _timer.start();
            _currentState = RUNNING;
            break;
    }
}

void StateMachine::update() {
    _timer.update();  // Keep updating the timer

    switch (_currentState) {
        case IDLE:
            // Timer not active
            break;
        case PRE_COUNTDOWN:
            if (!_timer.isPreCountdownActive()) {
                _currentState = RUNNING;
                _timer.start();
            }
            break;
        case RUNNING:
            if (_timer.getCurrentTime() == 0) {
                _currentState = IDLE;  // Timer reached 0
                _timer.reset();         // Reset the timer for next round
            }
            break;
        case PAUSED:
            // Timer is paused, show some UI or wait for a button press
            break;
    }
}
