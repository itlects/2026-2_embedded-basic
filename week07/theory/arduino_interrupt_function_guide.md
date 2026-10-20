# 7주차 핵심 이론 보충: 실제 코드로 이해하는 Arduino 함수와 인터럽트
## 1. 함수·상수·사용자 변수 구분
- **Arduino 코어 제공**: `pinMode()`, `digitalRead()`, `millis()`, `noInterrupts()`, `interrupts()`, `attachInterrupt()`, `digitalPinToInterrupt()`, `Serial.begin()`, `Serial.print()`, `Serial.println()`.
- **Arduino 코어 상수**: `INPUT_PULLUP`, `HIGH`, `LOW`, `FALLING`.
- **사용자가 정의한 변수**: `level`, `paused`, `BUTTON_PINS`, `elapsedSeconds`, `interruptEvents`.
- **C/C++ 언어 요소**: `const`(수정 금지), `volatile`(비동기 변경을 컴파일러에 알림), `uint8_t`(부호 없는 8비트 정수), `uint32_t`(부호 없는 32비트 정수).
- **외부 라이브러리**: `Adafruit_NeoPixel`와 인스턴스의 `begin()`, `clear()`, `setPixelColor()`, `Color()`, `setBrightness()`, `show()`.
- **사용자 정의 함수**: `updateClock()`, `updateButtons()`, `processButton()`, `renderLeds()`, `printStatus()`는 Arduino 내장 함수가 아니라 이 실습에서 정의한 함수다.

## 2. Arduino 기본 함수
### 2.1 pinMode(pin, INPUT_PULLUP)
```cpp
pinMode(BUTTON_PINS[0], INPUT_PULLUP);
```
지정 핀을 입력으로 구성하고 내부 풀업을 활성화한다. 버튼 반대쪽을 GND에 연결하면 평상시 HIGH, 누르는 동안 LOW가 된다. ESP32 내부 풀업은 일반적으로 3.3V 논리 전압 기준이다.
### 2.2 digitalRead(pin)
```cpp
if (digitalRead(BUTTON_PINS[i]) == HIGH) {
  lowStartedAt[i] = 0;
}
```
입력 전압을 논리 HIGH/LOW로 읽는다. 본 실습에서는 ISR 발생 직후 곧바로 인정하지 않고, 30ms 이상 LOW를 유지하는지 확인하는 디바운싱 용도로 사용한다.
### 2.3 millis()
```cpp
uint32_t now = millis();
if ((uint32_t)(now - lastClockTick) >= 1000UL) {
  lastClockTick += 1000UL;
}
```
부팅 이후 경과한 밀리초를 반환한다. 반환값은 계속 증가하다가 약 49.7일 후 32비트 범위에서 순환한다. 부호 없는 정수로 **두 시각의 차**를 계산하면 일반적인 짧은 간격 측정에서 오버플로에 강하다. `delay(1000)`와 달리 루프에서 버튼 이벤트를 계속 처리할 수 있다. 이는 실제 시각을 유지하는 RTC가 아니다.
### 2.4 Serial.begin/print/println
```cpp
Serial.begin(115200);
Serial.print("[STATE] level=");
Serial.println(level);
```
시리얼 통신을 초기화하고 학습자가 실행 결과를 관찰하게 한다. `print()`는 줄바꿈 없이, `println()`은 줄바꿈까지 출력한다. 시리얼 모니터의 속도도 115200으로 맞춘다. ISR 안에서 Serial 출력을 수행하지 않는다.

## 3. 외부 인터럽트 API
```cpp
attachInterrupt(
  digitalPinToInterrupt(BUTTON_PINS[0]),
  onIncrease,
  FALLING
);
```
- `digitalPinToInterrupt(pin)`: Arduino 논리 핀 번호에서 인터럽트 번호/식별자를 얻는다.
- `attachInterrupt(interrupt, callback, mode)`: 콜백 ISR을 연결한다.
- `FALLING`: HIGH에서 LOW로 바뀔 때 발생하는 하강 에지를 뜻한다.
- UNO ATmega328P에서 `attachInterrupt()`가 다루는 외부 인터럽트는 주로 D2(INT0), D3(INT1)이다. D4까지 세 버튼을 모두 인터럽트 입력으로 사용하기 위해 본 통합 실습은 PCINT를 사용한다.
- `noInterrupts()`/`interrupts()`: 보통 단일 코어 AVR 환경에서 짧은 공유 데이터 복사 중 인터럽트를 차단/복원하는 데 쓰인다. 호출 전에 인터럽트가 비활성화되었을 가능성이 있으면 상태 복원을 고려해야 한다.

## 4. UNO ATmega328P PCINT 레지스터
```cpp
ISR(PCINT2_vect) {
  uint8_t current = PIND;
  uint8_t falling = previousPortD & (uint8_t)~current;
  interruptEvents |= (falling >> 2) & 0x07;
  previousPortD = current;
}
```
- `ISR(PCINT2_vect)`: AVR 하드웨어의 포트 D 핀 변경 인터럽트 벡터에 연결하는 매크로다. Arduino 기본 일반 함수가 아니라 AVR 코어 도구체인의 기능이다.
- `PIND`: 포트 D 핀의 현재 논리 상태를 담는 하드웨어 입력 레지스터다. D2·D3·D4는 비트 2·3·4에 대응한다.
- `PCMSK2`: 포트 D에서 어떤 비트의 변화가 PCINT를 유발할지 지정하는 마스크다. D2=PCINT18, D3=PCINT19, D4=PCINT20.
- `PCICR`: PCINT 그룹을 활성화한다. `PCIE2`는 포트 D 그룹 허용 비트다.
- `PCIFR`: 발생 플래그 레지스터이며 AVR에서는 해당 비트에 1을 써서 플래그를 지운다.
- `_BV(bit)`: 해당 비트에 1을 갖는 마스크를 만드는 AVR 매크로다.
- `falling`: 이전 상태는 1이고 현재 상태는 0인 비트만 추린다. ISR에서는 LED 출력·디바운스·시리얼 출력을 하지 않고 이벤트 플래그만 기록한다.

## 5. ESP32의 ISR과 임계구역
```cpp
void ARDUINO_ISR_ATTR onIncrease() {
  portENTER_CRITICAL_ISR(&eventMux);
  interruptEvents |= 0x01;
  portEXIT_CRITICAL_ISR(&eventMux);
}
```
- `ARDUINO_ISR_ATTR`: ESP32 Arduino 코어에서 ISR용 속성을 지정하는 매크로이며 코어 버전에 따라 정의가 달라질 수 있다.
- `portMUX_TYPE`: ESP32 FreeRTOS의 스핀락 제어에 사용하는 타입이다.
- `portENTER_CRITICAL_ISR()`/`portEXIT_CRITICAL_ISR()`: ISR 문맥에서 짧은 공유 데이터 변경 구간을 보호한다.
- 메인 루프는 이에 대응해 `portENTER_CRITICAL()`/`portEXIT_CRITICAL()`로 이벤트를 복사한 후 플래그를 비운다.
- ESP32는 멀티코어 환경일 수 있으므로 단순한 `volatile`만으로 동시 접근의 안전성을 보장하지 않는다.

## 6. NeoPixel 라이브러리
```cpp
Adafruit_NeoPixel bar(8, BAR_PIN, NEO_GRB + NEO_KHZ800);
bar.begin();
bar.setBrightness(50);
bar.clear();
bar.setPixelColor(0, bar.Color(0, 30, 0));
bar.show();
```
- 객체 생성자의 8은 **LED 개수**, BAR_PIN은 DIN 신호용 MCU 핀이다.
- `NEO_GRB`: 장치의 색상 바이트 순서, `NEO_KHZ800`: 프로토콜 속도 설정이다.
- `begin()`: NeoPixel 출력 초기화.
- `setBrightness()`: 전체 출력 밝기 스케일 설정(0~255). 교육용 실습은 낮은 밝기부터 시작한다.
- `clear()`: 라이브러리의 픽셀 버퍼를 0으로 초기화.
- `setPixelColor(index, color)`: 인덱스 0부터 LED 색상을 설정. **호출만으로는 실제 LED에 반영되지 않는다.**
- `Color(r,g,b)`: RGB 값을 LED 색상값으로 만든다.
- `show()`: 버퍼를 직렬 데이터로 전송하고 실제 LED 출력을 갱신한다. ISR 내부에서 실행하면 안 된다.
- WS2812 8LED + 12LED는 총 20개 LED이며 밝기/색상에 따라 외부 전원 요구량이 달라진다.

## 7. 사용자 정의 함수와 호출 관계
```text
setup()
 ├─ Serial.begin(115200)
 ├─ pinMode(..., INPUT_PULLUP)
 ├─ bar.begin() / ring.begin()
 └─ attachInterrupt(...) 또는 PCINT2 설정

loop()
 ├─ updateClock(now) → Serial.print/println()
 └─ updateButtons(now)
      ├─ takeInterruptEvents()
      ├─ digitalRead() → 디바운스
      └─ processButton()
           ├─ printStatus() → Serial.print/println()
           └─ renderLeds() → setPixelColor() → show()
```
`updateClock`와 `updateButtons`는 사용자가 만든 함수이고, `loop`가 반복 호출한다. `setup`·`loop`는 Arduino 스케치의 정해진 진입점 형태다.

## 8. 학생 코드 읽기 문제
1. `const uint8_t BAR_PIN = 6;`은 Arduino 내장 변수인가, 사용자가 정의한 상수인가?
2. 내부 풀업을 쓰는데 버튼을 누르면 LOW가 되는 까닭은?
3. `millis()`와 `delay()`의 차이는?
4. UNO의 PCINT2 벡터와 D2/D3의 INT0/INT1은 무엇이 다른가?
5. `volatile`이 있어도 왜 임계구역이 필요한가?
6. `setPixelColor()` 다음에 `show()`를 호출해야 하는 이유는?
7. ISR에서 `Serial.println()`이나 `show()`를 호출하지 않는 이유는?
8. 출력 로그 `[EVENT]`, `[STATE]`, `[CLOCK]`은 어떤 위치에서 생성되는가?

## 9. 참고 코드
- [UNO WS2812 인터럽트 통합 실습](../labs/uno_04_ws2812_interrupt/uno_04_ws2812_interrupt.ino)
- [ESP32 WS2812 인터럽트 통합 실습](../labs/esp32_04_ws2812_interrupt/esp32_04_ws2812_interrupt.ino)

**검증 범위:** 문법·API 의미에 대한 강의용 해설이다. 실제 하드웨어와 Arduino IDE의 코어·라이브러리 버전에 따른 빌드 및 실행 검증은 별도로 필요하다.
