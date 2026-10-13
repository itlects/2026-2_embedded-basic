# 실습 5-3 — ADC → PWM 가변램프

가변저항 ADC 값을 PWM 으로 바꿔 LED 밝기를 조절한다. 버튼으로 선형 map() 과 감마 보정(PWM = 255 × x^2.2)을 비교하고, DEAD_ZONE 으로 0 근처 깜빡임을 막는다.

노션 학습자료: [강의5주_학습자료](https://app.notion.com/p/3eab2e6bf80780afb272f967a82f8224)

## 연결

| 연결 | Arduino UNO R3 | ESP32 DevKitC V4 |
|---|---|---|
| 가변저항 Wiper | A0 | GPIO34 (3.3V 전원) |
| LED (330Ω 직렬) | D9 | GPIO25 |
| 버튼 (선형/감마) | D2 ↔ GND | GPIO13 ↔ GND |

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
2. 가변저항을 돌리며 ADC / V / PWM / Duty 와 밝기를 관찰한다.
3. 버튼으로 GAMMA 모드로 바꾸고 같은 노브 위치에서 밝기 차이를 비교한다.

## 예상 출력

```
LINEAR ADC=512 V=2.50 PWM=127 Duty=49.8%
GAMMA ADC=512 V=2.50 PWM=56 Duty=22.0%
```

## 생각해 보기

- 사람 눈이 밝기를 비선형으로 느낀다는 것을 어느 구간에서 가장 크게 체감했는가?
