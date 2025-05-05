#ifndef BROWSER_H
#define BROWSER_H

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
            label { font-size: 18px; margin-top: 20px; display: block; }
        </style>
    </head>
    <body>
        <h1>ESP32 Countdown Timer</h1>
        <p id="countdown">02:00</p>
        <button onclick="controlTimer('start')">Start</button>
        <button onclick="controlTimer('pause')">Pause</button>
        <button onclick="controlTimer('reset')">Reset</button>
        <button onclick="toggleTime()">Switch 2min/3min</button>
        <label>
            <input type="checkbox" id="readyToggle" onchange="toggleReady()"> Require Driver Ready
        </label>
        <label for="brightness">Brightness</label>
        <input type="range" id="brightness" min="10" max="100" value="50" oninput="setBrightness(this.value)">
        <script>
          function setBrightness(value) {
            // Scale 0–100 to 0–255
            let scaled = Math.round(value * 2.55);
            fetch(`/control?cmd=brightness&value=${scaled}`);
          }
        </script>
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

            function toggleReady() {
                let checkbox = document.getElementById('readyToggle');
                let state = checkbox.checked ? "on" : "off";
                fetch(`/control?cmd=readytoggle&state=${state}`);
            }
        </script>
    </body>
    </html>
)rawliteral";

#endif
