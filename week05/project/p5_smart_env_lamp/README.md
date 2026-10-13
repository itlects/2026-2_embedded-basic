# 5주차 종합과제 — 스마트 온습도 경보 무드등

DHT11 로 온도·습도를 2초마다 측정해 상태 LED(저온·쾌적·고온), 습도 막대 LED, 고온 경보 사이렌을 제어하고, 가변저항(ADC)으로 밝기와 경보 임계값을 조절하며, 측정값과 동작 모드를 I2C LCD 에 표시하는 무드등.

단계별 워크북(배치도·확인표 포함): [노션 — 수업과제-스마트온습도경보무드등-요구사항](https://app.notion.com/p/3f1b2e6bf807813daad0d6443efa6d21)

## 파일

| 폴더 | 내용 |
|---|---|
| `check_uno/`, `check_esp32/` | 2단계 회로 점검 스케치 (LED 순서·부저·가변저항·버튼·LCD·온습도 확인) |
| `uno/`, `esp32/` | 3단계 완성 코드 |

각 폴더: `sketch.ino` + Wokwi `diagram.json` + `libraries.txt`

## 부품

UNO 또는 ESP32 DevKitC V4 1, 가변저항 1kΩ 1, DHT11 1, LED 6개(빨강 2, 초록 1, 파랑 3), 저항 330Ω 6, 푸시버튼 4, 수동 부저 1, I2C LCD 16x2(PCF8574) 1

## 핀 배치

| 기능 | UNO | ESP32 |
|---|---|---|
| 가변저항 Wiper (ADC) | A0 | GPIO34 |
| DHT11 DATA | D4 | GPIO4 |
| 상태 LED 빨강 / 초록 / 파랑 (PWM) | D9 / D6 / D5 | GPIO25 / 26 / 27 |
| 습도바 1(파랑) / 2(파랑) / 3(빨강) | D7 / D8 / D13 | GPIO16 / 17 / 18 |
| 수동 부저 (+) | D11 | GPIO23 |
| BTN_MODE (NORMAL ↔ SET) | D2 | GPIO13 |
| BTN_ALARM (경보음 ON/OFF) | D3 | GPIO14 |
| BTN_REPORT (즉시 측정) | D10 | GPIO32 |
| BTN_RESET (초기화) | D12 | GPIO33 |
| I2C LCD SDA / SCL | A4 / A5 | GPIO21 / GPIO22 |
| I2C LCD VCC | 5V | 5V (VIN) |
| 가변저항·DHT11 전원 | 5V | **3.3V** (5V 금지) |

- 버튼은 모두 INPUT_PULLUP: 한쪽은 핀, 다른 쪽은 GND (누르면 LOW)
- UNO 의 D3·D11 은 tone()(Timer2) 때문에 PWM 을 쓸 수 없어 버튼·부저로 배정했다.
- LCD 주소는 보통 0x27 (일부 0x3F).

## SENSOR_MODE

| 실행 환경 | SENSOR_MODE | 센서 |
|---|---|---|
| 실물 회로 | 0 | DHT11 |
| Wokwi | 1 (기본값) | DHT22 (Wokwi 에는 DHT11 이 없음) |
| Tinkercad (UNO 만) | 2 | TMP36 (Vout → A1), Adafruit_LiquidCrystal |

라이브러리: DHT sensor library (Adafruit), Adafruit Unified Sensor, LiquidCrystal I2C

## 요구사항

| 기능 | 동작 |
|---|---|
| 상태 LED | 저온(< 20 °C) 파랑 점등 / 쾌적 초록 점등 / 고온(≥ 임계값) 빨강 1초 주기 점멸 / 센서오류 파랑 점멸 / SET 모드 보라 빠른 점멸 |
| 습도 막대 | < 40 % 1개, 40~60 % 2개, > 60 % 3개, 오류 시 모두 꺼짐 |
| 사이렌 | NORMAL + 고온 + 경보 ON 일 때 880/660 Hz 를 0.3초마다 교대 |
| 가변저항 | NORMAL: 밝기 PWM 20~255 / SET: 고온 임계값 20.0~35.0 °C (0.5 °C 단위, 31단계 재양자화) |
| 버튼 4개 | 모드 / 경보 ON·OFF / 즉시 측정 / 초기화 (임계 28.0, 경보 ON, NORMAL) |
| LCD | 1행 `T24.0C H55% OK`, 2행 `Th28.0 ALM:ON`, 버튼 메시지 1.5초 |
| 구조 | loop() 안에서 delay() 금지 (millis() 논블로킹) |

## 제출물

| 제출물 | 내용 |
|---|---|
| Wokwi 공유 링크 (또는 Tinkercad 링크) | 완성 코드가 동작하는 시뮬레이션 |
| 시리얼 로그 캡처 | 저온·쾌적·고온 리포트와 SET 모드 이벤트 줄이 보이도록 |
| 실물 회로 사진 + 30초 시연 영상 | 고온 경보 → BTN_ALARM 으로 끄기 → BTN_RESET 까지 |
| 워크북 결과 | 각 단계 확인 표(☑)와 11단계 시뮬레이션 vs 실물 비교 기록표 |

## 최종 확인 체크리스트

- [ ] 점검 스케치에서 LED·부저·가변저항·버튼·LCD·온습도 모두 정상
- [ ] LCD 화면 6가지와 버튼 메시지 4가지 확인
- [ ] 상태 LED 4가지(저온·쾌적·고온·센서오류) 확인
- [ ] 습도 막대 3단계 확인
- [ ] 밝기 표·임계값 표와 실제 값 비교
- [ ] 사이렌 ON/OFF 조건 확인
- [ ] 버튼 4개 동작 확인
- [ ] 시뮬레이션 vs 실물 비교 기록표 작성
