/*
 * 스마트팜 T2 — 온습도(DHT) 읽기 + 환기팬(초록 LED) 히스테리시스 테스트
 * 팬 ON  : 온도 ≥ 28.0 °C 또는 습도 ≥ 70 %
 * 팬 OFF : 온도 < 27.0 °C 그리고 습도 < 65 %   (켜짐/꺼짐 기준을 다르게 → 떨림 방지)
 */
#include <DHT.h>
#define DHT_TYPE DHT22            // 실물 DHT11 사용 시 DHT11
#if defined(ESP32)
const int POT_PIN = 34, DHT_PIN = 4;
const int GROW_R = 25, GROW_B = 26, PUMP_LED = 16, FAN_LED = 17, ALARM_LED = 18;
const int BUZZER_PIN = 23;
const int BTN_PINS[4] = {13, 14, 32, 33};        // MODE, PUMP, FAN, MUTE
const int ADC_MAX = 4095;
#else
const int POT_PIN = A0, DHT_PIN = 4;
const int GROW_R = 9, GROW_B = 10, PUMP_LED = 7, FAN_LED = 8, ALARM_LED = 13;
const int BUZZER_PIN = 11;
const int BTN_PINS[4] = {2, 3, 5, 6};
const int ADC_MAX = 1023;
#endif

DHT dht(DHT_PIN, DHT_TYPE);
const float FAN_ON_T = 28.0, FAN_OFF_T = 27.0;
const float FAN_ON_H = 70.0, FAN_OFF_H = 65.0;
bool fanOn = false;
unsigned long lastDhtMs = 0;

void setup() {
  Serial.begin(115200);
  pinMode(FAN_LED, OUTPUT);
  dht.begin();
  Serial.println(F("# T2 DHT + 팬 테스트 (2초마다 측정)"));
}

void loop() {
  if (millis() - lastDhtMs < 2000) return;        // DHT 는 2초 간격
  lastDhtMs = millis();
  float h = dht.readHumidity(), t = dht.readTemperature();
  if (isnan(t) || isnan(h)) { Serial.println(F("센서 오류: 배선·라이브러리·DHT_TYPE 확인")); return; }

  if (!fanOn && (t >= FAN_ON_T || h >= FAN_ON_H))      { fanOn = true;  Serial.println(F("# 팬 ON")); }
  else if (fanOn && t < FAN_OFF_T && h < FAN_OFF_H)    { fanOn = false; Serial.println(F("# 팬 OFF")); }
  digitalWrite(FAN_LED, fanOn ? HIGH : LOW);

  Serial.print(F("T=")); Serial.print(t, 1); Serial.print(F("C H=")); Serial.print(h, 0);
  Serial.print(F("% 팬=")); Serial.println(fanOn ? F("ON") : F("OFF"));
}
