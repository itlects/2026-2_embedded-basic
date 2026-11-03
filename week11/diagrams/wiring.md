# SPI 배선 다이어그램 (원리도)

```text
ESP32 DevKitC V4           microSD SPI (3.3V compatible)
3V3   ------------------>  VCC
GND   ------------------>  GND
GPIO18 ------------------>  SCK / CLK
GPIO19 <------------------  MISO / DO
GPIO23 ------------------>  MOSI / DI
GPIO5  ------------------>  CS

ESP32 DevKitC V4           DHT11 module
3V3   ------------------>  VCC
GND   ------------------>  GND
GPIO27 <----------------->  DATA / OUT
```

실제 사용 전 SD 모듈과 DHT11의 실물 핀 라벨 및 동작 전압을 확인하세요. 표시된 핀 번호는 ESP32 GPIO 번호이며 물리 헤더 순번이 아닙니다. DHT11은 단일 디지털 데이터선으로 동작합니다. 모든 접지는 공통 연결합니다.