# 실습 5-5 — 통합 Analog Monitor (3모드)

같은 ADC 값을 모드에 따라 LAMP(3색 동일 밝기), COLOR(파랑→초록→빨강 게이지), THEREMIN(200~2000 Hz, 10 Hz 단위) 으로 변환한다. 이 회로는 종합과제의 기본 골격이다.

노션 학습자료: [강의5주_학습자료](https://app.notion.com/p/3eab2e6bf80780afb272f967a82f8224)

## 연결

| 연결 | Arduino UNO R3 | ESP32 DevKitC V4 |
|---|---|---|
| LED R / G / B (330Ω) | D9 / D6 / D5 | GPIO25 / 26 / 27 |
| 수동 부저 (+) | D11 | GPIO23 |
| 가변저항 Wiper | A0 | GPIO34 (3.3V 전원) |
| 버튼 (모드 변경) | D2 ↔ GND | GPIO13 ↔ GND |

- LED 는 반드시 330Ω 직렬, 긴 다리(+)가 저항 쪽
- 버튼은 INPUT_PULLUP (누르면 LOW), ESP32 GPIO34~39 는 버튼 금지

## 파일

| 파일 | 내용 |
|---|---|
| `uno/sketch.ino` | Arduino UNO R3 코드 |
| `uno/diagram.json` | Wokwi 회로 (UNO) |
| `esp32/sketch.ino` | ESP32 DevKitC V4 코드 (코어 3.x) |
| `esp32/diagram.json` | Wokwi 회로 (ESP32) |

Wokwi: 새 프로젝트 → `sketch.ino`, `diagram.json` 탭 내용을 이 폴더의 파일로 교체 → Start

## 진행

1. 업로드 후 시리얼 모니터 115200.
2. 버튼으로 LAMP → COLOR → THEREMIN 을 바꾸며 노브를 돌린다.
3. 시리얼 한 줄의 각 값(ADC, V, Level, RGB, Freq)이 출력과 맞는지 확인한다.

## 예상 출력

```
MODE=COLOR ADC=612 V=2.99 Level=59% RGB=46/209/0 Freq=0
```

## 생각해 보기

- COLOR 모드에서 Level 25 % 와 75 % 의 RGB 값을 계산해 보라.
