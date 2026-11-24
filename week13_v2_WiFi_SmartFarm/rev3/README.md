# 13주차 rev3 · Wi-Fi 스마트팜 (ESP32 DevKitC V4)

| 폴더 | 내용 |
|---|---|
| examples/ex1_soil | 예제 1 토양수분센서 AO/DO 읽기·보정 |
| examples/ex2_pump | 예제 2 릴레이로 워터펌프 2초 구동 |
| examples/ex3_dht22 | 예제 3 DHT22 온습도 |
| examples/ex4_web_csv | 예제 4 ESP32 웹서버 + LED4·버튼4·DHT22 + LittleFS CSV |
| workbook/smartfarm | 워크북 스마트팜 통합 (LCD, 부저, 자동 급수, 웹 제어, CSV) |
| wokwi/* | Wokwi용 diagram.json · sketch.ino(Wokwi-GUEST) · libraries.txt · partitions.csv |
| images | 브레드보드 배선 그림, Wokwi 화면, ESP32-S3-CAM 핀맵 |

핀: 토양 AO 34 / DO 35, DHT22 19, LCD SDA 21 / SCL 22, LED 14·13·4·18, 버튼 32·33·25·26, 부저 27, 릴레이 IN 23.
펌프 전원은 Arduino UNO 5V → 릴레이 COM, NO → 펌프(+), 펌프(−) → UNO GND.

이전 rev2_led4_csv(PC Python 서버 방식)는 참고용으로 남겨 두었습니다.
