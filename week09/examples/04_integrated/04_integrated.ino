// Week09 Example 4: 버튼2 + LED2 + 가변저항 + 수동 부저 복합 이벤트 제어 (delay() 없음)
// ESP32 Arduino core 3.x
//   RUN  버튼 GPIO18 : 실행/정지 토글      LED1 GPIO25 : ADC 값 → PWM 밝기
//   MODE 버튼 GPIO19 : 경보 모드 토글      LED2 GPIO26 : RUN 중 400ms 점멸
//   가변저항 GPIO34  : ADC1_CH6           부저 GPIO27 : LEDC 톤 (3.3V 소형 수동 부저 또는 NPN 구동)
const int LED1 = 25, LED2 = 26, BTN_RUN = 18, BTN_MODE = 19;
const int POT = 34, BUZZER = 27;
const int ALARM_LEVEL = 3200;          // 경보 임계값 (0~4095)

struct DebouncedButton {
  int pin;
  bool stable = HIGH, rawBefore = HIGH;
  unsigned long changed = 0;
  bool pressed(unsigned long now) {    // 눌린 '순간'에만 true
    bool raw = digitalRead(pin);
    if (raw != rawBefore) changed = now;
    bool event = false;
    if (now - changed >= 25 && raw != stable) {
      stable = raw;
      event = (stable == LOW);
    }
    rawBefore = raw;
    return event;
  }
};

DebouncedButton runButton{BTN_RUN}, modeButton{BTN_MODE};
bool running = true, alarmMode = false, led2On = false;
int adc = 0;
unsigned long lastSample = 0, lastBlink = 0, lastAlarm = 0, lastLog = 0;
unsigned long beepEnd = 0;
bool beeping = false;

void beep(unsigned long now, uint32_t freq, unsigned long ms) {   // 논블로킹 톤 시작
  ledcWriteTone(BUZZER, freq);
  beepEnd = now + ms;
  beeping = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED2, OUTPUT);
  pinMode(BTN_RUN, INPUT_PULLUP);
  pinMode(BTN_MODE, INPUT_PULLUP);
  analogReadResolution(12);
  analogSetPinAttenuation(POT, ADC_11db);
  ledcAttach(BUZZER, 2000, 8);        // 부저 전용 LEDC 채널
  ledcWriteTone(BUZZER, 0);           // 무음
  digitalWrite(LED2, LOW);
  Serial.println("Week09 ready: RUN=GPIO18, MODE=GPIO19");
}

void loop() {
  unsigned long now = millis();

  // 1) 입력 이벤트
  if (runButton.pressed(now)) {
    running = !running;
    Serial.printf("[BTN] RUN=%s\n", running ? "ON" : "OFF");
    beep(now, 2000, 80);
  }
  if (modeButton.pressed(now)) {
    alarmMode = !alarmMode;
    Serial.printf("[BTN] ALARM_MODE=%s\n", alarmMode ? "ON" : "OFF");
    beep(now, 2500, 80);
  }

  // 2) 100ms마다 ADC → LED1 PWM
  if (now - lastSample >= 100) {
    lastSample = now;
    adc = analogRead(POT);
    int pwm = running ? constrain(map(adc, 0, 4095, 0, 255), 0, 255) : 0;
    analogWrite(LED1, pwm);
  }

  // 3) 400ms마다 LED2 점멸 (정지 중에는 꺼짐)
  if (now - lastBlink >= 400) {
    lastBlink = now;
    led2On = running && !led2On;
    digitalWrite(LED2, led2On ? HIGH : LOW);
  }

  // 4) 경보: RUN + MODE ON + ADC > 임계값 → 1초마다 150ms 경고음
  if (running && alarmMode && adc > ALARM_LEVEL && !beeping && now - lastAlarm >= 1000) {
    lastAlarm = now;
    beep(now, 1500, 150);
    Serial.printf("[ALARM] ADC=%d > %d\n", adc, ALARM_LEVEL);
  }

  // 5) 톤 종료 시각 확인
  if (beeping && (long)(now - beepEnd) >= 0) {
    ledcWriteTone(BUZZER, 0);
    beeping = false;
  }

  // 6) 상태 로그 (500ms)
  if (now - lastLog >= 500) {
    lastLog = now;
    Serial.printf("RUN=%d MODE=%d ADC=%4d LED2=%d\n", running, alarmMode, adc, led2On);
  }
}
