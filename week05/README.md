# Week05 — 아날로그 입력(ADC): 샘플링 · 양자화 · 가변램프 제어

> **강의계획서 5주차:** 아날로그 입력(ADC)에 대해 샘플링, 양자화 원리를 이해하고 아날로그 값을 디지털로 변환, 활용할 수 있다.
> 대상 보드: Arduino UNO R3 / ESP32 DevKitC V4 (Arduino-ESP32 코어 **3.x**)

학습자료(노션): [강의5주_학습자료](https://app.notion.com/p/3eab2e6bf80780afb272f967a82f8224)

## 폴더 구성

| 폴더 | 내용 |
|---|---|
| [examples/ex5-1_adc_sampling](examples/ex5-1_adc_sampling) | 실습 5-1 ADC 기본값 & 샘플링 주기 |
| [examples/ex5-2_quantization](examples/ex5-2_quantization) | 실습 5-2 전압 환산 & 양자화 |
| [labs/lab5-3_variable_lamp](labs/lab5-3_variable_lamp) | 실습 5-3 ADC → PWM 가변램프 (선형/감마) |
| [labs/lab5-4_level_meter](labs/lab5-4_level_meter) | 실습 5-4 LED 레벨미터 + 경고음 (재양자화·히스테리시스) |
| [labs/lab5-5_analog_monitor](labs/lab5-5_analog_monitor) | 실습 5-5 통합 Analog Monitor (LAMP / COLOR / THEREMIN) |
| [project/p5_smart_env_lamp](project/p5_smart_env_lamp) | 종합과제 — 스마트 온습도 경보 무드등 |
| [project/p5b_smart_farm](project/p5b_smart_farm) | 수업과제 2 — 스마트팜 워크북 |
| [project/p5c_tunnel_lamp](project/p5c_tunnel_lamp) | 수업과제 3 — 터널 멜로디 거리 램프 워크북 |

- 실습 5-1 ~ 5-5: 보드별 `uno/`, `esp32/` 폴더에 `sketch.ino` + Wokwi `diagram.json`
- 과제: 단계별 폴더에 `sketch.ino` + `diagram.json`(UNO) + `diagram_esp32.json`(ESP32) + `libraries.txt`

## 공통 핀 맵

| 기능 | Arduino UNO | ESP32 DevKitC V4 |
|---|---|---|
| 가변저항 Wiper (ADC) | A0 | GPIO34 (ADC1, 입력 전용) |
| 가변저항 전원 | 5V | **3.3V** (5V 금지) |
| 모드 버튼 | D2 | GPIO13 |
| 가변램프 LED (5-3) | D9 | GPIO25 |
| RGB LED (5-5, 과제) | R D9 / G D6 / B D5 | R 25 / G 26 / B 27 |
| 수동 부저 | D11 | GPIO23 |
| DHT11 (과제) | D4 | GPIO4 |

## Wokwi 사용법

1. wokwi.com → New Project → Arduino Uno 또는 ESP32
2. `sketch.ino` 탭을 이 저장소의 `sketch.ino` 로 교체
3. `diagram.json` 탭을 이 저장소의 `diagram.json` 으로 교체 (과제의 ESP32 는 `diagram_esp32.json` 내용을 붙여넣기)
4. DHT·LCD 를 쓰는 과제는 Library Manager 에 `libraries.txt` 의 라이브러리를 추가
5. Wokwi 에는 DHT11 이 없어 DHT22 로 대체한다 (코드 기본값)

## 검증

- 모든 스케치(25개)를 `arduino-cli` 로 UNO(avr)·ESP32(esp32 3.3.0) 컴파일 확인 (UNO·ESP32 공용 스케치는 두 보드 모두, 총 36회 빌드 통과)
- 과제 2·3 과 종합과제 코드는 노션 워크북에 실린 코드와 동일하다.
