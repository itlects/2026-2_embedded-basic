// Week07 Workbook 4 - 4-button interrupt LED sequencer with passive buzzer
// Board: ESP32 DevKit  Serial Monitor: 115200
// Switch pins -> switch -> GND, INPUT_PULLUP. LEDs: pin -> 220~330 ohm -> LED anode; cathode -> GND.
// Passive buzzer: signal -> GPIO 23, other terminal -> GND (small low-current passive piezo only).
const uint8_t BUTTONS[4] = {18, 19, 21, 22};
const uint8_t LEDS[5] = {13, 14, 25, 26, 27};
const uint8_t BUZZER = 23;
const uint32_t DEBOUNCE_MS = 35;
const uint16_t TEMPOS[3] = {450, 250, 120};
const uint16_t NOTES[5] = {262, 330, 392, 523, 392};

portMUX_TYPE buttonMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t irqFlags = 0;
void ARDUINO_ISR_ATTR isr0() { portENTER_CRITICAL_ISR(&buttonMux); irqFlags |= 1; portEXIT_CRITICAL_ISR(&buttonMux); }
void ARDUINO_ISR_ATTR isr1() { portENTER_CRITICAL_ISR(&buttonMux); irqFlags |= 2; portEXIT_CRITICAL_ISR(&buttonMux); }
void ARDUINO_ISR_ATTR isr2() { portENTER_CRITICAL_ISR(&buttonMux); irqFlags |= 4; portEXIT_CRITICAL_ISR(&buttonMux); }
void ARDUINO_ISR_ATTR isr3() { portENTER_CRITICAL_ISR(&buttonMux); irqFlags |= 8; portEXIT_CRITICAL_ISR(&buttonMux); }

bool awaiting[4] = {false, false, false, false};
uint32_t lowAt[4] = {0, 0, 0, 0};
bool running = false;
uint8_t mode = 0, speedIndex = 1, position = 0;
uint32_t lastStep = 0, buzzerStopAt = 0;
bool buzzerActive = false;

uint8_t fetchEvents() {
  portENTER_CRITICAL(&buttonMux);
  uint8_t flags = (uint8_t)irqFlags;
  irqFlags = 0;
  portEXIT_CRITICAL(&buttonMux);
  return flags;
}

void printState(const char* eventName) {
  Serial.print("[EVENT] "); Serial.println(eventName);
  Serial.print("[STATE] running="); Serial.print(running ? "ON" : "OFF");
  Serial.print(" mode="); Serial.print(mode);
  Serial.print(" tempo="); Serial.print(TEMPOS[speedIndex]);
  Serial.print("ms position="); Serial.println(position);
}

void drawLeds() {
  for (uint8_t i = 0; i < 5; ++i) {
    bool on = false;
    if (mode == 0) on = (i == position);                // chase
    else if (mode == 1) on = (i <= position);           // bar graph
    else on = ((position % 2) == 0);                    // all blink
    digitalWrite(LEDS[i], (running && on) ? HIGH : LOW);
  }
}

void handleButton(uint8_t index, uint32_t now) {
  switch (index) {
    case 0: 
      running = !running;
      lastStep = now;
      if (!running) { noTone(BUZZER); buzzerActive = false; }
      printState("A START/PAUSE");
      break;
    case 1:
      mode = (mode + 1) % 3;
      printState("B MODE");
      break;
    case 2:
      speedIndex = (speedIndex + 1) % 3;
      lastStep = now;   // 주기 변경 시 기준 시각 재설정(밀린 스텝이 연속 실행되는 현상 방지)
      printState("C SPEED");
      break;
    case 3:
      running = false; mode = 0; speedIndex = 1; position = 0;
      noTone(BUZZER); buzzerActive = false;
      printState("D RESET");
      break;
  }
  drawLeds();
}

void updateButtons(uint32_t now) {
  uint8_t newEvents = fetchEvents();
  for (uint8_t i = 0; i < 4; ++i) {
    if (newEvents & (1U << i)) {
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
    handleButton(i, now);
  }
}

void updateSequence(uint32_t now) {
  if (buzzerActive && (int32_t)(now - buzzerStopAt) >= 0) {
    noTone(BUZZER);
    buzzerActive = false;
  }
  if (!running) return;
  if ((uint32_t)(now - lastStep) < TEMPOS[speedIndex]) return;
  lastStep += TEMPOS[speedIndex];
  position = (position + 1) % 5;
  drawLeds();
  tone(BUZZER, NOTES[position]);
  buzzerStopAt = now + 80;
  buzzerActive = true;
  Serial.print("[STEP] led="); Serial.print(position + 1);
  Serial.print(" note="); Serial.println(NOTES[position]);
}

void setup() {
  Serial.begin(115200);
  for (uint8_t i = 0; i < 4; ++i) pinMode(BUTTONS[i], INPUT_PULLUP);
  for (uint8_t i = 0; i < 5; ++i) {
    pinMode(LEDS[i], OUTPUT);
    digitalWrite(LEDS[i], LOW);
  }
  pinMode(BUZZER, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTONS[0]), isr0, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTONS[1]), isr1, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTONS[2]), isr2, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTONS[3]), isr3, FALLING);
  lastStep = millis();
  Serial.println("=== Week07 Advanced Sequencer ===");
  Serial.println("A=RUN B=MODE C=SPEED D=RESET");
  printState("READY");
}

void loop() {
  uint32_t now = millis();
  updateButtons(now);
  updateSequence(now);
}
