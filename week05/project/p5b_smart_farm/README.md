# 수업과제 2 — 스마트팜 워크북

토양 수분(가변저항 = 아날로그 센서), 온습도(DHT11)로 펌프·환기팬을 자동 제어하고, 생장 LED 로 일출·일몰을 흉내 내며, 물이 부족하면 경보음을 울리고, 모든 상태를 I2C LCD 에 표시한다.

단계별 설명·확인표: [노션 — 수업과제2-스마트팜-워크북](https://app.notion.com/p/3f1b2e6bf807812ea339f86c95971d32)

모든 스케치는 **UNO·ESP32 공용**이다 (`#if defined(ESP32)` 로 핀 자동 선택).

## 단계 폴더

| 폴더 | 단계 | 확인 내용 |
|---|---|---|
| `t1_soil` | T1 | 토양 수분 ADC → % (보정 DRY_ADC / WET_ADC) |
| `t2_dht_fan` | T2 | DHT 2초 측정 + 환기팬 히스테리시스 (ON ≥ 28 °C 또는 ≥ 70 %, OFF < 27 °C 그리고 < 65 %) |
| `t3_pump` | T3 | 자동 급수 펌프 히스테리시스 (ON < 30 %, OFF ≥ 50 %) |
| `t4_grow_light` | T4 | 생장 LED 일출·일몰 PWM (하루 = 40초) |
| `t5_alarm` | T5 | 물 부족(< 15 %) 경보 LED 점멸 + 4초마다 솔-미-도, 음소거 |
| `t6_lcd_buttons` | T6 | I2C LCD + 버튼 4개 |
| `final` | 완성 | T1~T6 통합, AUTO / MANUAL 모드 |

각 폴더: `sketch.ino`, `diagram.json`(UNO), `diagram_esp32.json`(ESP32 — Wokwi 에 붙여넣을 때 이름을 diagram.json 으로), 필요한 경우 `libraries.txt`

## 핀 배치

| 기능 | UNO | ESP32 |
|---|---|---|
| 토양 수분 (ADC) | A0 | GPIO34 |
| DHT DATA | D4 | GPIO4 |
| 생장 LED 빨강 / 파랑 (PWM) | D9 / D10 | GPIO25 / 26 |
| 펌프 LED (파랑) / 팬 LED (초록) / 경보 LED (빨강) | D7 / D8 / D13 | GPIO16 / 17 / 18 |
| 수동 부저 | D11 | GPIO23 |
| BTN_MODE / PUMP / FAN / MUTE | D2 / D3 / D5 / D6 | GPIO13 / 14 / 32 / 33 |
| I2C LCD SDA / SCL | A4 / A5 | GPIO21 / 22 |
| VCC (가변저항·DHT) | 5V | **3.3V** |
| VCC (LCD) | 5V | 5V (VIN) |

- Wokwi 에는 DHT11 이 없어 `#define DHT_TYPE DHT22` 가 기본값이다. 실물 DHT11 은 `DHT11` 로 바꾼다.
- 실제 펌프·팬은 릴레이·트랜지스터로 구동한다. 이 워크북은 LED 로 상태만 표시한다.
- 라이브러리: DHT sensor library, Adafruit Unified Sensor, LiquidCrystal I2C
