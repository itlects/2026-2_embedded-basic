# 10주차 DHT11 센서/부저 워크북

**보드:** ESP32 DevKitC V4 / **제출:** 회로 사진, 시리얼 캡처, 부저 시연, 문항 답변

## 부품과 전원
DHT11 3핀 모듈(VCC/DATA/GND) 또는 4핀 단독 센서, 저전류 수동 피에조 부저, 점퍼선, 브레드보드, 선택 LED와 330Ω. 배선 전 USB 분리! GPIO는 3.3V 로직(5V 직접 연결 금지). 단독 DHT11에 DATA→3V3 4.7k~10kΩ 풀업 연결; 3핀 모듈은 내장 여부 확인. 센서 핀 라벨을 우선 확인.

## 1단계: 온습도 측정
ESP32 3V3→DHT11 VCC, GPIO4→DATA, GND→GND. [연결도](../diagrams/01_DHT11_Serial.svg). [완성 코드](../examples/01_DHT11_Serial.ino) 업로드 후 시리얼 모니터 115200. 2초마다 측정값을 읽고 온습도 변화 기록.

## 2단계: 부저 임계값 경고
기존 회로에 GPIO25→부저 +, GND→부저 − 연결. [연결도](../diagrams/02_DHT11_Buzzer.svg) / [완성 코드](../examples/02_DHT11_Buzzer_Alert.ino). TEMP_ON=30, TEMP_OFF=28, HUM_ON=75, HUM_OFF=70 (실내 환경에 맞춰 변경). 경보 ON/해제 확인. `millis()` 기반 부저 동작 관찰.

## 3단계: LED+CSV 확장
GPIO26→330Ω→LED 애노드, LED 캐소드→GND. [연결도](../diagrams/03_DHT11_LED_Buzzer.svg) / [완성 코드](../examples/03_DHT11_Dashboard_Extension.ino). 시리얼 CSV와 LED 상태 확인.

## 결과보고서
| 항목 | 관찰/결과 |
|---|---|
| 센서 형태(3핀/4핀) | |
| 실내 온도/습도 | |
| 변화 후 온도/습도 | |
| 변경한 ON/OFF 임계값 | |
| 실제 부저/LED 동작 | |
| 오류 발생/대응 | |

## 확인 질문
1. DHT11의 타이밍 기반 단일 데이터선 프로토콜과 I²C(SDA/SCL)의 차이는?
2. 비트 0과 비트 1의 High 펄스 길이 차이는?
3. 왜 센서는 약 2초 간격으로 읽는가?
4. 임계값 ON/OFF를 다르게 설정하는 목적은?
5. 읽기 실패 시 경보 출력이 꺼져야 하는 이유는?

**실물 보드 실행 검증은 아직 수행하지 않음.**
