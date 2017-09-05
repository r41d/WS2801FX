#include <WS2801FX.h>

#define LED_COUNT 60
#define LED_DATA_PIN 12
#define LED_CLOCK_PIN 14

#define TIMER_MS 5000

// Parameter 1 = number of pixels in strip
// Parameter 2 = data pin number
// Parameter 3 = clock pin number
// Parameter 4 (optional) = pixel type, one of WS2801_RGB/GRB/RBG
WS2801FX ws2801fx = WS2801FX(LED_COUNT, LED_DATA_PIN, LED_CLOCK_PIN, WS2801_RGB);

unsigned long last_change = 0;
unsigned long now = 0;

void setup() {
  ws2801fx.init();
  ws2801fx.setBrightness(255);
  ws2801fx.setSpeed(200);
  ws2801fx.setColor(0x007BFF);
  ws2801fx.setMode(FX_MODE_STATIC);
  ws2801fx.start();
}

void loop() {
  now = millis();

  ws2801fx.service();

  if(now - last_change > TIMER_MS) {
    ws2801fx.setMode((ws2801fx.getMode() + 1) % ws2801fx.getModeCount());
    last_change = now;
  }
}
