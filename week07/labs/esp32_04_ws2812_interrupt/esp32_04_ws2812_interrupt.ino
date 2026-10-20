#include <Adafruit_NeoPixel.h>


// Week 07 - Interrupt + WS2812 (8 LEDs bar, 12 LEDs ring)
// Button A: increase level / B: reset / C: pause or resume
// Open Serial Monitor at 115200 baud.
// LED modules: external regulated 5V, shared GND with MCU.
// ESP32: use a 5V logic level shifter on both WS2812 data lines.
const uint8_t BAR_PIN = 23;
const uint8_t RING_PIN = 22;
const uint8_t BUTTON_PINS[3] = {18, 19, 21};
const uint8_t BAR_COUNT = 8;
const uint8_t RING_COUNT = 12;
const uint32_t DEBOUNCE_MS = 30;

Adafruit_NeoPixel bar(BAR_COUNT, BAR_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring(RING_COUNT, RING_PIN, NEO_GRB + NEO_KHZ800);

portMUX_TYPE eventMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t interruptEvents = 0;

void ARDUINO_ISR_ATTR onIncrease() {
  portENTER_CRITICAL_ISR(&eventMux);
  interruptEvents |= 0x01;
  portEXIT_CRITICAL_ISR(&eventMux);
}

void ARDUINO_ISR_ATTR onReset() {
  portENTER_CRITICAL_ISR(&eventMux);
  interruptEvents |= 0x02;
  portEXIT_CRITICAL_ISR(&eventMux);
}

void ARDUINO_ISR_ATTR onPause() {
  portENTER_CRITICAL_ISR(&eventMux);
  interruptEvents |= 0x04;
  portEXIT_CRITICAL_ISR(&eventMux);
}
bool waitingForStablePress[3] = {false, false, false};
uint32_t lowStartedAt[3] = {0, 0, 0};
bool paused = false;
uint8_t level = 0;
uint32_t lastClockTick = 0;
uint32_t elapsedSeconds = 0;

void printStatus(const char* eventName) {
  Serial.print("[EVENT] ");
  Serial.println(eventName);

  Serial.print("[STATE] level=");
  Serial.print(level);
  Serial.print("/12, bar=");
  Serial.print(level < BAR_COUNT ? level : BAR_COUNT);
  Serial.print("/8, ring=");
  Serial.print(level);
  Serial.print("/12, pause=");
  Serial.println(paused ? "ON" : "OFF");
}

void renderLeds() {
  bar.clear();
  ring.clear();

  for (uint8_t i = 0; i < level && i < BAR_COUNT; i++) {
    bar.setPixelColor(i, bar.Color(0, 30, 0));
  }

  for (uint8_t i = 0; i < level && i < RING_COUNT; i++) {
    ring.setPixelColor(i, ring.Color(0, 0, 30));
  }

  // The final pixel is red when paused, replacing its level color.
  if (paused) {
    bar.setPixelColor(7, bar.Color(30, 0, 0));
    ring.setPixelColor(11, ring.Color(30, 0, 0));
  }

  // Never call show() from an interrupt service routine.
  bar.show();
  ring.show();
}

void setup() {
  Serial.begin(115200);

  for (uint8_t i = 0; i < 3; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }

  bar.begin();
  ring.begin();
  bar.setBrightness(50);
  ring.setBrightness(50);
  renderLeds();

  attachInterrupt(digitalPinToInterrupt(BUTTON_PINS[0]), onIncrease, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PINS[1]), onReset, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PINS[2]), onPause, FALLING);

  lastClockTick = millis();
  Serial.println("=== Week 07 WS2812 Interrupt Lab ===");
  Serial.println("A: increase  B: reset  C: pause/resume");
  printStatus("START");
}

void updateClock(uint32_t now) {
  while ((uint32_t)(now - lastClockTick) >= 1000UL) {
    lastClockTick += 1000UL;
    elapsedSeconds++;

    uint32_t hours = (elapsedSeconds / 3600UL) % 24UL;
    uint8_t minutes = (elapsedSeconds / 60UL) % 60UL;
    uint8_t seconds = elapsedSeconds % 60UL;

    Serial.print("[CLOCK] ");
    if (hours < 10) Serial.print('0');
    Serial.print(hours);
    Serial.print(':');
    if (minutes < 10) Serial.print('0');
    Serial.print(minutes);
    Serial.print(':');
    if (seconds < 10) Serial.print('0');
    Serial.println(seconds);
  }
}

uint8_t takeInterruptEvents() {
  portENTER_CRITICAL(&eventMux);
  uint8_t events = (uint8_t)interruptEvents;
  interruptEvents = 0;
  portEXIT_CRITICAL(&eventMux);
  return events;
}

void processButton(uint8_t index) {
  if (index == 0) {
    if (!paused) {
      level = (level + 1) % 13;
      printStatus("A INCREASE");
    } else {
      printStatus("A IGNORED (PAUSED)");
    }
  } else if (index == 1) {
    level = 0;
    printStatus("B RESET");
  } else {
    paused = !paused;
    printStatus(paused ? "C PAUSE" : "C RESUME");
  }

  renderLeds();
}

void updateButtons(uint32_t now) {
  uint8_t events = takeInterruptEvents();

  for (uint8_t i = 0; i < 3; i++) {
    if (events & (1U << i)) {
      waitingForStablePress[i] = true;
      lowStartedAt[i] = 0;
    }

    if (!waitingForStablePress[i]) {
      continue;
    }

    if (digitalRead(BUTTON_PINS[i]) == HIGH) {
      lowStartedAt[i] = 0;
      continue;
    }

    if (lowStartedAt[i] == 0) {
      lowStartedAt[i] = now;
    }

    if ((uint32_t)(now - lowStartedAt[i]) < DEBOUNCE_MS) {
      continue;
    }

    waitingForStablePress[i] = false;
    lowStartedAt[i] = 0;
    processButton(i);
  }
}

void loop() {
  uint32_t now = millis();
  updateClock(now);
  updateButtons(now);
}
