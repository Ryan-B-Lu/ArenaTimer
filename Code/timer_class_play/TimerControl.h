#ifndef TIMER_CONTROL_H
#define TIMER_CONTROL_H

class TimerControl {
public:
    TimerControl();

    void setTime(int minutes, int seconds);
    void start();
    void pause();
    void reset();
    void tick(); // Call this once per second when running

    bool isRunning() const;
    bool isFinished() const;
    int getTimeLeft() const;
    void getFormattedTime(char* buffer, int bufferSize) const;

private:
    int totalSeconds;
    int remainingSeconds;
    bool running;
};

#endif
