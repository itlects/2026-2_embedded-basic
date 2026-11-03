# 9주차 이론학습 — ESP32-DevKitC V4 GPIO·ADC·PWM

## 1. ESP32-DevKitC V4 특징
- ESP32 모듈을 장착한 개발보드. Wi-Fi·Bluetooth 및 GPIO·ADC·PWM(LEDC)·UART·I2C·SPI 지원
- Micro-USB 통신/전원, USB-UART 브리지, EN 리셋, BOOT 다운로드 버튼, 양쪽 GPIO 헤더
- 실장 모듈(WROOM/WROVER 등)과 헤더 실크인쇄를 반드시 확인
- 공식 핀배치: https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/user_guide.html

## 2. Arduino UNO R3와의 차이
| 항목 | UNO R3 | ESP32-DevKitC V4(전통적 ESP32 모듈) |
|---|---|---|
| MCU | ATmega328P | ESP32 |
| 구조 | 8비트 16MHz | 32비트 일반적 듀얼코어 최대 240MHz |
| 로직 레벨 | 5V | 3.3V |
| ADC | 10비트 0~1023 | 12비트 0~4095 (ADC 감쇠/보정 주의) |
| PWM | 6개 PWM 출력 핀 | LEDC 기능을 여러 GPIO에 할당 |
| 무선 | 별도 모듈 필요 | Wi-Fi/Bluetooth 내장 |
| 버튼 | INPUT_PULLUP | INPUT_PULLUP; GPIO34~39에서는 내부 풀업 불가 |
ESP32에 UNO의 5V 출력 신호를 직접 연결하면 안 된다.

## 3. GPIO 기본 원리
GPIO(General Purpose Input/Output)는 프로그래밍 가능한 디지털 입력·출력 핀이다. 입력은 버튼의 HIGH/LOW를 판독하고 출력은 LED를 켠다.
```cpp
const int LED=25, BUTTON=18;
void setup(){ pinMode(LED,OUTPUT); pinMode(BUTTON,INPUT_PULLUP); }
void loop(){ digitalWrite(LED,digitalRead(BUTTON)==LOW ? HIGH:LOW); }
```
- LED: GPIO25 → 220~330Ω 직렬 저항 → LED(+), LED(-) → GND
- 버튼: GPIO18 → 버튼 반대 접점 → GND. 내부 풀업이 3.3V에 연결해 주므로 놓았을 때 HIGH, 눌렀을 때 LOW
- 바운스: 접점 떨림이 여러 입력으로 보이므로 25ms 정도 상태 안정 확인

## 4. ADC 원리
ADC는 연속 아날로그 전압을 숫자로 바꾼다. 12비트 코드는 0~4095 (2^12=4096 단계).
- 가변저항 10kΩ 양끝을 3V3/GND, 가운데를 GPIO34(ADC1_CH6)로 연결.
- ADC1은 Wi-Fi 사용 시 ADC2 경합 문제를 피하기 좋다.
- ADC 값이 전압과 완전히 비례하지는 않는다. ESP32 11dB 감쇠의 공식 권장 측정 범위는 약 150~3100mV; 단순히 4095=3.3V라고 단정하지 않는다.
```cpp
const int POT=34;
void setup(){Serial.begin(115200);analogReadResolution(12);analogSetPinAttenuation(POT,ADC_ATTEN_DB_11);}
void loop(){Serial.println(analogRead(POT));delay(100);}
```

## 5. PWM 원리
PWM은 HIGH와 LOW를 빠르게 반복하면서 HIGH 비율(듀티비)을 조절하는 출력 방식.
- 듀티비 = HIGH 지속시간 / 한 주기 ×100%
- 8비트 0=0%, 128≈50%, 255≈100%. LED 밝기 및 수동부저 주파수 제어.
- ESP32의 LEDC 주변장치를 사용. Arduino-ESP32 3.x API의 예:
```cpp
const int LED=25;
void setup(){ledcAttach(LED,5000,8);}
void loop(){ledcWrite(LED,64);delay(1000);ledcWrite(LED,192);delay(1000);}
```

## 6. 핀맵 구조
공식 DevKitC V4는 좌우 헤더 **J2와 J3**로 구분한다. IO34를 코드에서 `34`로 쓰며, 물리 헤더 위치와 GPIO 번호를 혼동하지 않는다.

| 핀 | 헤더 | 연결 부품 | 용도 |
|---|---|---|---|
| GPIO18 | J3 | 버튼1→GND | INPUT_PULLUP RUN |
| GPIO19 | J3 | 버튼2→GND | INPUT_PULLUP MODE |
| GPIO25 | J2 | 220~330Ω→LED1(+) | PWM 출력 |
| GPIO26 | J2 | 220~330Ω→LED2(+) | LED 출력 |
| GPIO34 | J2 | 가변저항 가운데 | ADC1_CH6 입력 전용 |
| GPIO27 | J2 | 적정 드라이버/저전류 부저 | PWM/톤 |
| 3V3 | J2 | 가변저항 양끝 중 하나 | 전원 |
| GND | J2/J3 | 가변저항, LED(-), 버튼 반대편, 부저(-) | 공통 접지 |

주의 핀: GPIO6~11은 플래시, GPIO0/2/5/12/15는 부팅 스트래핑, GPIO1/3은 UART0. GPIO34~39는 출력 불가/내부 풀업 없음. GPIO16/17은 WROVER 모듈에서 내부 사용일 수 있음.

## 7. 필수 부품과 동작원리
| 부품 | 수량(복합 워크북) | 기능 |
|---|---:|---|
| ESP32-DevKitC V4 + Micro-USB 데이터 케이블 | 각 1 | MCU, 전원, 업로드 |
| 브레드보드·점퍼선 | 1·여러개 | 납땜 없는 공통 배선 |
| LED | 2 | 전류 통과 시 발광(극성 있음) |
| 220~330Ω 저항 | 2 | 각 LED 전류 제한 |
| 택트 버튼 | 2 | 누를 때 두 접점을 연결 |
| 가변저항 약 10kΩ | 1 | 분압으로 아날로그 입력 생성 |
| 3.3V 호환 저전류 수동부저 | 1 | PWM에 따른 소리 출력(전류 확인) |
선택 확장: DHT11 및 NeoPixel은 이번 주차 필수 회로에서 제외. 사용 시 신호전압, 전원용량 확인.

## 8. 전원·결선 안전
1. 보드 전원은 **USB, 5V+GND 헤더, 3V3+GND 헤더 중 한 가지만** 사용.
2. 모든 ESP32 GPIO는 3.3V 로직; 5V를 입력에 인가하지 않는다.
3. LED마다 전류제한 저항 필수, 극성 확인.
4. 전원을 제거한 상태에서 결선하고 GND 공통 확인.
5. 가변저항 가운데를 GPIO34에 연결하고 양끝은 3V3/GND.
6. 택트 스위치 동일측 두 다리의 내부 연결 여부 확인.
7. 수동부저 단품의 소비전류가 크면 트랜지스터 구동회로 필수.
8. GPIO와 외부 5V 센서·릴레이·모터를 직접 연결하지 않는다.

## 9. 이해도 점검
1. GPIO 입력과 출력은 어떻게 다른가?
2. UNO R3의 5V 핀 신호가 ESP32 GPIO에 위험한 이유는?
3. GPIO34에 LED를 연결해도 점등할 수 없는 이유는?
4. 12비트 ADC 4095는 어떤 숫자인가? 정확히 3.3V를 뜻하는가?
5. PWM 25%의 의미는 무엇인가?
6. `INPUT_PULLUP` 버튼은 눌렀을 때 HIGH인가 LOW인가?
7. 왜 LED에 직렬 저항이 필요한가?
8. ADC 측정과 PWM 출력의 역할을 순서대로 설명해 보라.

## 참고 공식 자료
- https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/user_guide.html
- https://docs.espressif.com/projects/arduino-esp32/en/latest/api/gpio.html
- https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html
- https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ledc.html
- https://docs.arduino.cc/hardware/uno-rev3/
- https://machinejw.tistory.com/21
