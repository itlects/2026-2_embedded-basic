# 12주차 v2 검증 기록

검증일: 2026-11-19

## ESP32-S3 컴파일

Arduino-ESP32 3.3.2, DHT sensor library 1.4.6, Adafruit Unified Sensor 1.1.15.
FQBN: `esp32:esp32:esp32s3:FlashSize=16M,PSRAM=opi,PartitionScheme=app3M_fat9M_16MB,CDCOnBoot=default,USBMode=hwcdc`

|예제|결과|사용 용량|
|---|---|---|
|00_board_check|통과|Sketch uses 289043 bytes (9%) of program storage space. Maximum is 3145728 bytes.|
|01_wifi_hello|통과|Sketch uses 898123 bytes (28%) of program storage space. Maximum is 3145728 bytes.|
|02_wifi_two_led|통과|Sketch uses 925347 bytes (29%) of program storage space. Maximum is 3145728 bytes.|
|03_dht_check|통과|Sketch uses 318215 bytes (10%) of program storage space. Maximum is 3145728 bytes.|
|04_dht_mobile_ap|통과|Sketch uses 926695 bytes (29%) of program storage space. Maximum is 3145728 bytes.|
|05_dht_pc_client|통과|Sketch uses 931547 bytes (29%) of program storage space. Maximum is 3145728 bytes.|

## PC 서버

Python 3.13.18에서 `python server/test_server.py` 실행: 테스트 5개 통과.
센서 POST/CSV 기록, 최신값 GET, LED 명령, 잘못된 입력 처리를 확인했습니다.
대시보드와 펌웨어 내 JavaScript는 Node.js `--check` 구문 검사 통과.
배선 그림의 GPIO4/1/2, LED 저항·극성, 공통 GND와 표·코드를 대조했습니다.

## 실물 확인 범위

실물 보드 업로드, N16R8 메모리 출력, 실제 DHT11 측정, 모바일 Wi-Fi 접속과 LED 점등, Windows 방화벽 조작은 현장 실행하지 않았습니다. 워크북의 단계별 통과 기준으로 수업 전 확인해야 합니다. 컴파일 통과는 실물 동작 검증을 대신하지 않습니다.
