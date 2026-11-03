# 9주차 — ESP32-DevKitC V4 GPIO · ADC · PWM 와 복합 이벤트 제어

> 대상: 동서울대학교 임베디드프로세서기초, 3시간 / 8주차 중간고사 이후 첫 주차
> 보드: ESP32-DevKitC V4 (ESP32-WROOM-32) / Arduino IDE + **esp32 by Espressif 3.x**
> 강의 페이지(Notion): 강의9주차_학습자료 · 검토 내역: [REVIEW.md](REVIEW.md)

## 학습 목표
1. DevKitC V4의 구조(USB-UART, EN/BOOT, J2/J3)와 UNO와의 차이를 설명한다.
2. 3.3V 논리, 공통 GND, 입력 전용·부팅 스트랩·플래시 핀의 안전 수칙을 지킨다.
3. GPIO 출력/입력(INPUT_PULLUP·디바운스), ADC1(GPIO34), LEDC PWM을 실습한다.
4. `millis()`로 버튼·ADC·PWM·LED·부저를 블로킹 없이 동시에 처리한다.

## 3시간 운영
| 시간 | 내용 | 자료 | 산출물 |
|---|---|---|---|
| 0~20분 | 보드·부품 확인, IDE, 안전 점검 | [이론](theory/01_board_gpio_adc.md) · [워크북0](workbooks/00_preflight.md) · 예제 0 | board_check 로그 |
| 20~50분 | GPIO 출력 | [워크북1](workbooks/01_led.md) · 예제 1 | 점멸 영상 |
| 50~80분 | 버튼 입력·디바운스 | [워크북2](workbooks/02_button.md) · 예제 2, 2B | 10회 시험·비교표 |
| 80~110분 | ADC1 → PWM | [워크북3](workbooks/03_adc_pwm.md) · 예제 3, 3B | 측정표 |
| 110~165분 | 복합 제어 | [워크북4](workbooks/04_integrated.md) · 예제 4 | 동작 영상 |
| 165~180분 | 보고서 | [보고서 양식](workbooks/05_report_template.md) | 제출 |

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
│   └─ 04_integrated                버튼2·LED2·가변저항·부저 복합
└─ images/                          SVG 원본 + PNG (gen_theory.py, gen_examples.py로 재생성)
    ├─ t1~t9_*                      이론 그림(핀맵, GPIO, 디바운스, ADC, PWM, 흐름, 부품, 전원, UNO 비교)
    ├─ ex1~ex4_*_schematic/breadboard  예제별 회로도·브레드보드 배선도
    ├─ ex4_buzzer_npn_driver        부저 트랜지스터 구동(선택)
    └─ archive_v1/                  1차 생성 연결표 그림(보관)
```
그림 재생성: `cd images && python3 gen_theory.py . && python3 gen_examples.py . && for f in *.svg; do rsvg-convert -z 1.5 "$f" -o "${f%.svg}.png"; done`

## 검증 상태
- 모든 스케치: Arduino-ESP32 3.x 공개 헤더 시그니처(esp32-hal-ledc.h·adc.h) 기준 문법 검사 통과.
- 예제 4: 호스트 시뮬레이션(바운스 입력·ADC 변화)으로 1회 누름=1회 토글, 경보 1초 주기 확인.
- 실제 보드 업로드·배선 테스트는 수업 전 교수자가 수행해야 한다.

## 참고
- Espressif DevKitC V4: https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/user_guide.html
- Arduino-ESP32 설치: https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html
- 보조: https://machinejw.tistory.com/21
