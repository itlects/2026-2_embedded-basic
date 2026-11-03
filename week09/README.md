# 9주차 — ESP32-DevKitC V4 GPIO·ADC·논블로킹 복합 실습

> 대상: 동서울대학교 IT융합학과 2학년, 3시간 / 8주차 중간고사 이후 첫 주차
> **보드**: ESP32-DevKitC V4, ESP32-WROOM-32 계열 / Arduino IDE (Espressif Arduino-ESP32 3.x)

## 학습 목표
1. ESP32 보드의 GPIO/전원/ADC1 핀과 부팅 관련 주의 핀을 설명한다.
2. LED 저항 직렬 연결과 INPUT_PULLUP 버튼의 회로를 스스로 설계한다.
3. ADC1(GPIO34)로 가변저항의 0~3.3V 입력을 읽어 PWM 출력에 반영한다.
4. millis()를 써서 버튼·센서·부저·LED를 블로킹 없이 함께 제어한다.

## 3시간 운영
| 시간 | 내용 | 산출물 |
|---|---|---|
| 0~20분 | 보드 핀맵/3.3V/Arduino IDE 설정 | 연결 검토표 |
| 20~50분 | 예제1 LED 출력 | LED 점멸 영상 |
| 50~80분 | 예제2 버튼 입력/디바운스 | 버튼 로그 |
| 80~110분 | 예제3 ADC1-가변저항-LED PWM | ADC 로그 |
| 110~165분 | 워크북4 복합 제어 | 동작 영상/시리얼 캡처 |
| 165~180분 | 분석/결과보고/정리 | 결과표 |

## 자료
- [이론과 핀 배치](theory/01_board_gpio_adc.md)
- [기본 결선 체크리스트](workbooks/00_preflight.md)
- [워크북1 LED](workbooks/01_led.md) / [회로 그림](images/01_led.svg) / [코드](examples/01_led/01_led.ino)
- [워크북2 버튼](workbooks/02_button.md) / [회로 그림](images/02_button.svg) / [코드](examples/02_button/02_button.ino)
- [워크북3 ADC/PWM](workbooks/03_adc_pwm.md) / [회로 그림](images/03_adc_pwm.svg) / [코드](examples/03_adc_pwm/03_adc_pwm.ino)
- [워크북4 복합제어](workbooks/04_integrated.md) / [회로 그림](images/04_integrated.svg) / [코드](examples/04_integrated/04_integrated.ino)
- [학생 실습 보고서 양식](workbooks/05_report_template.md)

## 개발환경
Arduino IDE → 파일 > 환경설정 > 추가 보드 매니저 URLs:
`https://espressif.github.io/arduino-esp32/package_esp32_index.json`
→ 보드 매니저에서 **esp32 by Espressif Systems** 설치 → **ESP32 Dev Module** 또는 실제 보드의 ESP32 DevKitC 항목 선택 → 포트 확인 → 모니터 **115200 baud**.

## 정확성·안전
- ESP32 GPIO는 **3.3V 논리**이다. 5V를 GPIO/ADC 입력에 넣지 않는다.
- LED는 220~330Ω 저항을 직렬로 넣고, 공통 GND를 완성한다.
- GPIO34~39는 **입력 전용**이며 내부 pull-up/down이 없다. 워크북에서는 GPIO34에 가변저항의 중심핀만 연결한다.
- GPIO6~11은 보드 모듈의 SPI 플래시 관련 핀이라 사용하지 않는다. GPIO0/2/5/12/15는 부팅 스트래핑 역할이 있으므로 예제에서 피한다. GPIO1/3은 업로드/시리얼에 사용한다.
- GPIO25·26 등의 PWM은 ESP32 Arduino 코어의 `analogWrite` API를 이용한다.
- WS2812는 보통 5V 전원을 사용하며 신호 레벨 매칭/전원용량 검토가 필요하므로 9주차 기본 회로에서 제외했다.
- 실물 및 Arduino-ESP32 컴파일은 아직 수행하지 않았다. 배선과 API는 검토했으며 수업 시작 전 실제 보드 테스트가 필요하다.

## 참고
- Espressif DevKitC V4: https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/user_guide.html
- Arduino ESP32 설치: https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html
- 보조 블로그: https://machinejw.tistory.com/21
