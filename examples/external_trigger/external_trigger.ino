#include <WS2801FX.h>

#define LED_COUNT 60
#define LED_DATA_PIN 12
#define LED_CLOCK_PIN 14

#define ANALOG_PIN A0
#define ANALOG_THRESHOLD 512

#define TIMER_MS 3000

// Parameter 1 = number of pixels in strip
// Parameter 2 = data pin number
// Parameter 3 = clock pin number
// Parameter 4 (optional) = pixel type, one of WS2801_RGB/GRB/RBG
WS2801FX ws2801fx = WS2801FX(LED_COUNT, LED_DATA_PIN, LED_CLOCK_PIN, WS2801_RGB);

unsigned long last_trigger = 0;
unsigned long now = 0;

void setup() {
  ws2801fx.init();
  ws2801fx.setBrightness(255);
  ws2801fx.setMode(FX_MODE_RANDOM_COLOR);
}

void loop() {
  now = millis();

  ws2801fx.service();

  // trigger on a regular basis
  if(now - last_trigger > TIMER_MS) {
    ws2801fx.trigger();
    last_trigger = now;
  }

  // trigger, if analog value is above threshold
  // this comes in handy, when using a microphone on analog input
  if(analogRead(ANALOG_PIN) > ANALOG_THRESHOLD) {
    ws2801fx.trigger();
  }
}
