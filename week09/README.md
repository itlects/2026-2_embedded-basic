# 9주차 — ESP32-DevKitC V4 시리얼 통신(UART · I2C · SPI · 1-Wire)과 GPIO · ADC · PWM

> 대상: 동서울대학교 임베디드프로세서기초, 3시간 / 8주차 중간고사 이후 첫 주차
> 보드: ESP32-DevKitC V4 (ESP32-WROOM-32) / Arduino IDE + **esp32 by Espressif 3.x**
> 강의 페이지(Notion): 강의9주차_학습자료 · 검토 내역: [REVIEW.md](REVIEW.md)

## 학습 목표 (Part B — 시리얼 통신, 9주차 핵심)
1. 시리얼 통신의 공통 원리를 이해하고, UART, I2C, SPI, 1-Wire 각 방식의 신호선 구성, 속도, 장단점을 비교 설명할 수 있다.
2. 실습: 시리얼 모니터로 센서값 송신, PC에서 명령 수신 시 릴레이 제어 (양방향 통신)

| 자료 | 내용 |
|---|---|
| [이론 Part B](theory/02_serial_comm.md) | 공통 원리, 4방식 비교, UART·I2C·SPI·1-Wire 파형 |
| [워크북 5](workbooks/06_uart_bidir_relay.md) | UART 양방향: 센서값 송신 + 명령으로 릴레이 (예제 5, 5B) |
| [워크북 6](workbooks/07_i2c_bh1750.md) | I2C BH1750 조도 (예제 6B 스캐너, 6) |
| [워크북 7](workbooks/08_spi_max6675.md) | SPI 보드 간 통신 UNO↔UNO (예제 7B 마스터·7C 슬레이브) + MAX6675 열전대 (예제 7) |
| [워크북 8](workbooks/09_onewire_ds18b20.md) | 1-Wire DS18B20 온도 (예제 8) |

예제 5·6·7·8은 같은 스케치가 ESP32와 Arduino UNO 모두에서 동작한다(핀 자동 선택). 5B만 ESP32 전용.

## 학습 목표 (Part A — 보드 기초: GPIO · ADC · PWM, 사전학습·보충)
1. DevKitC V4의 구조(USB-UART, EN/BOOT, J2/J3)와 UNO와의 차이를 설명한다.
2. 3.3V 논리, 공통 GND, 입력 전용·부팅 스트랩·플래시 핀의 안전 수칙을 지킨다.
3. GPIO 출력/입력(INPUT_PULLUP·디바운스), ADC1(GPIO34), LEDC PWM을 실습한다.
4. `millis()`로 버튼·ADC·PWM·LED·부저를 블로킹 없이 동시에 처리한다.

## 3시간 운영 (시리얼 통신 중심)
| 시간 | 내용 | 자료 | 산출물 |
|---|---|---|---|
| 0~15분 | 보드·IDE 확인, 안전 점검 | [워크북0](workbooks/00_preflight.md) · 예제 0 | board_check 로그 |
| 15~40분 | 시리얼 공통 원리·4방식 비교·UART | [이론 Part B](theory/02_serial_comm.md) · 예제 5B | 루프백 로그 |
| 40~90분 | **UART 양방향: 센서 송신 + 릴레이 명령** | [워크북5](workbooks/06_uart_bidir_relay.md) · 예제 5 | 명령·응답 로그, 영상 |
| 90~115분 | I2C BH1750 | [워크북6](workbooks/07_i2c_bh1750.md) · 예제 6B, 6 | 주소·조도 기록 |
| 115~140분 | SPI 보드 간 통신 + MAX6675 | [워크북7](workbooks/08_spi_max6675.md) · 예제 7B·7C, 7 | 양쪽 시리얼 로그, 비트 해석표 |
| 140~160분 | 1-Wire DS18B20 | [워크북8](workbooks/09_onewire_ds18b20.md) · 예제 8 | ROM ID·온도 |
| 160~180분 | 4방식 비교 정리, 보고서 | [보고서 양식](workbooks/05_report_template.md) | 제출 |

Part A(GPIO·ADC·PWM, 워크북 1~4)는 ESP32 보드 적응을 위한 사전학습·보충 과제로 사용한다(가변저항·버튼 회로는 Part B에서 재사용).

### Part A 운영(별도 시간 확보 시)
| 내용 | 자료 |
|---|---|
| GPIO 출력 | [워크북1](workbooks/01_led.md) · 예제 1 |
| 버튼 입력·디바운스 | [워크북2](workbooks/02_button.md) · 예제 2, 2B |
| ADC1 → PWM | [워크북3](workbooks/03_adc_pwm.md) · 예제 3, 3B |
| 복합 제어 | [워크북4](workbooks/04_integrated.md) · 예제 4 |

## 핀 할당
| 기능 | 핀 | 헤더 |
|---|---|---|
| LED1 (PWM) | GPIO25 | J2-9 |
| LED2 | GPIO26 | J2-10 |
| 수동 부저 | GPIO27 | J2-11 |
| 가변저항(ADC1_CH6) | GPIO34 | J2-5 |
| 버튼 RUN / MODE | GPIO18 / GPIO19 | J3-9 / J3-8 |
| 3V3 / GND | 3V3 / GND | J2-1 / J2-14, J3-1 |

## 폴더
```
week09/
├─ theory/01_board_gpio_adc.md      이론(보드·UNO 비교·핀맵·GPIO·ADC·PWM·부품·전원)
├─ workbooks/00~05                  학생 실습 워크북 + 보고서 양식
├─ examples/
│   ├─ 00_board_check               개발환경·보드 확인 (부품 없음)
│   ├─ 01_led                       GPIO25 LED millis 점멸
│   ├─ 02_button / 02b_bounce_count 버튼 토글 / 바운싱 관찰
│   ├─ 03_adc_pwm / 03b_ledc_pwm_steps  ADC→PWM / LEDC 듀티 단계
│   ├─ 04_integrated                버튼2·LED2·가변저항·부저 복합
│   ├─ 05_uart_bidir_relay          UART 양방향: 센서 송신 + 명령→릴레이 (ESP32/UNO)
│   ├─ 05b_uart2_loopback           ESP32 UART2 TX↔RX 루프백
│   ├─ 06b_i2c_scanner / 06_i2c_bh1750   I2C 주소 스캔 / 조도
│   ├─ 07b_spi_master / 07c_spi_slave_uno  SPI 보드 간 통신(마스터 UNO·ESP32 / 슬레이브 UNO 레지스터)
│   ├─ 07_spi_max6675               SPI 열전대
│   └─ 08_onewire_ds18b20           1-Wire 온도 (OneWire 라이브러리)
└─ images/                          SVG 원본 + PNG (gen_theory.py, gen_examples.py로 재생성)
    ├─ t1~t9_*                      이론 그림(핀맵, GPIO, 디바운스, ADC, PWM, 흐름, 부품, 전원, UNO 비교)
    ├─ ex1~ex4_*_schematic/breadboard  예제별 회로도·브레드보드 배선도
    ├─ ex4_buzzer_npn_driver        부저 트랜지스터 구동(선택)
    ├─ s1~s6_*                      시리얼 공통 원리·비교·UART·I2C·SPI·1-Wire (gen_serial.py)
    ├─ ex5~ex8_*                    시리얼 예제 연결도(ESP32·UNO 핀표 포함)
    └─ archive_v1/                  1차 생성 연결표 그림(보관)
```
그림 재생성: `cd images && python3 gen_theory.py . && python3 gen_examples.py . && python3 gen_serial.py . && for f in *.svg; do rsvg-convert -z 1.5 "$f" -o "${f%.svg}.png"; done`

## 검증 상태
- 시리얼 예제 5~8: ESP32·UNO 두 조건에서 Arduino 코어 API 시그니처 기준 문법 검사 통과, 예제 5는 호스트 시뮬레이션으로 명령 처리·AUTO 히스테리시스 확인.
- 모든 스케치: Arduino-ESP32 3.x 공개 헤더 시그니처(esp32-hal-ledc.h·adc.h) 기준 문법 검사 통과.
- 예제 4: 호스트 시뮬레이션(바운스 입력·ADC 변화)으로 1회 누름=1회 토글, 경보 1초 주기 확인.
- 실제 보드 업로드·배선 테스트는 수업 전 교수자가 수행해야 한다.

## 참고
- Espressif DevKitC V4: https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/user_guide.html
- Arduino-ESP32 설치: https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html
- 보조: https://machinejw.tistory.com/21
