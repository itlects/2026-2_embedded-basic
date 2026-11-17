# 12주차 · Wi-Fi 온습도 모니터 만들기

오늘은 휴대폰으로 DHT11 온습도를 보고 LED를 켜고 끕니다. 이어서 PC로 측정값을 전송합니다.

## 1. 준비물

Windows 11 Home PC, ESP32 DevKitC V4, 데이터 USB 케이블, 3.3V 지원 DHT11 모듈, LED 1개, 330Ω 저항 1개, 브레드보드·점퍼선, 휴대폰.

## 2. 실습 파일 받기

[전체 실습 ZIP 다운로드](https://drive.google.com/file/d/1WpO4QQ1IhXnyxXSASvCc1Mx113N7q-kV/view)

1. ZIP을 다운로드합니다.
2. 파일을 우클릭 → **모두 압축 풀기**를 누릅니다.
3. 압축을 푼 폴더 안의 `week12`를 엽니다.

## 3. Arduino IDE 설정

1. Arduino IDE 2.x를 실행합니다.
2. **파일 → 기본 설정 → 추가 보드 관리자 URL**에 다음 주소를 넣습니다.

```text
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

3. **보드 관리자**에서 `esp32 by Espressif Systems` **3.3.2**를 설치합니다.
4. **라이브러리 관리자**에서 다음 두 라이브러리를 설치합니다.

|라이브러리|버전|
|---|---|
|DHT sensor library by Adafruit|1.4.6|
|Adafruit Unified Sensor|1.1.15|

5. ESP32를 USB로 연결합니다.
6. **도구 → 보드 → ESP32 Dev Module**을 선택합니다.
7. **도구 → 포트**에서 ESP32의 COM 포트를 선택합니다.
8. **시리얼 모니터 속도**는 `115200`으로 맞춥니다.

## 4. 실습 순서

1. **워크북 1:** 배선 → 센서 확인 → Wi-Fi 검색 → 휴대폰 접속·LED 제어
2. **워크북 2:** PC 서버 실행 → 센서값 전송 → 휴대폰 관제
3. 시간이 남으면 워크북 2의 **선택 실습: MySQL 저장**을 진행합니다.

## 실습 전에 알아둘 것

|항목|이번 실습에서의 의미|
|---|---|
|AP|휴대폰이 ESP32에 직접 접속하는 방식|
|STA|ESP32가 공유기에 접속하는 방식|
|GET / POST|값 조회 / 센서값·제어 명령 전송|
|millis()|일정 간격으로 센서를 읽을 때 사용|
|isnan()|센서 읽기 실패 확인|
|GPIO27 / GPIO26|DHT11 데이터 / LED 출력|

## 제출

배선 사진 1장, 모바일 화면 1장, `POST: 201` 시리얼 화면 1장을 제출합니다.
