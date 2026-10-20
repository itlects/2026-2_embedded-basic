# 워크북 5: DHT11 온습도 센서 기반 인터럽트 경보 시스템

## 학습 목표
DHT11 온습도를 읽고 LED 5개로 습도 수준을 표시한다. 버튼 4개는 인터럽트 입력으로 경보 켜기·끄기, 습도 임계값 높이기·낮추기, 부저 음소거를 제어한다. `millis()`를 사용하여 센서 측정과 경보음을 비차단으로 처리한다.

## 준비물과 주의
DHT11 **온습도 센서** 1개(가습기 아님), Arduino UNO 또는 ESP32 1개, LED 5개, 220~330Ω 저항 5개, 택트 버튼 4개, 저전류 수동 피에조 부저 1개, 브레드보드·점퍼선·USB 케이블.
DHT11은 약 1Hz 이하 수준의 저속 센서이므로 이 예제는 2500ms 간격으로 측정한다. 3핀 모듈은 VCC/DATA/GND 인쇄를 확인하며, **4핀 단품 센서**라면 데이터 풀업 저항(통상 4.7~10kΩ)을 추가한다. ESP32의 센서 VCC는 3.3V를 사용하여 데이터 HIGH 전압이 3.3V가 되도록 한다. UNO는 5V 센서 전원을 사용할 수 있다.

## 연결표
|역할|UNO|ESP32|전기 연결|
|---|---|---|---|
|DHT11 DATA|A0(디지털 입력으로 사용)|GPIO32|DATA|
|DHT11 VCC|5V|3.3V|센서 정격 확인|
|DHT11 GND|GND|GND|공통 접지|
|버튼 A 경보 ON/OFF|D2|GPIO18|입력→버튼→GND|
|버튼 B 임계값 +5%|D3|GPIO19|입력→버튼→GND|
|버튼 C 임계값 -5%|D4|GPIO21|입력→버튼→GND|
|버튼 D 음소거|D5|GPIO22|입력→버튼→GND|
|LED1~LED5|D8~D12|GPIO13,14,25,26,27|출력→각각 220~330Ω→LED 양극, 음극→GND|
|수동 피에조 부저|D6|GPIO23|저전류 피에조 양극→신호, 음극→GND|

전류 소모가 큰 자기식 부저·모듈은 GPIO 직접 구동하지 않고 트랜지스터 드라이버를 사용한다. 모든 부품의 GND는 공통이어야 한다.

## 한글 연결 개략도
![UNO DHT11·버튼·LED·부저 연결도](https://raw.githubusercontent.com/itlects/2026-2_embedded-basic/main/week07/images/workbook05_uno_dht11_alarm_ko.svg)
![ESP32 DHT11·버튼·LED·부저 연결도](https://raw.githubusercontent.com/itlects/2026-2_embedded-basic/main/week07/images/workbook05_esp32_dht11_alarm_ko.svg)

## 주요 함수 이론
- `DHT dht(DHT_PIN, DHT11)`: DHT 센서 라이브러리 객체 선언.
- `dht.begin()`: 센서 라이브러리 초기화.
- `dht.readHumidity()`: 상대습도(%) 측정.
- `dht.readTemperature()`: 섭씨 온도(°C) 측정.
- `isnan()`: 측정 실패(NaN) 확인.
- `INPUT_PULLUP`: 버튼을 떼면 HIGH, 누르면 LOW.
- UNO: `ISR(PCINT2_vect)`, ESP32: `attachInterrupt(..., FALLING)`은 ISR 입력.
- `tone()`/`noTone()`: 저전류 피에조 경보음 출력/중단.
- `millis()`: 2500ms 측정, 35ms 버튼 디바운스, 경보음 120ms 간격 제어.
- `Serial.print()`/`println()`: 온도, 습도, 설정, 센서 오류 출력.

## 실습 순서
1. USB를 뽑고 센서의 실제 VCC/DATA/GND 표기를 읽는다. 버튼 4개와 LED 5개(각 저항 포함), 수동 부저를 표대로 결선한다.
2. Arduino IDE 라이브러리 관리자에서 **DHT sensor library**와 **Adafruit Unified Sensor**를 설치한다.
3. [UNO 전체 코드](https://github.com/itlects/2026-2_embedded-basic/blob/main/week07/labs/uno_06_dht11_alarm/uno_06_dht11_alarm.ino) 또는 [ESP32 전체 코드](https://github.com/itlects/2026-2_embedded-basic/blob/main/week07/labs/esp32_06_dht11_alarm/esp32_06_dht11_alarm.ino)를 열어 업로드한다.
4. Serial Monitor를 115200bps로 설정한다. 기본 임계값 40%, 경보 ON, 음소거 OFF임을 확인한다.
5. 측정 습도가 0~20/20~40/40~60/60~80/80~100%일 때 대략 LED 1~5개가 켜지는지 관찰한다(0%에서는 0개).
6. A 버튼으로 경보 ON/OFF, B/C 버튼으로 임계값 20~90% 범위에서 5%씩 증감, D 버튼으로 부저 음소거를 시험한다.
7. 임계값을 현재 습도보다 높여 경보를 활성화하고 부저 경보음과 Serial 출력이 나오는지 확인한다.
8. DHT DATA를 잠시 분리하는 시험은 **전원을 끈 상태에서 배선을 변경**한 후 다시 켜서 오류 출력을 관찰한다.

## 예상 Serial 출력(실측 결과 아님)
```text
DHT11 Alarm A:on/off B:+5 C:-5 D:mute
[EVENT] READY
[CONFIG] threshold=40% alarm=ON mute=OFF
[SENSOR] temp=24.0C humidity=35.0%
[ALARM] LOW HUMIDITY
[EVENT] B THRESHOLD +5
[CONFIG] threshold=45% alarm=ON mute=OFF
[EVENT] D MUTE TOGGLE
[CONFIG] threshold=45% alarm=ON mute=ON
```
측정값, LED 개수, 이벤트 출력 순서는 센서 주변 환경과 버튼 조작에 따라 달라진다.

## 실습 체크리스트
- [ ] 보드별 DHT11 핀과 전원 정격 확인
- [ ] 버튼 A~D 및 LED 1~5 연결
- [ ] 시리얼 온습도 값 관찰
- [ ] 습도에 따른 LED 점등 개수 확인
- [ ] 임계값 변경, 경보 ON/OFF, 음소거 확인
- [ ] 센서 통신 오류 관찰 및 분석
- [ ] 회로 사진·Serial 출력·실험보고서 제출

**검증 범위:** 코드와 연결표에 대한 설계 자료이다. Arduino IDE 실제 컴파일·실물 센서 테스트는 수행되지 않았다.
