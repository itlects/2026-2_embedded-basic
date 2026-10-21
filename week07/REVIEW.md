# 7주차 자료 검토 결과 (2026-10-21)

## 검증 방법
| 대상 | 방법 | 결과 |
|---|---|---|
| UNO 스케치 6개 | avr-gcc 7.3 + ArduinoCore-avr 빌드 (`-Wall -Wextra`) | 모두 경고 없이 빌드 |
| UNO 실행 동작 | simavr(ATmega328P 16MHz) 시뮬레이션, 바운스 포함 버튼 입력 주입 | 시계 1.000초 주기, 버튼 기능·디바운스 정상 |
| ESP32 스케치 6개 | Arduino-ESP32 코어 3.x / 2.x API 시그니처 기준 문법·형식 검사 | 모두 통과 |
| 그림 15개 | SVG를 렌더링해 핀 번호·결선을 코드와 대조 | 오류 5건 발견 → 수정 |

실물 보드 테스트는 수업 전에 별도로 해야 한다.

## 코드 검토
| 파일 | 판정 | 내용 |
|---|---|---|
| `examples/uno_01_external_interrupt` | 정상 | — |
| `examples/esp32_01_external_interrupt` | 수정 | ESP32는 듀얼코어라 `noInterrupts()`만으로는 공유 플래그 보호가 불완전 → `portMUX` 임계구역으로 변경 (실습 3과 같은 방식으로 통일) |
| `examples/*_02_clock` | 정상 | millis() 폴링 시계 |
| `examples/*_02b_timer_interrupt_clock` | **신규** | 강의 주제가 '인터럽트 처리'인데 시계가 인터럽트를 쓰지 않았음 → UNO Timer1 CTC / ESP32 HW 타이머 인터럽트 시계 추가 |
| `labs/uno_03_buttons_pcint` | 개선 | 동작은 정상. 시계를 Timer1 인터럽트로 변경해 '두 인터럽트 소스 + 이벤트 기반 loop' 구조로 정리, 함수 분리·주석 추가 |
| `labs/esp32_03_buttons_gpio` | 개선 | 위와 동일 (HW 타이머 인터럽트 시계, 코어 3.x/2.x 겸용) |
| `labs/*_04_ws2812_interrupt` | 정상 | ISR에서 show() 호출 없음, 디바운스·일시정지 정상 |
| `labs/*_06_dht11_alarm` (워크북 5, 추가분) | 정상 | UNO는 DHT 라이브러리 포함 빌드 성공(9.1KB), ESP32 문법 통과. 습도<임계값 경보, 임계값 20~90% 제한, 센서 오류 시 LED 깜빡임 로직 확인 |
| `labs/*_06_dht11_alarm` (워크북 5) | 정상 | UNO 빌드(DHT sensor library 포함)·ESP32 문법 검사 통과. 저습 경보(humidity < threshold) 로직과 워크북 설명 일치 |
| `labs/*_05_advanced_sequencer` | 수정 | C(속도) 버튼으로 주기를 줄이면 밀린 스텝이 연속 실행되는 현상(시뮬레이션에서 4ms 간격 2스텝 확인) → 속도 변경 시 `lastStep = now` |

참고: 5번 실습의 B(모드) 직후 position 값이 그대로 유지되는 것은 의도된 동작이다.

## 그림 검토
| 그림 | 문제 | 조치 |
|---|---|---|
| `theory_2_2_vector_isr_flow_ko` | 연결선에 방향(화살표)이 없고 ④~⑥ 순서가 오른쪽→왼쪽으로 읽혀 ⑥에서 ④로 가는 것처럼 보임 | 화살표 추가, 플래그·I비트·RETI 등 하드웨어 절차를 정확히 표기 |
| `esp32_ws2812_8bar_12ring_ko` | +5V 빨간 선이 74AHCT125 박스를 관통, 우측에 끝이 열린 빨간 선, 모듈 전원 연결 불명확 | 새로 그림: 레벨 변환 IC VCC/GND/OE 결선, +5V·GND 버스, 콘덴서 극성, 교차 표시 |
| `uno_ws2812_8bar_12ring_ko` | 동일한 전원선 오류(열린 끝 선) | 새로 그림 |
| `workbook04_esp32_schematic_ko` | LED 라벨이 "LEGPIO18 …"로 깨짐(문자열 치환 오류), 부저 안내 박스에 "D6"(UNO 핀) 표기 | 라벨·핀 수정, LED 극성 기호 추가 |
| `workbook05_uno/esp32_dht11_alarm_ko` | 버튼 반대 접점·LED 음극이 '→ GND' 글자로만 끝나고 선이 없음, DHT11 VCC/GND·부저·보드 GND 미표시 | 새로 그림: 모든 GND를 공통선으로, DHT11 3선, 4핀 단품용 풀업(점선), 부저 극성 |
| `workbook05_*_dht11_alarm_ko` | 버튼 반대쪽·LED 음극이 '→ GND' 글자로만 끝나 실제 GND 연결선이 없음, DHT11 VCC·GND·부저 결선 미표시, 보드 GND 핀 없음 | 새로 그림: DHT11 3선, 4핀 단품용 풀업(점선), 공통 GND, 부저 |
| `workbook04_uno_schematic_ko` | LED·부저 신호선이 버튼 GND선과 교차해 연결된 것처럼 보임 | 교차부에 넘어가기(⌒) 표시 |

원본은 `images/archive_before_fix/`에 보관(삭제하지 않음). 노션에 삽입된 그림은 같은 파일명을 쓰므로 자동으로 수정본이 표시된다.

### 새로 추가한 그림
- `example02_usb_only_connection_ko.svg` — 예제 2·2B 연결(USB만)
- `theory_2_3_avr_vector_table_ko.svg` — ATmega328P 벡터 테이블(이번 주 사용 벡터 강조)
- `theory_2_5_timer1_ctc_clock_ko.svg` — Timer1 CTC 1초 인터럽트 원리
- `theory_2_6_event_driven_structure_ko.svg` — 실습 3의 ISR ↔ loop() 이벤트 구조
- `uno_3buttons_breadboard_ko.svg`, `esp32_3buttons_breadboard_ko.svg` — 학생이 그대로 따라 꽂는 브레드보드 배선도(대각선 다리 사용)

## 문서
- 워크북 1~5 끝에 **작업 확인 결과 리포트** 표 추가(기본 정보·결선 확인·기능 확인·측정·분석·문제 해결·제출물·교수자 확인)
- README: 학습목표·순서·핀 요약 정리, Timer1 사용 시 D9·D10 PWM 불가 안내
- 워크북 1: 예제 2B(타이머 인터럽트 시계)와 비교 실험 추가
- 워크북 2: 브레드보드 배선도·구조도 삽입, 기능 확인표, 시뮬레이션 출력 예, 도전 과제
- 워크북 3: 연결도 삽입, 74AHCT125 핀 번호
- 이론 보충: 벡터 테이블·CPU 처리 절차, 타이머 레지스터 해설, millis vs 타이머 비교, 이벤트 기반 구조, 문제 2개 추가

## 워크북 결과 리포트 표
워크북 1~5 끝에 '작업 확인 결과 리포트'를 추가했다: 기본 정보 → 결선 확인 → 기능 확인(기대/실제/판정) → 측정·분석 → 문제 해결 기록 → 제출물 → 교수자 확인.
