// Week09 Example 2: GPIO18 button -> GPIO25 LED toggle
const int LED_PIN = 25, BUTTON_PIN = 18;
bool ledOn = false;
bool stable = HIGH, previousRaw = HIGH;
unsigned long changedAt = 0;
const unsigned long DEBOUNCE_MS = 25;
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, LOW);
}
void loop() {
  bool raw = digitalRead(BUTTON_PIN);
  unsigned long now = millis();
  if (raw != previousRaw) changedAt = now;
  if (now - changedAt >= DEBOUNCE_MS && raw != stable) {
    stable = raw;
    if (stable == LOW) {
      ledOn = !ledOn;
      digitalWrite(LED_PIN, ledOn);
      Serial.printf("Button pressed, LED=%s\n", ledOn ? "ON" : "OFF");
    }
  }
  previousRaw = raw;
}
