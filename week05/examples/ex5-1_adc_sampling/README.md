# 실습 5-1 — ADC 기본값 읽기 & 샘플링 주기

가변저항 전압을 일정한 주기로 읽어 Serial Plotter 로 보고, 샘플링 주기 T 를 바꾸며 빠른 변화를 놓치는 현상을 확인한다. 시작할 때 `analogRead()` 1회 시간을 측정한다.

노션 학습자료: [강의5주_학습자료](https://app.notion.com/p/3eab2e6bf80780afb272f967a82f8224)

## 연결

| 연결 | Arduino UNO R3 | ESP32 DevKitC V4 |
|---|---|---|
| 가변저항 양 끝 | 5V / GND | **3.3V** / GND |
| 가변저항 Wiper | A0 | GPIO34 |
| 버튼 (샘플링 주기 변경) | D2 ↔ GND | GPIO13 ↔ GND |

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

1. 업로드 후 Serial Plotter 115200 을 연다.
2. 버튼으로 T = 10 / 100 / 500 / 1000 ms 를 바꾼다.
3. 가변저항을 1초 안에 왕복시키며 그래프 모양을 비교한다.
4. 마지막에 Wiper 선을 뽑아 플로팅 잡음을 확인한다.

## 예상 출력

```
# analogRead 1회 = 107.4 us  → 최대 약 9311 samples/s   (UNO, 시뮬레이터 값)
# T = 100 ms, fs = 10.0 Hz
Min:0,Max:1023,ADC:512
```

## 생각해 보기

- T = 1000 ms 일 때 0.5초 만에 왕복시키면 그래프는 어떻게 보이는가? 왜 그런가?
