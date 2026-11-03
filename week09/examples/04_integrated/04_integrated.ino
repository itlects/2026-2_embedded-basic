// Week09 Example 4: two buttons + two LEDs + pot + passive buzzer
// ESP32 Arduino core 3.x. Uses analogWriteFrequency/pin analogWrite for buzzer.
const int LED1 = 25, LED2 = 26, BTN_RUN = 18, BTN_MODE = 19;
const int POT = 34, BUZZER = 27;
struct DebouncedButton {
  int pin; bool stable = HIGH, rawBefore = HIGH; unsigned long changed = 0;
  bool pressed(unsigned long now) {
    bool raw = digitalRead(pin);
    if (raw != rawBefore) changed = now;
    bool event = false;
    if (now - changed >= 25 && raw != stable) {
      stable = raw; event = (stable == LOW);
    }
    rawBefore = raw;
    return event;
  }
};
DebouncedButton runButton{BTN_RUN}, modeButton{BTN_MODE};
bool running = true, alarmMode = false, led2On = false;
int adc = 0; unsigned long lastSample = 0, lastBlink = 0;
unsigned long beepUntil = 0;
void chirp(unsigned long now) { 
  // active-high passive buzzer ONLY if its module is rated for 3.3V GPIO and low current
  analogWriteFrequency(BUZZER, 2000);
  analogWrite(BUZZER, 128);
  beepUntil = now + 100;
}
void setup() {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT); pinMode(LED2, OUTPUT); pinMode(BUZZER, OUTPUT);
  pinMode(BTN_RUN, INPUT_PULLUP); pinMode(BTN_MODE, INPUT_PULLUP);
  analogReadResolution(12);
  analogSetPinAttenuation(POT, ADC_11db);
  digitalWrite(LED1, LOW); digitalWrite(LED2, LOW);
  analogWrite(BUZZER, 0);
  Serial.println("Week09 ready; RUN GPIO18, MODE GPIO19");
}
void loop() {
  unsigned long now = millis();
  if (runButton.pressed(now)) {
    running = !running; Serial.printf("RUN=%d\n", running);
    chirp(now);
  }
  if (modeButton.pressed(now)) {
    alarmMode = !alarmMode; Serial.printf("ALARM_MODE=%d\n", alarmMode);
    chirp(now);
  }
  if (now - lastSample >= 100) {
    lastSample = now;
    adc = analogRead(POT);
    int pwm = running ? map(adc, 0, 4095, 0, 255) : 0;
    analogWrite(LED1, constrain(pwm, 0, 255));
    Serial.printf("ADC=%d PWM=%d\n", adc, pwm);
  }
  if (now - lastBlink >= 400) {
    lastBlink = now;
    led2On = running && !led2On;
    digitalWrite(LED2, led2On ? HIGH : LOW);
  }
  // Mode ON and high ADC: additional 150ms warning tone, every 1 second.
  static unsigned long lastAlarm = 0;
  if (running && alarmMode && adc > 3200 &&
      now - lastAlarm >= 1000 && now >= beepUntil) {
    lastAlarm = now;
    analogWriteFrequency(BUZZER, 1500);
    analogWrite(BUZZER, 128);
    beepUntil = now + 150;
  }
  if (beepUntil != 0 && (long)(now - beepUntil) >= 0) {
    analogWrite(BUZZER, 0);
    beepUntil = 0;
  }
}
