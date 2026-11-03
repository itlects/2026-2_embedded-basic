// Week09 Example 3B: LEDC로 PWM 주파수·해상도를 직접 지정 (예제 3 회로의 LED만 사용: GPIO25)
// Arduino-ESP32 3.x API: ledcAttach(pin, freq, resolution), ledcWrite(pin, duty)
const int LED_PIN = 25;
const uint32_t PWM_FREQ = 5000;     // 5kHz
const uint8_t  PWM_BITS = 8;        // 8비트 → 0~255
const uint32_t STEPS[] = {0, 64, 128, 192, 255};
const int NSTEPS = sizeof(STEPS) / sizeof(STEPS[0]);
int idx = 0;
unsigned long last = 0;

void setup() {
  Serial.begin(115200);
  if (!ledcAttach(LED_PIN, PWM_FREQ, PWM_BITS)) {
    Serial.println("ledcAttach failed (freq x 2^bits too high?)");
  }
}

void loop() {
  unsigned long now = millis();
  if (now - last >= 1500) {
    last = now;
    uint32_t duty = STEPS[idx];
    ledcWrite(LED_PIN, duty);
    float percent = duty * 100.0f / ((1 << PWM_BITS) - 1);
    Serial.printf("duty=%3lu/255  %5.1f%%  avg~%.2f V\n", (unsigned long)duty, percent, 3.3f * percent / 100.0f);
    idx = (idx + 1) % NSTEPS;
  }
}
