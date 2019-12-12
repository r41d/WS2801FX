/*
  WS2801FX Webinterface.

  Harm Aldick - 2016
  www.aldick.org


  FEATURES
    * Webinterface with mode, color, speed and brightness selectors


  LICENSE

  The MIT License (MIT)

  Copyright (c) 2016  Harm Aldick

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.


  CHANGELOG
  2016-11-26 initial version

*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WS2801FX.h>

#include "index.html.h"
#include "main.js.h"

#define WIFI_SSID "YOURSSID"
#define WIFI_PASSWORD "YOURPASSWORD"

//#define STATIC_IP                       // uncomment for static IP, set IP below
#ifdef STATIC_IP
  IPAddress ip(192, 168, 0, 123);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
#endif

// QUICKFIX...See https://github.com/esp8266/Arduino/issues/263
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define LED_COUNT 50
#define LED_DATA_PIN 12
#define LED_CLOCK_PIN 14

#define WIFI_TIMEOUT 30000              // checks WiFi every ...ms. Reset after this time, if WiFi cannot reconnect.
#define HTTP_PORT 80

#define DEFAULT_WEB_COLOR 0xFF5900
#define DEFAULT_WEB_BRIGHTNESS 255
#define DEFAULT_WEB_SPEED 200
#define DEFAULT_WEB_MODE FX_MODE_STATIC

#define BRIGHTNESS_STEP 15              // in/decrease brightness by this amount per click
#define SPEED_STEP 10                   // in/decrease brightness by this amount per click

unsigned long last_wifi_check_time = 0;
String modes = "";

WS2801FX ws2801fx = WS2801FX(LED_COUNT, LED_DATA_PIN, LED_CLOCK_PIN, WS2801_RBG);
ESP8266WebServer server(HTTP_PORT);


void setup(){
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("Starting...");

  modes.reserve(5000);
  modes_setup();

  Serial.println("WS2801FX setup");
  ws2801fx.init();
  ws2801fx.setMode(DEFAULT_WEB_MODE);
  ws2801fx.setColor(DEFAULT_WEB_COLOR);
  ws2801fx.setSpeed(DEFAULT_WEB_SPEED);
  ws2801fx.setBrightness(DEFAULT_WEB_BRIGHTNESS);
  ws2801fx.start();

  Serial.println("Wifi setup");
  wifi_setup();

  Serial.println("HTTP server setup");
  server.on("/", srv_handle_index_html);
  server.on("/main.js", srv_handle_main_js);
  server.on("/modes", srv_handle_modes);
  server.on("/set", srv_handle_set);
  server.onNotFound(srv_handle_not_found);
  server.begin();
  Serial.println("HTTP server started.");

  Serial.println("ready!");
}


void loop() {
  unsigned long now = millis();

  server.handleClient();
  ws2801fx.service();

  if (now - last_wifi_check_time > WIFI_TIMEOUT) {
    Serial.print("Checking WiFi... ");
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi connection lost. Reconnecting...");
      wifi_setup();
    } else {
      Serial.println("OK");
    }
    last_wifi_check_time = now;
  }
}



/*
 * Connect to WiFi. If no connection is made within WIFI_TIMEOUT, ESP gets resettet.
 */
void wifi_setup() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.mode(WIFI_STA);
#ifdef STATIC_IP
  WiFi.config(ip, gateway, subnet);
#endif

  unsigned long connect_start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    if (millis() - connect_start > WIFI_TIMEOUT) {
      Serial.println();
      Serial.print("Tried ");
      Serial.print(WIFI_TIMEOUT);
      Serial.print("ms. Resetting ESP now.");
      ESP.reset();
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}


/*
 * Build <li> string for all modes.
 */
void modes_setup() {
  modes = "";
  for (uint8_t i = 0; i < ws2801fx.getModeCount(); i++) {
    modes += "<li><a href='#' class='m' id='";
    modes += i;
    modes += "'>";
    modes += ws2801fx.getModeName(i);
    modes += "</a></li>";
  }
}

/* #####################################################
#  Webserver Functions
##################################################### */

void srv_handle_not_found() {
  server.send(404, "text/plain", "File Not Found");
}

void srv_handle_index_html() {
  server.send_P(200, "text/html", index_html);
}

void srv_handle_main_js() {
  server.send_P(200, "application/javascript", main_js);
}

void srv_handle_modes() {
  server.send(200, "text/plain", modes);
}

void srv_handle_set() {
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "c") {
      char color[7];
      server.arg(i).toCharArray(color, 7);
      uint32_t tmp = (uint32_t) strtol(color, NULL, 16);
      if (tmp >= 0x000000 && tmp <= 0xFFFFFF) {
        ws2801fx.setColor(tmp);
      }
    }

    if (server.argName(i) == "m") {
      char mode[2];
      server.arg(i).toCharArray(mode,2);
      uint8_t tmp = (uint8_t) strtol(mode, NULL, 10);
      ws2801fx.setMode(tmp % ws2801fx.getModeCount());
    }

    if (server.argName(i) == "b") {
      if (server.arg(i)[0] == '-') {
        ws2801fx.decreaseBrightness(BRIGHTNESS_STEP);
      } else {
        ws2801fx.increaseBrightness(BRIGHTNESS_STEP);
      }
    }

    if (server.argName(i) == "s") {
      if (server.arg(i)[0] == '-') {
        ws2801fx.decreaseSpeed(SPEED_STEP);
      } else {
        ws2801fx.increaseSpeed(SPEED_STEP);
      }
    }
  }
  server.send(200, "text/plain", "OK");
}
