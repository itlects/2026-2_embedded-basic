#include <DHT.h>


// DHT11 environment alarm: 4 interrupt buttons + 5 LEDs + passive piezo.
// Install "DHT sensor library" and "Adafruit Unified Sensor" by Adafruit.
const uint8_t DHT_PIN = 32;
const uint8_t BUZZER = 23;
const uint8_t BUTTONS[4] = {18, 19, 21, 22};
const uint8_t LEDS[5] = {13, 14, 25, 26, 27};
const uint32_t SAMPLE_MS = 2500;
const uint32_t DEBOUNCE_MS = 35;
DHT dht(DHT_PIN, DHT11);

portMUX_TYPE eventMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t eventBits = 0;
void ARDUINO_ISR_ATTR onA() { portENTER_CRITICAL_ISR(&eventMux); eventBits |= 1; portEXIT_CRITICAL_ISR(&eventMux); }
void ARDUINO_ISR_ATTR onB() { portENTER_CRITICAL_ISR(&eventMux); eventBits |= 2; portEXIT_CRITICAL_ISR(&eventMux); }
void ARDUINO_ISR_ATTR onC() { portENTER_CRITICAL_ISR(&eventMux); eventBits |= 4; portEXIT_CRITICAL_ISR(&eventMux); }
void ARDUINO_ISR_ATTR onD() { portENTER_CRITICAL_ISR(&eventMux); eventBits |= 8; portEXIT_CRITICAL_ISR(&eventMux); }

bool awaiting[4] = {false, false, false, false};
uint32_t lowAt[4] = {0, 0, 0, 0};
uint32_t lastSample = 0, lastBeep = 0, beepStarted = 0;
bool alarmEnabled = true, muted = false, sensorOk = false;
bool beeping = false;
uint8_t threshold = 40;
float humidity = NAN, temperature = NAN;

uint8_t takeEvents() {
  portENTER_CRITICAL(&eventMux);
  uint8_t bits = (uint8_t)eventBits;
  eventBits = 0;
  portEXIT_CRITICAL(&eventMux);
  return bits;
}

void printState(const char* reason) {
  Serial.print("[EVENT] "); Serial.println(reason);
  Serial.print("[CONFIG] threshold="); Serial.print(threshold);
  Serial.print("% alarm="); Serial.print(alarmEnabled ? "ON" : "OFF");
  Serial.print(" mute="); Serial.println(muted ? "ON" : "OFF");
}

void stopBeep() {
  noTone(BUZZER);
  beeping = false;
}

void updateLeds() {
  // Five LEDs represent humidity bands: each 20% adds one LED.
  // Reading error: blink all LEDs once per second.
  uint8_t bars = sensorOk ? (uint8_t)constrain((int)ceil(humidity / 20.0f), 0, 5) : 0;
  bool errorBlink = ((millis() / 500UL) % 2UL) == 0;
  for (uint8_t i = 0; i < 5; i++) {
    bool on = sensorOk ? (i < bars) : errorBlink;
    digitalWrite(LEDS[i], on ? HIGH : LOW);
  }
}

void handleButton(uint8_t i) {
  if (i == 0) {
    alarmEnabled = !alarmEnabled;
    if (!alarmEnabled) stopBeep();
    printState("A ALARM TOGGLE");
  } else if (i == 1) {
    threshold = min(90, (int)threshold + 5);
    printState("B THRESHOLD +5");
  } else if (i == 2) {
    threshold = max(20, (int)threshold - 5);
    printState("C THRESHOLD -5");
  } else {
    muted = !muted;
    if (muted) stopBeep();
    printState("D MUTE TOGGLE");
  }
}

void updateButtons(uint32_t now) {
  uint8_t bits = takeEvents();
  for (uint8_t i = 0; i < 4; i++) {
    if (bits & (1U << i)) {
      awaiting[i] = true;
      lowAt[i] = 0;
    }
    if (!awaiting[i]) continue;
    if (digitalRead(BUTTONS[i]) == HIGH) {
      lowAt[i] = 0;
      continue;
    }
    if (lowAt[i] == 0) lowAt[i] = now;
    if ((uint32_t)(now - lowAt[i]) < DEBOUNCE_MS) continue;
    awaiting[i] = false;
    lowAt[i] = 0;
    handleButton(i);
  }
}

void sampleDht(uint32_t now) {
  if ((uint32_t)(now - lastSample) < SAMPLE_MS) return;
  lastSample = now;
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    sensorOk = false;
    stopBeep();
    Serial.println("[SENSOR] DHT11 read failed - check wiring");
    return;
  }

  humidity = h;
  temperature = t;
  sensorOk = true;
  Serial.print("[SENSOR] temp=");
  Serial.print(temperature, 1);
  Serial.print("C humidity=");
  Serial.print(humidity, 1);
  Serial.println("%");
}

void updateAlarm(uint32_t now) {
  bool dry = sensorOk && alarmEnabled && !muted && humidity < threshold;
  if (!dry) {
    if (beeping) stopBeep();
    return;
  }
  if (beeping) {
    if ((uint32_t)(now - beepStarted) >= 120UL) stopBeep();
  } else if ((uint32_t)(now - lastBeep) >= 1000UL) {
    tone(BUZZER, 1200);
    beepStarted = now;
    lastBeep = now;
    beeping = true;
    Serial.println("[ALARM] LOW HUMIDITY");
  }
}

void setup() {
  Serial.begin(115200);
  for (uint8_t i = 0; i < 4; i++) pinMode(BUTTONS[i], INPUT_PULLUP);
  for (uint8_t i = 0; i < 5; i++) {
    pinMode(LEDS[i], OUTPUT);
    digitalWrite(LEDS[i], LOW);
  }
  pinMode(BUZZER, OUTPUT);
  dht.begin();
  attachInterrupt(digitalPinToInterrupt(BUTTONS[0]), onA, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTONS[1]), onB, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTONS[2]), onC, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTONS[3]), onD, FALLING);
  lastSample = millis() - SAMPLE_MS;
  Serial.println("DHT11 Alarm A:on/off B:+5 C:-5 D:mute");
  printState("READY");
}

void loop() {
  uint32_t now = millis();
  updateButtons(now);
  sampleDht(now);
  updateAlarm(now);
  updateLeds();
}
