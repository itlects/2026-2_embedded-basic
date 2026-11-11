// =====================================================================
// Wokwi 워크북 A — DHT22(=DHT11 대체) 온습도 경보: 부저 GPIO25 + LED GPIO26
// Wokwi에는 DHT11 부품이 없으므로 같은 단일선 프로토콜의 DHT22로 연습한다.
// 실물 DHT11로 옮길 때는 SENSOR_TYPE 한 줄만 DHTesp::DHT11 로 바꾼다.
// 시뮬레이션 중 DHT22를 클릭 → 온도/습도 슬라이더로 경보 조건을 만든다.
// 라이브러리(libraries.txt): DHT sensor library for ESPx
// =====================================================================
#include "DHTesp.h"

constexpr uint8_t DHT_PIN    = 4;
constexpr uint8_t BUZZER_PIN = 25;
constexpr uint8_t LED_PIN    = 26;
#define SENSOR_TYPE DHTesp::DHT22          // 실물 DHT11: DHTesp::DHT11

constexpr float TEMP_ON = 30.0f, TEMP_OFF = 28.0f;
constexpr float HUM_ON  = 75.0f, HUM_OFF  = 70.0f;
constexpr uint32_t READ_MS = 2000, BEEP_MS = 120, PERIOD_MS = 600;

DHTesp dht;
bool alarmOn = false, beeping = false;
uint32_t lastRead = 0, beepStart = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  dht.setup(DHT_PIN, SENSOR_TYPE);
  Serial.println("ms,tempC,humidity,alert");
}

void readAndDecide(uint32_t now) {
  TempAndHumidity v = dht.getTempAndHumidity();
  if (dht.getStatus() != DHTesp::ERROR_NONE || isnan(v.temperature)) {
    alarmOn = false;
    Serial.printf("%lu,nan,nan,ERROR(%s)\n", (unsigned long)now, dht.getStatusString());
    return;
  }
  float t = v.temperature, h = v.humidity;
  if (!alarmOn && (t >= TEMP_ON || h >= HUM_ON))        alarmOn = true;   // OR 로 켜고
  else if (alarmOn && t <= TEMP_OFF && h <= HUM_OFF)    alarmOn = false;  // AND 로 끈다
  Serial.printf("%lu,%.1f,%.1f,%d\n", (unsigned long)now, t, h, alarmOn);
}

void updateOutputs(uint32_t now) {
  digitalWrite(LED_PIN, alarmOn);
  if (!alarmOn) { if (beeping) { noTone(BUZZER_PIN); beeping = false; } return; }
  if (!beeping && now - beepStart >= PERIOD_MS) { tone(BUZZER_PIN, 2000); beeping = true; beepStart = now; }
  else if (beeping && now - beepStart >= BEEP_MS) { noTone(BUZZER_PIN); beeping = false; }
}

void loop() {
  uint32_t now = millis();
  if (now - lastRead >= READ_MS) { lastRead = now; readAndDecide(now); }
  updateOutputs(now);
  delay(10);   // Wokwi 시뮬레이션 부하를 줄인다(실물에서는 지워도 됨)
}
