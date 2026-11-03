// Week09 Example 2B: 버튼 바운싱 관찰 (예제 2 회로 그대로: GPIO18 버튼, GPIO25 LED)
// raw 변화 횟수(바운스 포함)와 디바운스 후 '눌림' 횟수를 비교한다.
const int BUTTON_PIN = 18, LED_PIN = 25;
unsigned long DEBOUNCE_MS = 25;      // 시리얼로 숫자(예: 5, 25, 100)를 보내면 바뀐다

bool lastRaw = HIGH, stable = HIGH;
unsigned long changedAt = 0;
unsigned long rawEdges = 0, presses = 0;
bool ledOn = false;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("Press the button 10 times. Send a number to change DEBOUNCE_MS.");
}

void loop() {
  unsigned long now = millis();
  bool raw = digitalRead(BUTTON_PIN);
  if (raw != lastRaw) {            // 모든 변화(바운스 포함)를 센다
    rawEdges++;
    changedAt = now;
    lastRaw = raw;
  }
  if (now - changedAt >= DEBOUNCE_MS && raw != stable) {
    stable = raw;
    if (stable == LOW) {
      presses++;
      ledOn = !ledOn;
      digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
      Serial.printf("press #%lu  rawEdges=%lu  (debounce %lu ms)\n", presses, rawEdges, DEBOUNCE_MS);
    }
  }
  if (Serial.available()) {
    long v = Serial.parseInt();
    if (v > 0) {
      DEBOUNCE_MS = v; rawEdges = 0; presses = 0;
      Serial.printf("DEBOUNCE_MS = %lu, counters reset\n", DEBOUNCE_MS);
    }
  }
}
