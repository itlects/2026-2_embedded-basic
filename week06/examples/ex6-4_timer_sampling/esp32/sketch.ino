/*
 * 예제 6-4  하드웨어 타이머 인터럽트로 "정확한 샘플링 주기" 만들기  —  ESP32 DevKitC V4
 * ------------------------------------------------------------------
 * 회로 : 가변저항  양 끝 3.3V / GND, 가운데(Wiper) → GPIO34   (5V 연결 금지)
 *        GPIO13 ↔ 푸시버튼 ↔ GND         (loop 부하 ON/OFF)
 *        GPIO2  내장 LED                   (부하 ON 표시, 배선 불필요)
 *
 * 비교하는 두 가지 샘플링 방식 (목표 주기 10 ms = 100 Hz)
 *   (A) 하드웨어 타이머 : 10 ms 마다 ISR → 샘플링 태스크를 깨움 → ADC 읽기
 *   (B) loop + millis() : 5주차 방식. loop 가 바쁘면 검사가 늦어진다
 *
 * 하드웨어 타이머 API (Arduino-ESP32 코어 3.x)
 *   timerBegin(1000000)                 : 타이머 클록 1 MHz (1 tick = 1 µs)
 *   timerAttachInterrupt(t, onTimer)    : 알람 시 호출할 ISR 등록
 *   timerAlarm(t, 10000, true, 0)       : 10,000 tick(10 ms)마다 반복 알람
 *
 * 왜 ISR 에서 바로 analogRead() 하지 않나?
 *   ESP32 코어 3.x 의 analogRead() 는 내부에서 잠금(mutex)을 사용하므로 ISR 에서 호출하면 안 된다.
 *   그래서 ISR 은 "깨우기"만 하고, 우선순위가 높은 FreeRTOS 태스크가 실제로 읽는다.
 *   (ISR 은 짧게 → 7주차 인터럽트에서 다시 다룬다)
 */

const int POT_PIN  = 34;
const int BTN_PIN  = 13;
const int LOAD_LED = 2;                 // DevKitC 내장 LED (보드에 따라 없을 수 있음)

const uint32_t SAMPLE_US = 10000;       // 10 ms

hw_timer_t*   sampleTimer = nullptr;
TaskHandle_t  sampleTaskHandle = nullptr;
portMUX_TYPE  statMux = portMUX_INITIALIZER_UNLOCKED;

// ---------- 샘플링 결과 (태스크가 쓰고 loop 가 읽음, statMux 로 보호) ----------
int      tAdc = 0;
uint32_t tMinUs = UINT32_MAX, tMaxUs = 0, tCount = 0;

void ARDUINO_ISR_ATTR onTimer() {       // 10 ms 마다 하드웨어가 호출
  BaseType_t woken = pdFALSE;
  vTaskNotifyGiveFromISR(sampleTaskHandle, &woken);
  portYIELD_FROM_ISR(woken);
}

void sampleTask(void* arg) {
  uint32_t lastUs = 0;
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   // ISR 이 깨울 때까지 대기
    uint32_t now = micros();
    int adc = analogRead(POT_PIN);

    portENTER_CRITICAL(&statMux);
    tAdc = adc;
    if (lastUs != 0) {
      uint32_t dt = now - lastUs;
      if (dt < tMinUs) tMinUs = dt;
      if (dt > tMaxUs) tMaxUs = dt;
    }
    tCount++;
    portEXIT_CRITICAL(&statMux);
    lastUs = now;
  }
}

// ---------- (B) loop + millis 방식 : 타이밍만 측정 ----------
unsigned long loopLastMs = 0;
uint32_t loopLastUs = 0, loopMinUs = UINT32_MAX, loopMaxUs = 0;

const unsigned long DEBOUNCE_MS = 30;
int lastReading = HIGH, stableState = HIGH;
unsigned long lastChangeMs = 0;
bool heavyLoad = false;

bool buttonPressed() {
  int reading = digitalRead(BTN_PIN);
  if (reading != lastReading) { lastChangeMs = millis(); lastReading = reading; }
  if (millis() - lastChangeMs >= DEBOUNCE_MS && reading != stableState) {
    stableState = reading;
    return stableState == LOW;
  }
  return false;
}

unsigned long lastReportMs = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(LOAD_LED, OUTPUT);
  analogReadResolution(12);

  // loop(우선순위 1)보다 높은 우선순위 5, loop 와 같은 코어 1 에서 실행
  xTaskCreatePinnedToCore(sampleTask, "sample", 4096, nullptr, 5,
                          &sampleTaskHandle, 1);

  sampleTimer = timerBegin(1000000);                  // 1 MHz
  timerAttachInterrupt(sampleTimer, &onTimer);
  timerAlarm(sampleTimer, SAMPLE_US, true, 0);        // 10 ms 반복
}

void loop() {
  if (buttonPressed()) {
    heavyLoad = !heavyLoad;
    digitalWrite(LOAD_LED, heavyLoad);
  }

  if (millis() - loopLastMs >= SAMPLE_US / 1000) {
    loopLastMs = millis();
    uint32_t now = micros();
    if (loopLastUs != 0) {
      uint32_t dt = now - loopLastUs;
      if (dt < loopMinUs) loopMinUs = dt;
      if (dt > loopMaxUs) loopMaxUs = dt;
    }
    loopLastUs = now;
  }

  if (heavyLoad) delayMicroseconds(7000);     // 무거운 작업 흉내

  if (millis() - lastReportMs >= 1000) {
    lastReportMs = millis();

    portENTER_CRITICAL(&statMux);
    uint32_t mn = tMinUs, mx = tMaxUs, cnt = tCount;
    int adc = tAdc;
    tMinUs = UINT32_MAX; tMaxUs = 0; tCount = 0;
    portEXIT_CRITICAL(&statMux);

    Serial.printf("Target:%lu,TimerMin:%lu,TimerMax:%lu,LoopMin:%lu,LoopMax:%lu,"
                  "Samples:%lu,ADC:%d,Load:%d\n",
                  (unsigned long)SAMPLE_US, (unsigned long)mn, (unsigned long)mx,
                  (unsigned long)loopMinUs, (unsigned long)loopMaxUs,
                  (unsigned long)cnt, adc, heavyLoad ? 1 : 0);
    loopMinUs = UINT32_MAX; loopMaxUs = 0;
  }
}
