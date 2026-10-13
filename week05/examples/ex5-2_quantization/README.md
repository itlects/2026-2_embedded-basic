# 실습 5-2 — 전압 환산 & 양자화 실험

10-bit(ESP32 12-bit)로 읽은 값을 오른쪽 시프트해 8 / 6 / 4 / 2-bit ADC 를 흉내 낸다. 버튼으로 비트 수를 바꾸며 V(매끈한 선), Vq(계단), Err_mV(양자화 오차)를 비교한다. ESP32 는 analogReadMilliVolts() 보정값 Vcal 도 출력한다.

노션 학습자료: [강의5주_학습자료](https://app.notion.com/p/3eab2e6bf80780afb272f967a82f8224)

## 연결

| 연결 | Arduino UNO R3 | ESP32 DevKitC V4 |
|---|---|---|
| 가변저항 양 끝 | 5V / GND | **3.3V** / GND |
| 가변저항 Wiper | A0 | GPIO34 |
| 버튼 (비트 수 변경) | D2 ↔ GND | GPIO13 ↔ GND |

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

1. 배선은 5-1 과 동일하다.
2. Serial Plotter 115200 에서 V, Vq, Err_mV 를 본다.
3. 버튼으로 10 → 8 → 6 → 4 → 2-bit 를 바꾸며 계단 개수와 MaxErr_mV 를 기록한다.

## 예상 출력

```
# 4-bit : 단계 수 16, 1 LSB = 312.50 mV, 최대 오차 ±156.25 mV
V:2.502,Vq:2.656,Err_mV:-153.8,Code:8,MaxErr_mV:153.8
```

## 생각해 보기

- 4-bit ADC(Vref 5V)로 2.6 V 를 측정하면 코드값과 복원 전압은?
