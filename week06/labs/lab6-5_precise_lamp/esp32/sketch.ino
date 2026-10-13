/*
 * 실습 6-5  타이머 기반 정밀 가변램프  —  ESP32 DevKitC V4
 * ------------------------------------------------------------------
 * 회로 : GPIO25 → 330Ω → LED1(+) → GND         램프 LED (LEDC PWM)
 *        GPIO26 → 330Ω → LED2(+) → GND         심장박동 LED (타이머 틱으로 0.5 s 토글)
 *        가변저항: 양 끝 3.3V / GND, Wiper → GPIO34   (5V 연결 금지)
 *        GPIO13 ↔ 푸시버튼 ↔ GND               PWM 주파수 변경 (2 → 10 → 50 → 500 Hz)
 *
 * 타이머 자원
 *   하드웨어 타이머 0 : 10 ms 알람 → 샘플링 태스크를 깨움 (100 Hz 샘플링, 50번마다 LED2 토글)
 *   LEDC              : 램프 LED PWM (주파수마다 해상도를 바꿔 설정)
 *
 * 요구사항
 *   1) 가변저항을 정확히 100 Hz 로 샘플링하고 최근 8개 평균으로 잡음을 줄인다.
 *   2) 평균값으로 LED1 듀티를 바꾼다.
 *   3) 버튼으로 PWM 주파수를 바꿔 2 Hz 에서는 깜빡임, 500 Hz 에서는 밝기로 보이는지 확인한다.
 *   4) LED2 는 loop 가 아무리 바빠도 정확히 0.5 s 마다 토글된다.
 *   5) loop() 에서 delay() 사용 금지. 500 ms 마다 상태를 시리얼로 출력한다.
 *
 * 코어 : Arduino-ESP32 3.x
 */

const int LAMP_PIN = 25;
const int BEAT_PIN = 26;
const int POT_PIN  = 34;
const int BTN_PIN  = 13;

struct PwmSetting { uint32_t freq; uint8_t res; };
const PwmSetting SETTINGS[] = { {2, 16}, {10, 14}, {50, 11}, {500, 10} };
const int SETTING_COUNT = sizeof(SETTINGS) / sizeof(SETTINGS[0]);
int setNo = 3;                                   // 500 Hz 에서 시작

hw_timer_t*  sampleTimer = nullptr;
TaskHandle_t sampleTaskHandle = nullptr;
portMUX_TYPE dataMux = portMUX_INITIALIZER_UNLOCKED;

const int AVG_N = 8;
// 아래 변수들은 dataMux(임계구역)로 보호한다
long     sharedSum = 0;
bool     newSample = false;
uint32_t sampleCount = 0;

void ARDUINO_ISR_ATTR onTimer() {
  BaseType_t woken = pdFALSE;
  vTaskNotifyGiveFromISR(sampleTaskHandle, &woken);
  portYIELD_FROM_ISR(woken);
}

void sampleTask(void* arg) {
  int  buf[AVG_N] = {0};
  int  idx = 0;
  long sum = 0;
  int  beatTick = 0;
  bool beat = false;
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);     // 10 ms 마다 깨어남
    int adc = analogRead(POT_PIN);
    sum += adc - buf[idx];
    buf[idx] = adc;
    idx = (idx + 1) % AVG_N;

    portENTER_CRITICAL(&dataMux);
    sharedSum = sum;
    newSample = true;
    sampleCount++;
    portEXIT_CRITICAL(&dataMux);

    if (++beatTick >= 50) {                      // 50 × 10 ms = 0.5 s
      beatTick = 0;
      beat = !beat;
      digitalWrite(BEAT_PIN, beat);
    }
  }
}

const unsigned long DEBOUNCE_MS = 30;
int lastReading = HIGH, stableState = HIGH;
unsigned long lastChangeMs = 0;

bool buttonPressed() {
  int reading = digitalRead(BTN_PIN);
  if (reading != lastReading) { lastChangeMs = millis(); lastReading = reading; }
  if (millis() - lastChangeMs >= DEBOUNCE_MS && reading != stableState) {
    stableState = reading;
    return stableState == LOW;
  }
  return false;
}

int avgAdc = 0;
uint32_t dutyRaw = 0;
unsigned long lastPrintMs = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BEAT_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  analogReadResolution(12);

  ledcAttach(LAMP_PIN, SETTINGS[setNo].freq, SETTINGS[setNo].res);

  xTaskCreatePinnedToCore(sampleTask, "sample", 4096, nullptr, 5,
                          &sampleTaskHandle, 1);
  sampleTimer = timerBegin(1000000);             // 1 MHz
  timerAttachInterrupt(sampleTimer, &onTimer);
  timerAlarm(sampleTimer, 10000, true, 0);       // 10 ms 반복

  Serial.println("Lab6-5 Precise Lamp (ESP32)");
}

void loop() {
  if (buttonPressed()) {
    setNo = (setNo + 1) % SETTING_COUNT;
    ledcChangeFrequency(LAMP_PIN, SETTINGS[setNo].freq, SETTINGS[setNo].res);
  }

  bool fresh = false;
  long sum = 0;
  portENTER_CRITICAL(&dataMux);
  if (newSample) { sum = sharedSum; newSample = false; fresh = true; }
  portEXIT_CRITICAL(&dataMux);

  if (fresh) {
    avgAdc = sum / AVG_N;
    uint32_t maxDuty = (1UL << SETTINGS[setNo].res) - 1;
    dutyRaw = (uint32_t)((uint64_t)maxDuty * avgAdc / 4095);
    ledcWrite(LAMP_PIN, dutyRaw);
  }

  if (millis() - lastPrintMs >= 500) {
    lastPrintMs = millis();
    portENTER_CRITICAL(&dataMux);
    uint32_t cnt = sampleCount;
    sampleCount = 0;
    portEXIT_CRITICAL(&dataMux);

    uint32_t maxDuty = (1UL << SETTINGS[setNo].res) - 1;
    Serial.printf("f=%luHz ADC=%d res=%ubit duty=%lu/%lu Duty=%.1f%% Samples/0.5s=%lu\n",
                  (unsigned long)SETTINGS[setNo].freq, avgAdc, SETTINGS[setNo].res,
                  (unsigned long)dutyRaw, (unsigned long)maxDuty,
                  dutyRaw * 100.0 / maxDuty, (unsigned long)cnt);
  }
}
