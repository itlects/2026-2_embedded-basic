// =====================================================================
// Wokwi 워크북 B — 로직 분석기로 보는 DHT 타이밍 (실습 5와 같은 코드) — 라이브러리 없이 DHT 단일선 타이밍을 직접 해독
//   * 실물 ESP32 + DHT11 : SENSOR_IS_DHT22 = 0
//   * Wokwi  ESP32 + DHT22: SENSOR_IS_DHT22 = 1   (Wokwi에는 DHT22만 있음)
// 배선 : DATA→GPIO4 (10kΩ 풀업 → 3V3), VCC→3V3, GND→GND
// 목적 : ① 시작 신호 ② 응답 80/80µs ③ 비트별 HIGH 길이(≈27µs=0, ≈70µs=1)
//        ④ 40비트 → 5바이트 → 체크섬 검사 → 온도·습도 계산 과정을 눈으로 확인
// 출력 : 시리얼 115200 baud
// =====================================================================
#include <Arduino.h>

#define SENSOR_IS_DHT22 1          // Wokwi = DHT22 (실물 DHT11이면 0)

constexpr uint8_t  DHT_PIN      = 4;
constexpr uint8_t  MARK_PIN     = 13;   // (선택) 로직분석기용 표시 핀: 비트 판정 순간 토글
constexpr uint32_t START_LOW_US = SENSOR_IS_DHT22 ? 1200 : 20000;  // DHT11 ≥18ms, DHT22 ≈1ms
constexpr uint32_t THRESH_US    = 48;   // HIGH 길이가 이보다 길면 1 (0≈26~28µs, 1≈70µs)

uint16_t highUs[40];   // 각 비트의 HIGH 펄스 길이(µs)
uint8_t  data[5];      // 습도정수, 습도소수, 온도정수, 온도소수, 체크섬

// level 상태가 끝날 때까지 기다리고 그 길이(µs)를 반환. 시간 초과면 0.
uint32_t waitWhile(int level, uint32_t timeoutUs) {
  uint32_t t0 = micros();
  while (digitalRead(DHT_PIN) == level) {
    if (micros() - t0 > timeoutUs) return 0;
  }
  return micros() - t0;
}

// 0: 성공, 음수: 실패 단계 번호
int readRaw() {
  memset(data, 0, sizeof(data));

  // ① 시작 신호: MCU가 DATA를 LOW로 끌어내렸다가 놓는다(풀업이 HIGH로 복귀)
  pinMode(DHT_PIN, OUTPUT);
  digitalWrite(DHT_PIN, LOW);
  if (START_LOW_US >= 10000) delay(START_LOW_US / 1000); else delayMicroseconds(START_LOW_US);
  pinMode(DHT_PIN, INPUT_PULLUP);   // 버스를 놓는다(오픈드레인처럼 동작)

  noInterrupts();                   // µs 단위 측정이므로 잠시 인터럽트 차단
  // ② 센서 응답: (HIGH 20~40µs) → LOW 80µs → HIGH 80µs
  if (!waitWhile(HIGH, 200)) { interrupts(); return -1; }
  uint32_t respLow  = waitWhile(LOW, 200);
  uint32_t respHigh = waitWhile(HIGH, 200);
  if (!respLow || !respHigh) { interrupts(); return -2; }

  // ③ 40비트: 각 비트 = LOW ≈50µs + HIGH(26~28µs=0 / ≈70µs=1)
  for (int i = 0; i < 40; i++) {
    if (!waitWhile(LOW, 200)) { interrupts(); return -3; }
    uint32_t hi = waitWhile(HIGH, 200);
    if (!hi) { interrupts(); return -4; }
    highUs[i] = hi;
    digitalWrite(MARK_PIN, !digitalRead(MARK_PIN));       // 판정 순간 표시
    data[i / 8] <<= 1;
    if (hi > THRESH_US) data[i / 8] |= 1;                 // 긴 HIGH = 1
  }
  interrupts();

  Serial.printf("응답 펄스: LOW %lu us, HIGH %lu us (기준 80/80)\n",
                (unsigned long)respLow, (unsigned long)respHigh);
  return 0;
}

void printFrame() {
  Serial.println("비트별 HIGH 길이(µs)와 판정 [바이트 단위]");
  for (int b = 0; b < 5; b++) {
    Serial.printf("  byte%d: ", b);
    for (int k = 0; k < 8; k++) {
      int i = b * 8 + k;
      Serial.printf("%2u(%d) ", highUs[i], highUs[i] > THRESH_US ? 1 : 0);
    }
    Serial.printf(" = 0x%02X (%3u)\n", data[b], data[b]);
  }
  uint8_t sum = (uint8_t)(data[0] + data[1] + data[2] + data[3]);  // 하위 8비트만
  Serial.printf("체크섬: %u+%u+%u+%u = 0x%02X, 수신 0x%02X → %s\n",
                data[0], data[1], data[2], data[3], sum, data[4],
                sum == data[4] ? "OK" : "FAIL");
  if (sum != data[4]) return;

#if SENSOR_IS_DHT22
  // DHT22: 16비트 값 ×0.1, 온도 최상위비트=부호
  float h = ((data[0] << 8) | data[1]) * 0.1f;
  int16_t rawT = ((data[2] & 0x7F) << 8) | data[3];
  float t = rawT * 0.1f * ((data[2] & 0x80) ? -1 : 1);
#else
  // DHT11: 정수부 + 소수부(대부분 0)
  float h = data[0] + data[1] * 0.1f;
  float t = (data[2] & 0x7F) + data[3] * 0.1f;
  if (data[2] & 0x80) t = -t;
#endif
  Serial.printf("결과 → 온도 %.1f °C, 습도 %.1f %%RH\n\n", t, h);
}

void setup() {
  Serial.begin(115200);
  pinMode(MARK_PIN, OUTPUT);
  pinMode(DHT_PIN, INPUT_PULLUP);
  delay(1500);   // 전원 인가 후 센서 안정화(≥1초)
  Serial.println(SENSOR_IS_DHT22 ? "DHT22 raw decoder" : "DHT11 raw decoder");
}

void loop() {
  int r = readRaw();
  if (r == 0) printFrame();
  else Serial.printf("읽기 실패(단계 %d): 배선·풀업·센서 종류 설정 확인\n\n", r);
  delay(3000);
}
