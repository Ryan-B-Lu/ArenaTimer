#include "StateMachine.h"

StateMachine sm;

void setup() {
    Serial.begin(115200);
    sm.begin();
}

void loop() {
    sm.update();
    
    // Simulate button press (you'd use digitalRead or an interrupt in real life)
    if (digitalRead(buttonPin) == HIGH) {
        sm.handleButtonPress(); // Handle button interaction
    }
    
    // Add your LED updates here (show timer value, etc.)
    // Example: update the LED display based on the state of the timer
    if (sm.getCurrentState() == RUNNING) {
        Serial.print("Timer Running: ");
        Serial.println(sm.getTimer().getCurrentTime());
    } else if (sm.getCurrentState() == PAUSED) {
        Serial.println("Timer Paused.");
    }
    
    delay(100); // Add a delay to prevent too much serial output
}



/*#include "TimerControl.h"

TimerControl timer;

void setup() {
    Serial.begin(115200);
    timer.setTime(5, 0); // 5 minutes
    timer.start();
}

unsigned long lastTick = 0;

void loop() {
    unsigned long now = millis();
    
    // Simulate a 1-second tick
    if (now - lastTick >= 1000) {
        lastTick = now;
        timer.tick();

        char timeStr[6];
        timer.getFormattedTime(timeStr, sizeof(timeStr));
        Serial.println(timeStr);

        if (timer.isFinished()) {
            Serial.println("Timer Done!");
        }
    }

    // Do other stuff like check buttons or update display
}*/
