# 실습 5-4 — LED 레벨미터 + 경고음

ADC 를 7단계(레벨 0~6)로 재양자화해 LED 6개로 표시한다. 경계값 = k × 1024 / 7 (ESP32 4096). 경계 근처 떨림은 히스테리시스(HYST)로 막고, 레벨 6 이면 0.5초마다 1 kHz 경고음을 낸다.

노션 학습자료: [강의5주_학습자료](https://app.notion.com/p/3eab2e6bf80780afb272f967a82f8224)

## 연결

| 연결 | Arduino UNO R3 | ESP32 DevKitC V4 |
|---|---|---|
| LED1~6 (초록·초록·파랑·파랑·빨강·빨강) + 330Ω | D3, D4, D5, D6, D7, D8 | GPIO16, 17, 18, 19, 21, 22 |
| 수동 부저 (+) | D11 | GPIO23 |
| 가변저항 Wiper | A0 | GPIO34 (3.3V 전원) |
| 버튼 (경고음 ON/OFF) | D2 ↔ GND | GPIO13 ↔ GND |

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

1. 업로드 후 시리얼 모니터 115200 — 시작 시 경계값 6개가 출력된다.
2. 노브를 천천히 돌려 레벨 3 경계를 올라갈 때·내려갈 때 ADC 를 기록한다.
3. 코드의 HYST 를 0 으로 바꿔 경계 근처에서 LED 가 떨리는지 비교한다.
4. 레벨 6 에서 경고음, 버튼으로 끄고 켠다.

## 예상 출력

```
# 경계값: 146 292 438 585 731 877  HYST=10
Level=3 ADC=449 (올라감)  다음 상승 ≥595 / 하강 <428
```

## 생각해 보기

- 히스테리시스가 없으면 경계값 근처에서 어떤 현상이 생기는가?
