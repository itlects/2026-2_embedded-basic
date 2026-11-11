// =====================================================================
// 10주차 실습 3 — DHT11 + 수동 부저(GPIO25) + LED(GPIO26) + CSV 로깅
// 보드 : ESP32-DevKitC V4 (Arduino IDE 보드 선택: "ESP32 Dev Module")
// 배선 : DHT11 DATA→GPIO4, VCC→3V3, GND→GND
//        부저(+)→GPIO25, 부저(−)→GND
//        GPIO26 → 330Ω → LED 애노드(+, 긴 다리) / LED 캐소드(−) → GND
// 라이브러리 : DHT sensor library (Adafruit) + Adafruit Unified Sensor
// 출력 : 시리얼 115200 baud, CSV 형식 → 시리얼 플로터/엑셀로 분석 가능
// =====================================================================
#include <DHT.h>

constexpr uint8_t DHT_PIN    = 4;
constexpr uint8_t BUZZER_PIN = 25;
constexpr uint8_t LED_PIN    = 26;

// 히스테리시스: 켜는 기준(ON)과 끄는 기준(OFF)을 다르게 둔다
constexpr float TEMP_ON = 30.0f, TEMP_OFF = 28.0f;   // °C
constexpr float HUM_ON  = 75.0f, HUM_OFF  = 70.0f;   // %RH

constexpr uint32_t READ_MS   = 2000;   // DHT11은 1초 이상 간격 필요 → 2초
constexpr uint32_t BEEP_MS   = 120;    // 삑 소리 길이
constexpr uint32_t PERIOD_MS = 600;    // 삑 반복 주기
constexpr uint16_t BEEP_HZ   = 2000;   // 수동 부저 주파수

DHT dht(DHT_PIN, DHT11);

bool     alarmOn   = false;   // 현재 경보 상태
bool     beeping   = false;   // 부저가 지금 울리는 중인가
uint32_t lastRead  = 0;
uint32_t beepStart = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(BUZZER_PIN, OUTPUT);
  dht.begin();
  delay(1000);                                 // 센서 안정화
  Serial.println("ms,tempC,humidity,alert");   // CSV 머리줄
}

// ---- 1) 2초마다 측정·판단 -----------------------------------------
void readAndDecide(uint32_t now) {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {                  // 읽기 실패: 경보 해제, 오류 기록
    alarmOn = false;
    Serial.printf("%lu,nan,nan,ERROR\n", (unsigned long)now);
    return;
  }

  if (!alarmOn && (t >= TEMP_ON || h >= HUM_ON)) {          // 하나라도 넘으면 ON
    alarmOn = true;
  } else if (alarmOn && t <= TEMP_OFF && h <= HUM_OFF) {    // 둘 다 충분히 내려가야 OFF
    alarmOn = false;
  }

  Serial.printf("%lu,%.1f,%.1f,%d\n", (unsigned long)now, t, h, alarmOn ? 1 : 0);
}

// ---- 2) 출력 갱신(매 loop, 멈추지 않음) ---------------------------
void updateOutputs(uint32_t now) {
  digitalWrite(LED_PIN, alarmOn ? HIGH : LOW);

  if (!alarmOn) {
    if (beeping) { noTone(BUZZER_PIN); beeping = false; }
    return;
  }
  if (!beeping && now - beepStart >= PERIOD_MS) {   // 삑 시작
    tone(BUZZER_PIN, BEEP_HZ);
    beeping = true;
    beepStart = now;
  } else if (beeping && now - beepStart >= BEEP_MS) {  // 삑 끝
    noTone(BUZZER_PIN);
    beeping = false;
  }
}

void loop() {
  uint32_t now = millis();
  if (now - lastRead >= READ_MS) {
    lastRead = now;
    readAndDecide(now);
  }
  updateOutputs(now);
}
