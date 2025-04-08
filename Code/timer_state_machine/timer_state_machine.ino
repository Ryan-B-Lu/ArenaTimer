#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <FastLED.h>
#include <Preferences.h>  // Use NVS for WiFi storage
#include "Browser.h"

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

enum states_t{check_wifi, timer_idle, timer_running, timer_paused, waiting_for_ready, tapout };

void setup() {
  

}

void loop() {
  

}
