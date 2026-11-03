# 10주차 ESP32 DHT11 센서 인터페이스

학습목표: DHT11 독자적인 단일 데이터선 타이밍 프로토콜과 I²C의 차이 이해, 온습도 시리얼 측정, 부저 임계값 경고.

- ESP32-DevKitC V4: DHT11 DATA=GPIO4; 수동부저 +=GPIO25; 선택 LED=GPIO26(330Ω 직렬).
- 센서 전원=3.3V; 공통 GND; 단독 4핀 DHT11 사용 시 DATA→3V3 4.7k~10kΩ 풀업 필요; 3핀 모듈은 내장 여부 확인.
- Arduino IDE: ESP32 by Espressif Systems, Adafruit DHT sensor library, Adafruit Unified Sensor 설치.
- 예제 01 센서 출력, 02 경보/히스테리시스, 03 LED/CSV.
- DHT11은 I²C 센서도 Dallas 1-Wire 센서도 아님. I²C는 별도 SDA·SCL 두 신호선을 사용하는 버스.
- 하드웨어 동작은 미검증(실습실에서 검증해야 함).

[워크북](workbooks/Workbook10_DHT11.md) | [Espressif 핀맵](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/user_guide.html) | [DHT11 프로토콜](https://www.haoyuelectronics.com/Attachment/DHT11/DHT11.pdf)
