# 9주차 자료 검토 및 보완 내역 (2026-11-03)

1차 자료(ChatGPT 생성: Notion 본문 · Drive 문서 6개 · GitHub week09)를 검토하고 아래를 보완했다.

## A. 오류 수정
| # | 위치 | 문제 | 조치 |
|---|---|---|---|
| 1 | Notion 3-4 · theory ADC 예제 | `analogSetPinAttenuation(POT, ADC_ATTEN_DB_11)` — ESP-IDF 상수(adc_atten_t)를 Arduino 함수(adc_attenuation_t)에 넣어 **C++ 컴파일 오류** | `ADC_11db`로 수정 |
| 2 | Notion 3-A | 'ESP32 보드 실물 확인' 절에 LED 연결 그림(01_led.svg)이 보드 그림으로 삽입됨 · 3-1과 내용 중복 | 핀맵 그림(t1)으로 교체, 중복 정리 |
| 3 | 예제 4 부저 | `analogWrite`+`analogWriteFrequency`로 톤 생성 — 동작은 하나 의도가 불명확하고 LED PWM과 혼동 | `ledcAttach`+`ledcWriteTone`(전용 채널)로 변경 |
| 4 | 예제 3 | 전압 확인 수단 없음 | `analogReadMilliVolts()` 출력 추가 |
| 5 | 그림 4개 | 영어 '연결표' 그림 — 회로도·브레드보드 배선·부품 모양 없음, 학생이 따라 꽂을 수 없음 | 한글 회로도 4 + 브레드보드 배선도 4(구멍 좌표 포함)로 대체 |

## B. 보충한 내용
| 영역 | 추가 |
|---|---|
| 이론 그림 | 핀맵(J2/J3 38핀, 분류 색·사용 핀 표시), GPIO 출력/풀업 입력, 디바운스 파형, ADC(분배·양자화·11dB 실제 응답), PWM(듀티·평균전압·LEDC 주파수×해상도), ADC→PWM 흐름, 부품 8종, 전원 경로·금지/확인 목록, UNO 비교표 |
| 보드 특성 | USB-UART 드라이버(CP210x/CH340), BOOT 수동 업로드, 3.3V LDO, 보드 폭(브레드보드 점유) 문제, DAC 핀 언급, 30핀 DevKit과 구분 |
| GPIO | LED 전류 계산과 색상별 Vf(파랑·흰 LED는 3.3V에서 어두움), 핀당 10mA 설계, 내부 풀업 ≈45kΩ, 플로팅 |
| ADC | 측정 범위 150~3100mV(공식), 양 끝 포화, ADC2–Wi-Fi 충돌, analogReadMilliVolts |
| PWM | LEDC 제약(주파수 × 2^bits ≤ 80MHz)과 해상도별 최대 주파수, analogWrite vs ledcAttach, 톤 출력 |
| 부저 | 수동/능동 구별, 직결 조건, **NPN 구동 회로(1kΩ·플라이백 다이오드)** |
| 예제 | 00 board_check, 02b 바운싱 카운터(시리얼로 디바운스 시간 변경), 03b LEDC 듀티 단계 |
| 워크북 | 각 워크북에 회로도 → 부품 → 구멍 좌표 배선표 → 코드 → 예상 시리얼 출력 → 검증표 → 문제 해결표 → 과제 구성. 워크북 0(IDE·업로드 문제 해결·안전 체크리스트) 신설 |
| 보고서 | 안전 점검 근거, 예제 0~4·2B·3B, 측정 데이터, 분석 질문 7개 |

## C. 검증
- g++ 문법 검사(3.x 헤더 시그니처 스텁, -Wall -Wformat=2): 7개 스케치 통과, 수정 전 ADC 스니펫은 오류 재현.
- 예제 4 호스트 시뮬레이션: 바운스 6ms 입력 → 토글 1회, 경보 1초 주기·150ms, 정지 시 LED 꺼짐 확인.
- 핀 기능은 Espressif DevKitC V4 User Guide J2/J3 표와 대조.
- **미검증**: 실제 보드 업로드, 부저 음량, 실측 ADC 값 → 수업 전 교수자 확인 필요.
