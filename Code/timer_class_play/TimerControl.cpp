#include "TimerControl.h"

TimerControl::TimerControl()
    : totalSeconds(0), remainingSeconds(0), running(false) {}

void TimerControl::setTime(int minutes, int seconds) {
    totalSeconds = (minutes * 60) + seconds;
    remainingSeconds = totalSeconds;
    running = false;
}

void TimerControl::start() {
    if (remainingSeconds > 0)
        running = true;
}

void TimerControl::pause() {
    running = false;
}

void TimerControl::reset() {
    remainingSeconds = totalSeconds;
    running = false;
}

void TimerControl::tick() {
    if (running && remainingSeconds > 0) {
        remainingSeconds--;
        if (remainingSeconds == 0) {
            running = false;
        }
    }
}

bool TimerControl::isRunning() const {
    return running;
}

bool TimerControl::isFinished() const {
    return remainingSeconds == 0;
}

int TimerControl::getTimeLeft() const {
    return remainingSeconds;
}

void TimerControl::getFormattedTime(char* buffer, int bufferSize) const {
    int mins = remainingSeconds / 60;
    int secs = remainingSeconds % 60;
    snprintf(buffer, bufferSize, "%02d:%02d", mins, secs);
}
