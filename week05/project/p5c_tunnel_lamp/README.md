# 수업과제 3 — 터널 멜로디 거리 램프 워크북

터널 양쪽 벽에 LED 를 3구간으로 배치하고 초음파 센서로 사람의 위치를 측정한다. 사람이 들어오면 멜로디가 흐르며 사람이 있는 구간과 앞 구간이 켜지고, 지나간 구간은 꺼진다. 터널을 통과하면 멜로디가 멈추고 모든 LED 가 꺼진다.

단계별 설명·확인표: [노션 — 수업과제3-터널멜로디거리램프-워크북](https://app.notion.com/p/3f1b2e6bf8078155a5aac88a0282162d)

모든 스케치는 **UNO·ESP32 공용**이다.

## 단계 폴더

| 폴더 | 단계 | 확인 내용 |
|---|---|---|
| `t1_ultrasonic` | T1 | HC-SR04 거리 측정 (60 ms 샘플링 주기) |
| `t2_zone_led` | T2 | 거리 → 구간 Z1~Z3 양자화, 구간 LED, 가변저항 밝기 |
| `t3_melody` | T3 | 논블로킹 멜로디 재생/정지 (반짝반짝 작은 별) |
| `final` | 완성 | 상태 머신 IDLE → IN → PASSED, 앞 구간 미리 켜기, LCD |

각 폴더: `sketch.ino`, `diagram.json`(UNO), `diagram_esp32.json`(ESP32), 필요한 경우 `libraries.txt`

## 구간

| 측정 거리 | 판정 |
|---|---|
| 100 cm 초과 또는 반사 없음 | 터널 밖 |
| 70 ~ 100 cm | Z1 입구 (초록) |
| 40 ~ 70 cm | Z2 중간 (파랑) |
| 10 ~ 40 cm | Z3 출구 (빨강) |
| 10 cm 이하 | 출구 통과선 |

## 핀 배치

| 기능 | UNO | ESP32 |
|---|---|---|
| 초음파 TRIG / ECHO | D7 / D8 | GPIO5 / GPIO19 |
| Z1 / Z2 / Z3 LED (각 2개, LED 마다 330Ω) | D5 / D6 / D9 | GPIO25 / 26 / 27 |
| 가변저항 (밝기) | A0 | GPIO34 (3.3V 전원) |
| 수동 부저 | D11 | GPIO23 |
| BTN_MUTE | D2 | GPIO13 |
| I2C LCD SDA / SCL | A4 / A5 | GPIO21 / 22 |
| 초음파·LCD VCC | 5V | 5V (VIN) |

- **ESP32 주의:** HC-SR04 의 ECHO 는 5V 신호 → 1kΩ·2kΩ 분압 또는 3.3V 겸용 HC-SR04P 사용 (Wokwi 는 그대로 동작)
- UNO 의 D3·D11 은 tone() 때문에 PWM 불가 → 구간 LED 는 D5·D6·D9
- 라이브러리: LiquidCrystal I2C
