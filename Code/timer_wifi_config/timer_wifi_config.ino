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

bool wifiConnected = false;
bool setupMode = false;

void startAPMode() {
    Serial.println("Starting Access Point mode...");
    WiFi.softAP(apSSID, apPassword);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
    setupMode = true;
}

void handleRoot() {
    server.send(200, "text/html", "<h1>Enter WiFi Credentials</h1><form action='/setwifi' method='post'>SSID: <input name='ssid'><br>Password: <input name='pass' type='password'><br><input type='submit' value='Save'></form>");
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
        delay(500);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        wifiConnected = true;
    } else {
        Serial.println("Failed to connect, starting AP mode...");
        startAPMode();
    }
}

void setup() {
    Serial.begin(115200);
    connectWiFi();
    
    if (setupMode) {
        server.on("/", handleRoot);
        server.on("/setwifi", handleSetWiFi);
        server.begin();
    }
}

void loop() {
    if (setupMode) {
        server.handleClient();
    }
}
