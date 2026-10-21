# 7주차 — 인터럽트 처리

## 학습목표
인터럽트 벡터와 처리 절차를 이해하고 이벤트 기반 프로그램을 구현할 수 있다.
- 실습: 시계 만들기 (millis() 폴링 시계 → 타이머 인터럽트 시계)
- 카운터 응용: 버튼 3개를 인터럽트 입력으로 설정해 카운터 증가/초기화/일시정지 구현

## 학습 순서
| 단계 | 자료 | 핵심 |
|---|---|---|
| 이론 | [theory/arduino_interrupt_function_guide.md](theory/arduino_interrupt_function_guide.md), `images/theory_*` | 폴링 vs 인터럽트, 벡터 테이블, CPU 처리 절차, 디바운스, 타이머 인터럽트, 이벤트 기반 구조 |
| 예제 1 | `examples/*_01_external_interrupt` | 버튼 1개 하강 에지 → ISR 플래그 → loop 출력 |
| 예제 2 | `examples/*_02_clock` | `millis()` 폴링 1초 시계 (비차단) |
| 예제 2B | `examples/*_02b_timer_interrupt_clock` | **타이머 인터럽트** 1초 시계 (UNO Timer1 CTC / ESP32 HW 타이머) |
| 실습 3 | `labs/*_03_*` + [워크북 2](workbooks/workbook_02_three_button_counter.md) | 타이머 인터럽트 시계 + 3버튼 인터럽트 카운터(증가/초기화/일시정지) |
| 확장 1 | `labs/*_04_*` + 워크북 3 | WS2812 8LED+12LED 인터럽트 표시 |
| 확장 2 | `labs/*_05_*` + 워크북 4 | 4버튼·LED 5개·부저 시퀀서 |
| 확장 3 | `labs/*_06_*` + 워크북 5 | DHT11 습도 감시·인터럽트 경보 |

### 워크북 순서
1. [워크북 1 — 인터럽트와 시계 만들기](workbooks/workbook_01_interrupt_clock.md)
2. [워크북 2 — 시계와 3버튼 인터럽트 카운터](workbooks/workbook_02_three_button_counter.md)
3. [워크북 3 — WS2812 8LED·12LED 확장](workbooks/workbook_03_ws2812_8bar_12ring.md)
4. [워크북 4 — 4버튼·LED 5개·부저 시퀀서](workbooks/workbook_04_advanced_interrupt_sequencer.md)
5. [워크북 5 — DHT11 습도 감시·인터럽트 경보](workbooks/workbook_05_dht11_interrupt_alarm.md)

모든 워크북 끝에 학생이 채우는 **작업 확인 결과 리포트** 표(기본 정보·결선 확인·기능 확인·분석·문제 해결·제출물·교수자 확인)가 있다.

## 폴더
- `examples/`: 기본 예제 (외부 인터럽트, millis 시계, 타이머 인터럽트 시계)
- `labs/`: 통합 실습
- `workbooks/`: 학생용 단계별 실습 가이드
- `theory/`: 코드로 이해하는 함수·레지스터 해설
- `images/`: 이론 그림, 회로도, 브레드보드 배선도(SVG)
  - `images/archive_before_fix/`: 검토 전 원본 그림(보관용)
- [REVIEW.md](REVIEW.md): 자료·코드 검토 결과와 수정 내역

## 핀 요약
| 기능 | Arduino UNO | ESP32 DevKit |
|---|---|---|
| 버튼 A 증가 | D2 (PCINT18) | GPIO18 |
| 버튼 B 초기화 | D3 (PCINT19) | GPIO19 |
| 버튼 C 일시정지/재개 | D4 (PCINT20) | GPIO21 |
| 1초 시계 | Timer1 CTC (TIMER1_COMPA_vect) | 하드웨어 타이머 알람 |

## 주의
Arduino UNO(ATmega328P)의 외부 인터럽트 INT0/INT1은 D2/D3 두 개뿐이다. 3버튼 인터럽트 실습은 D2·D3·D4의 핀 변경 인터럽트(PCINT2 그룹)를 사용한다. ESP32 DevKit은 GPIO18·19·21의 GPIO 인터럽트를 사용한다. 모든 버튼은 내부 풀업(`INPUT_PULLUP`)을 쓰고 버튼의 반대편(대각선 다리)을 GND에 연결한다. UNO에는 5V, ESP32에는 3.3V 논리 레벨만 인가한다.

UNO Timer1을 시계로 쓰면 D9·D10의 `analogWrite()`(PWM)는 사용할 수 없다(6주차 내용과 연결).

**검증 구분:** UNO 스케치는 avr-gcc 빌드 + simavr 시뮬레이션으로 동작을 확인했고, ESP32 스케치는 API 시그니처 기준 문법 검사를 했다. 실물 보드 테스트는 수업 전 별도로 수행한다.
