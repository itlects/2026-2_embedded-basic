// Week09 Example 1: GPIO25 LED, nonblocking blink
const int LED_PIN = 25;
unsigned long lastToggle = 0;
bool ledOn = false;
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}
void loop() {
  unsigned long now = millis();
  if (now - lastToggle >= 500) {
    lastToggle = now;
    ledOn = !ledOn;
    digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
    Serial.println(ledOn ? "LED ON" : "LED OFF");
  }
}
