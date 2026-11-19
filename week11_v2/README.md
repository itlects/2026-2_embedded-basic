# 11주차 v2 — ESP32-S3-WROOM N16R8 SPI microSD / DHT11 CSV Logger

**특징:** 기존 week11(ESP32 클래식) 보존, 신규 week11_v2(ESP32-S3) 독립 구성.

- 기본: 외장 **3.3V-compatible SPI microSD 모듈**과 DHT11 사용, 5초 간격 CSV 로깅.
- SPI: GPIO12 SCK, GPIO13 MISO, GPIO11 MOSI, GPIO10 CS.
- DHT11: GPIO4 DATA, VCC 3V3, GND GND.
- 내부 저장용 SPI Flash와 외부 센서용 SPI 버스는 다르며 16 MB Flash, 8 MB PSRAM은 SD 카드 용량이 아니다.
- N16R8 모듈 GPIO35~37 등 내장 PSRAM/Flash 간섭 우려 핀 회피.
- **FNK0083 보드의 뒷면 내장 microSD 슬롯은 제조사 안내상 SDMMC, GPIO39 CLK/GPIO38 CMD/GPIO40 D0**: SPI 과제와 혼용 금지.
- 공급 전압과 레벨 시프터는 SD 모듈 회로별로 다름: 실제 부품의 3.3V VCC/3.3V logic 지원 확인 필수.
- Arduino IDE: 'ESP32S3 Dev Module', Flash 16MB, OPI PSRAM(실물 사양 확인), Adafruit DHT library + Unified Sensor 설치.
- SD 형식 FAT32, Serial 115200.
- 01_sd_spi_test -> 02_dht11_test -> 03_csv_logger -> 04_read_csv -> 05_onboard_sdmmc_optional 순서.
- millis()는 부팅 이후 경과시간이며 실제 시계 시간이 아니다. NTP/RTC가 없으면 CSV에 실제 날짜 기록 불가.

[학생용 단계별 워크북](workbooks/week11_v2_workbook.md)

[Espressif ESP32-S3 시작하기](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/get-started/index.html)

[Freenove FNK0083 기본 설명](https://docs.freenove.com/projects/fnk0083/en/latest/fnk0083/codes/C/Preface.html)

[Freenove SDMMC 장](https://docs.freenove.com/projects/fnk0083/en/latest/fnk0083/codes/C/28_Read_and_Write_the_SDcard.html)

실물 연결·실행 검증 전이며 보드의 실크스크린을 기준으로 배선해야 합니다.
