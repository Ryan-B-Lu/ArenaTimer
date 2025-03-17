#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <FastLED.h>
#include <Preferences.h>  // Use NVS for WiFi storage

Preferences preferences;
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

const char* apSSID = "TimerSetup";
const char* apPassword = "12345678";

bool setupMode = false;

// Pin assignments
#define RESET_BTN 27
#define PAUSE_BTN 14
#define START_BTN 12 // OK, boot fails if pulled high, strapping pin
#define TIME_SEL_SW 13
#define LED_BUILTIN 2
#define BLUE_BTN 16
#define RED_BTN 4

// LED pin assignments and setup
#define DIGIT_PIN   5
#define BORDER_PIN  17
#define DIGIT_LED_COUNT 202
#define BORDER_LED_COUNT 140
#define LEFT_BORDER 0
#define RIGHT_BOARDER 70

#define ORANGE CRGB(255, 50, 0)

CRGB digit_leds[DIGIT_LED_COUNT];
CRGB border_leds[BORDER_LED_COUNT];

int countdown_time = 120; // Default to 2 minutes (120 seconds)
int current_time = countdown_time;
bool is_running = false;
bool lastTimeSelState = false;
bool blueReady = false;
bool redReady = false;
bool preCountdownRunning = false;

bool border_toggle = 1;

// Debounce variables for each button
unsigned long lastDebounceTimeStart = 0;
unsigned long lastDebounceTimePause = 0;
unsigned long lastDebounceTimeReset = 0;
unsigned long lastDebounceTimeTimeSel = 0;
unsigned long lastDebounceTimeBlue = 0;
unsigned long lastDebounceTimeRed = 0;

int countdown = 3; //number of seconds until match start

// Debounce delay (in milliseconds)
unsigned long debounceDelay = 50;

const char* html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Countdown Timer</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; background-color: black; color: white; }
        #countdown { font-size: 48px; color: white; }
        button { font-size: 24px; margin: 10px; }
    </style>
</head>
<body>
    <h1>ESP32 Countdown Timer</h1>
    <p id="countdown">02:00</p>
    <button onclick="controlTimer('start')">Start</button>
    <button onclick="controlTimer('pause')">Pause</button>
    <button onclick="controlTimer('reset')">Reset</button>
    <button onclick="toggleTime()">Switch 2min/3min</button>
    <script>
        let webSocket = new WebSocket(`ws://${window.location.hostname}:81/`);
        webSocket.onmessage = function(event) {
            document.getElementById('countdown').textContent = event.data;
        };

        function controlTimer(action) {
            fetch(`/control?cmd=${action}`);
        }

        function toggleTime() {
            fetch('/control?cmd=switch');
        }
    </script>
</body>
</html>
)rawliteral";

void startAPMode() {
    Serial.println("Starting Access Point mode...");
    WiFi.softAP(apSSID, apPassword);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
    setupMode = true;
}

void handleRoot() {
  server.send(200, "text/html", html);
}

void handleRoot_AP() {
    server.send(200, "text/html", "<h1>Enter WiFi Credentials</h1><form action='/setwifi' method='post'>SSID: <input name='ssid'><br>Password: <input name='pass' type='password'><br><input type='submit' value='Save'></form>");
}

void handleControl() {
  String cmd = server.arg("cmd");
  if (cmd == "start") {
    is_running = true;
  } else if (cmd == "pause") {
    is_running = false;
  } else if (cmd == "reset") {
    is_running = false;
    current_time = countdown_time;
    updateClient(); // Update the client immediately
    updateLEDs();
  } else if (cmd == "switch") {
    countdown_time = (countdown_time == 120) ? 180 : 120;
    current_time = countdown_time;
    updateClient(); // Update the client immediately
    updateLEDs();
  }
  server.send(200, "text/plain", "OK");
}

void updateClient() {
  int minutes = current_time / 60;
  int seconds = current_time % 60;
  char buffer[6];
  snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, seconds);
  webSocket.broadcastTXT(buffer);
}

void updateTimer() {
  if (preCountdownRunning) {
    static unsigned long lastCountdownTime = 0;
    unsigned long currentMillis = millis();

    // Countdown every 1 second
    if (currentMillis - lastCountdownTime >= 1000) {
      lastCountdownTime = currentMillis;

      // Update the display for the current countdown number
      setDigit(0, 0, false);
      setDigit(0, 49, false);
      setColon();
      setDigit(countdown, 101, true);
      setDigit(0, 150, true);

      FastLED.show();  // Ensure LEDs update properly this time!

      countdown--;

      // When countdown hits 0, start the main timer
      if (countdown < 0) {
        preCountdownRunning = false;
        countdown = 3;
        current_time = countdown_time;
        is_running = true;

        updateClient();  // Reset to 2:00 (or 3:00)
        updateLEDs();
      }
    }
  }

  // Main timer counting down
  if (is_running && current_time > 0) {
    current_time--;
    
    updateClient();  // Reset to 2:00 (or 3:00)
    updateLEDs();
  }
}

void checkButtons() {
  unsigned long currentMillis = millis();

  // Start button
  if (digitalRead(START_BTN) == LOW && currentMillis - lastDebounceTimeStart > debounceDelay) {
    preCountdownRunning = true; // Initiate the pre-countdown
    // Use ternary operation to set border LEDs to orange or black
      for (int i = 0; i < BORDER_LED_COUNT; i++) {
        border_leds[i] = ORANGE;
      }

      setDigit(0, 0, false);
      setDigit(0, 49, false);
      setColon();
      setDigit(countdown, 101, true);
      setDigit(0, 150, true);

      FastLED.show();
    lastDebounceTimeStart = currentMillis;  // Update debounce time
  }

  // Pause button
  if (digitalRead(PAUSE_BTN) == LOW && currentMillis - lastDebounceTimePause > debounceDelay) {
    is_running = false;
    lastDebounceTimePause = currentMillis;
  }

  // Blue Ready button
  if (digitalRead(BLUE_BTN) == LOW && currentMillis - lastDebounceTimeBlue > debounceDelay) {
    blueReady = true;

    // Set border LEDs
    for (int i = 0; i < BORDER_LED_COUNT/2; i++) {
      border_leds[i] = CRGB::Blue; // First half blue
    }
    FastLED.show(); // Show the initialized border LED colors

    lastDebounceTimePause = currentMillis;
  }

   // Blue Ready button
  if (digitalRead(RED_BTN) == LOW && currentMillis - lastDebounceTimeRed > debounceDelay) {
    redReady = true;

    // Set border LEDs
    for (int i = BORDER_LED_COUNT/2; i < BORDER_LED_COUNT ; i++) {
      border_leds[i] = CRGB::Red; // Second half red
    }
    FastLED.show(); // Show the initialized border LED colors
    
    lastDebounceTimePause = currentMillis;
  }

  // Reset button
  if (digitalRead(RESET_BTN) == LOW && currentMillis - lastDebounceTimeReset > debounceDelay) {
    is_running = false;
    current_time = countdown_time;
    blueReady = false;
    redReady = false;
    updateClient();
    updateLEDs();
    lastDebounceTimeReset = currentMillis;
  }

  // Time select switch
  bool timeSelState = digitalRead(TIME_SEL_SW) == LOW;
  if (timeSelState != lastTimeSelState && currentMillis - lastDebounceTimeTimeSel > debounceDelay) {
    lastTimeSelState = timeSelState;
    countdown_time = timeSelState ? 180 : 120;
    current_time = countdown_time;
    updateClient();
    updateLEDs();
    lastDebounceTimeTimeSel = currentMillis;
  }
}

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  // Nothing to do here for now
}

void handleSetWiFi() {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");
    
    if (ssid.length() > 0 && pass.length() > 0) {
        preferences.begin("wifi", false);
        preferences.putString("ssid", ssid);
        preferences.putString("pass", pass);
        preferences.end();
        
        server.send(200, "text/html", "<h1>WiFi Credentials Saved! Rebooting...</h1>");
        delay(3000);
        ESP.restart();
    } else {
        server.send(200, "text/html", "<h1>Invalid Input</h1>");
    }
}

void connectWiFi() {
    preferences.begin("wifi", true);
    String ssid = preferences.getString("ssid", "");
    String pass = preferences.getString("pass", "");
    preferences.end();
    
    if (ssid.length() == 0) {
        Serial.println("No WiFi credentials stored, starting AP mode...");
        startAPMode();
        return;
    }
    
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.begin(ssid.c_str(), pass.c_str());
    
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 30000) {
        Serial.print(".");

        if(border_toggle)
        {
            //turn off LEDs
          for (int i = 0; i < BORDER_LED_COUNT; i++) {
            border_leds[i] = CRGB::Black;  // Turn off all LEDs
          }
          FastLED.show();  // Update LEDs
        }
        else
          setBorder();
        border_toggle = !border_toggle;

        delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());

        server.on("/", handleRoot);
        server.on("/control", handleControl);
        server.begin();

        webSocket.begin();
        webSocket.onEvent(onWebSocketEvent);
    } else {
        Serial.println("Failed to connect, starting AP mode...");
        startAPMode();
    }
}

void setup() {
  Serial.begin(115200);

  // Initialize LED strips
  FastLED.addLeds<NEOPIXEL, DIGIT_PIN>(digit_leds, DIGIT_LED_COUNT);
  FastLED.addLeds<NEOPIXEL, BORDER_PIN>(border_leds, BORDER_LED_COUNT);
  FastLED.setBrightness(255); // Set BRIGHTNESS to max
 
  updateLEDs();//update the text LEDs
  setBorder();//start the border on

  connectWiFi();

  setBorder(); //ensure we leave with border on
  
  if (setupMode) {
      server.on("/", handleRoot_AP);
      server.on("/setwifi", handleSetWiFi);
      server.begin();
  }

  // Set up the buttons and switch pins
  pinMode(RESET_BTN, INPUT_PULLUP);
  pinMode(PAUSE_BTN, INPUT_PULLUP);
  pinMode(START_BTN, INPUT_PULLUP);
  pinMode(TIME_SEL_SW, INPUT_PULLUP);
  pinMode(BLUE_BTN, INPUT_PULLUP);
  pinMode(RED_BTN, INPUT_PULLUP);
  lastTimeSelState = digitalRead(TIME_SEL_SW) == LOW;
  
  // Start the timer update task
  xTaskCreate(
      [](void*) {
          TickType_t lastWakeTime = xTaskGetTickCount();
          while (true) {
              if (is_running || preCountdownRunning) {
                  updateTimer();
              }
              vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1000));
          }
      },
      "TimerTask",
      2048,
      nullptr,
      1,
      nullptr
  );

  // Start the button check task
  xTaskCreate(
      [](void*) {
          while (true) {
              checkButtons();
              vTaskDelay(pdMS_TO_TICKS(50)); // Check buttons every 50ms
          }
      },
      "ButtonCheckTask",
      2048,
      nullptr,
      1,
      nullptr
  );

    xTaskCreate(
      [](void*) {
          TickType_t lastWakeTime = xTaskGetTickCount();
          while (true) {
              if (preCountdownRunning) {
                  static bool flashBorder = true;

                  // Toggle the border flash
                  flashBorder = !flashBorder;
                  for (int i = 0; i < BORDER_LED_COUNT; i++) {
                      border_leds[i] = flashBorder ? ORANGE : CRGB::Black;
                  }
                  FastLED.show();
              }
              vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(500)); // 500ms toggle
          }
      },
      "BorderFlashTask",
      2048,
      nullptr,
      1,
      nullptr
  );
}


void loop() {
  if (setupMode) {
      server.handleClient();
  }

  server.handleClient();
  webSocket.loop();
}

void setDigit(int digit, int offset, bool inverted) {
  static const uint8_t digitMap[10][7] = {
    {1, 1, 1, 1, 1, 1, 0},  // 0
    {0, 1, 1, 0, 0, 0, 0},  // 1
    {1, 1, 0, 1, 1, 0, 1},  // 2
    {1, 1, 1, 1, 0, 0, 1},  // 3
    {0, 1, 1, 0, 0, 1, 1},  // 4
    {1, 0, 1, 1, 0, 1, 1},  // 5
    {1, 0, 1, 1, 1, 1, 1},  // 6
    {1, 1, 1, 0, 0, 0, 0},  // 7
    {1, 1, 1, 1, 1, 1, 1},  // 8
    {1, 1, 1, 1, 0, 1, 1}   // 9
  };

  // Remap segments for inverted digits
  static const uint8_t invertedMap[7] = {3, 4, 5, 0, 1, 2, 6};

  for (int i = 0; i < 7; i++) {
    int segmentIndex = inverted ? invertedMap[i] : i;
    int ledIndex = offset + segmentIndex * 7;
    for (int j = 0; j < 7; j++) {
      if (digitMap[digit][i]) {
        digit_leds[ledIndex + j] = CRGB::Red;  // Red color
      } else {
        digit_leds[ledIndex + j] = CRGB::Black;  // Turn off
      }
    }
  }
}

void setColon() {
  for (int i = 0; i < 3; i++) {
    digit_leds[98 + i] = CRGB::Red;  // Colon dot
    digit_leds[199 + i] = CRGB::Red;  // Colon dot
  }
}

void updateLEDs() {
  int minutes = current_time / 60;
  int seconds = current_time % 60;

  // Convert minutes and seconds to individual digits
  int digit1 = minutes / 10;
  int digit2 = minutes % 10;
  int digit3 = seconds / 10;
  int digit4 = seconds % 10;

  // Update each 7-segment digit and colon dots
  setDigit(digit1, 0, false);     // First digit
  setDigit(digit2, 49, false);    // Second digit
  setColon();                     // Colon dots
  setDigit(digit4, 101, true);    // Third digit (upside down and backwards)
  setDigit(digit3, 150, true);    // Fourth digit (upside down and backwards)

  FastLED.show();  // Update the strip to show the changes
}

void setBorder()
{
  for (int i = 0; i < BORDER_LED_COUNT; i++) {
    border_leds[i].setRGB(127,127,127); //white
  }
 
  FastLED.show(); // Show the initialized border LED colors
}
