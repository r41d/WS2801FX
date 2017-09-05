#include <WS2801FX.h>

#define LED_COUNT 60
#define LED_DATA_PIN 12
#define LED_CLOCK_PIN 14

// Parameter 1 = number of pixels in strip
// Parameter 2 = data pin number
// Parameter 3 = clock pin number
// Parameter 4 (optional) = pixel type, one of WS2801_RGB/GRB/RBG
WS2801FX ws2801fx = WS2801FX(LED_COUNT, LED_DATA_PIN, LED_CLOCK_PIN, WS2801_RGB);

String cmd = "";               // String to store incoming serial commands
boolean cmd_complete = false;  // whether the command string is complete


void setup() {
  Serial.begin(115200);
  ws2801fx.init();
  ws2801fx.setBrightness(30);
  ws2801fx.setSpeed(200);
  ws2801fx.setColor(0x007BFF);
  ws2801fx.setMode(FX_MODE_STATIC);
  ws2801fx.start();

  printModes();
  printUsage();
}

void loop() {
  ws2801fx.service();

  // On Atmega32U4 based boards (leonardo, micro) serialEvent is not called
  // automatically when data arrive on the serial RX. We need to do it ourself
  #if defined(__AVR_ATmega32U4__)
  serialEvent();
  #endif

  if(cmd_complete) {
    process_command();
  }
}

/*
 * Checks received command and calls corresponding functions.
 */
void process_command() {
  if(cmd == F("b+")) {
    ws2801fx.increaseBrightness(25);
    Serial.print(F("Increased brightness by 25 to: "));
    Serial.println(ws2801fx.getBrightness());
  }

  if(cmd == F("b-")) {
    ws2801fx.decreaseBrightness(25);
    Serial.print(F("Decreased brightness by 25 to: "));
    Serial.println(ws2801fx.getBrightness());
  }

  if(cmd.startsWith(F("b "))) {
    uint8_t b = (uint8_t) cmd.substring(2, cmd.length()).toInt();
    ws2801fx.setBrightness(b);
    Serial.print(F("Set brightness to: "));
    Serial.println(ws2801fx.getBrightness());
  }

  if(cmd == F("s+")) {
    ws2801fx.increaseSpeed(10);
    Serial.print(F("Increased speed by 10 to: "));
    Serial.println(ws2801fx.getSpeed());
  }

  if(cmd == F("s-")) {
    ws2801fx.decreaseSpeed(10);
    Serial.print(F("Decreased speed by 10 to: "));
    Serial.println(ws2801fx.getSpeed());
  }

  if(cmd.startsWith(F("s "))) {
    uint8_t s = (uint8_t) cmd.substring(2, cmd.length()).toInt();
    ws2801fx.setSpeed(s);
    Serial.print(F("Set speed to: "));
    Serial.println(ws2801fx.getSpeed());
  }

  if(cmd.startsWith(F("m "))) {
    uint8_t m = (uint8_t) cmd.substring(2, cmd.length()).toInt();
    ws2801fx.setMode(m);
    Serial.print(F("Set mode to: "));
    Serial.print(ws2801fx.getMode());
    Serial.print(" - ");
    Serial.println(ws2801fx.getModeName(ws2801fx.getMode()));
  }

  if(cmd.startsWith(F("c "))) {
    uint32_t c = (uint32_t) strtol(&cmd.substring(2, cmd.length())[0], NULL, 16);
    ws2801fx.setColor(c);
    Serial.print(F("Set color to: "));
    Serial.print(F("0x"));
    if(ws2801fx.getColor() < 0x100000) { Serial.print(F("0")); }
    if(ws2801fx.getColor() < 0x010000) { Serial.print(F("0")); }
    if(ws2801fx.getColor() < 0x001000) { Serial.print(F("0")); }
    if(ws2801fx.getColor() < 0x000100) { Serial.print(F("0")); }
    if(ws2801fx.getColor() < 0x000010) { Serial.print(F("0")); }
    Serial.println(ws2801fx.getColor(), HEX);
  }

  cmd = "";              // reset the commandstring
  cmd_complete = false;  // reset command complete
}

/*
 * Prints a usage menu.
 */
void printUsage() {
  Serial.println(F("Usage:"));
  Serial.println();
  Serial.println(F("m <n> \t : select mode <n>"));
  Serial.println();
  Serial.println(F("b+ \t : increase brightness"));
  Serial.println(F("b- \t : decrease brightness"));
  Serial.println(F("b <n> \t : set brightness to <n>"));
  Serial.println();
  Serial.println(F("s+ \t : increase speed"));
  Serial.println(F("s- \t : decrease speed"));
  Serial.println(F("s <n> \t : set speed to <n>"));
  Serial.println();
  Serial.println(F("c 0x007BFF \t : set color to 0x007BFF"));
  Serial.println();
  Serial.println();
  Serial.println(F("Have a nice day."));
  Serial.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
  Serial.println();
}


/*
 * Prints all available WS2801FX blinken modes.
 */
void printModes() {
  Serial.println(F("Supporting the following modes: "));
  Serial.println();
  for(int i=0; i < ws2801fx.getModeCount(); i++) {
    Serial.print(i);
    Serial.print(F("\t"));
    Serial.println(ws2801fx.getModeName(i));
  }
  Serial.println();
}


/*
 * Reads new input from serial to cmd string. Command is completed on \n
 */
void serialEvent() {
  while(Serial.available()) {
    char inChar = (char) Serial.read();
    if(inChar == '\n') {
      cmd_complete = true;
    } else {
      cmd += inChar;
    }
  }
}
