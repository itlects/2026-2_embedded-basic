# 8주차 — 중간고사 1차 수행평가 (실습)

버튼 4개로 LED 4개와 수동 부저를 제어하는 시스템을 배선하고 프로그램한다.
시험 문제(상세)는 Notion의 **중간평가문제** 페이지를 기준으로 한다.

## 폴더 구성

| 경로 | 내용 |
|---|---|
| `images/midterm_parts_guide_ko` | 부품 확인: LED·저항·택트 스위치·수동 부저 극성, 브레드보드 내부 연결 |
| `images/midterm_uno_schematic_ko` | Arduino UNO 회로 연결도 |
| `images/midterm_uno_breadboard_ko` | Arduino UNO 브레드보드 배선도 (구멍 좌표 표기) |
| `images/midterm_esp32_schematic_ko` | ESP32 DevKitC V4 회로 연결도 |
| `images/midterm_esp32_breadboard_ko` | ESP32 DevKitC V4 브레드보드 배선도 (구멍 좌표 표기) |
| `images/midterm_timing_ko` | 동작 요구사항을 시간 흐름(250ms 단위)으로 표시 |
| `wiring_test/midterm_wiring_test.ino` | 배선 확인용 테스트 스케치 (채점 대상 아님) |
| `starter/midterm_starter/midterm_starter.ino` | 시험용 시작 코드 (정답 아님) |

그림은 `.svg`(원본)와 `.png`(인쇄·문서용)를 함께 제공한다. `images/gen_figs.py`로 다시 만들 수 있다.

## 핀 요약

| 장치 | Arduino UNO R3 | ESP32 DevKitC V4 |
|---|---|---|
| LED1~LED4 | D3, D4, D5, D6 | GPIO16, 17, 18, 19 |
| 버튼1~버튼4 | D7, D8, D9, D10 | GPIO21, 22, 23, 25 |
| 수동 부저 | D11 | GPIO26 |

- LED: GPIO → 220~330Ω → LED 긴 다리(+), 짧은 다리(−) → GND
- 버튼: GPIO ↔ 버튼(대각선 다리) ↔ GND, `INPUT_PULLUP` (누르면 LOW)
- ESP32는 보드 매니저 esp32 3.x 코어 사용, GPIO에 5V 인가 금지
