# 11주차 v2 — SPI + DHT11 CSV 데이터 로거 학생 워크북

대상: Windows 10/11, Freenove ESP32-S3-WROOM N16R8 (예상), Arduino IDE. 이 워크북은 외장 SPI microSD 모듈 전용이다.

## 학습 질문
1. 전원 재연결 후에도 측정값을 남길 방법은?
2. SPI의 SCK, MOSI, MISO, CS 네 신호는 어떤 방향으로 움직이나?
3. SDMMC와 SPI가 같은 프로토콜인가?
4. 5초마다 데이터를 기록하려면 delay()와 millis() 중 어느 방법이 적합한가?

## 부품 및 연결
ESP32-S3 N16R8 1, **3.3V 전원·3.3V 신호 지원이 확인된 SPI microSD 모듈** 1, microSD 카드(FAT32) 1, DHT11 모듈 1, 브레드보드·점퍼선·USB 케이블, 필요 시 DATA 풀업 4.7~10kΩ.

| SPI microSD 핀 | ESP32-S3 GPIO | 기능 |
|---|---|---|
| VCC | 3V3 | 확인된 모듈만 |
| GND | GND | 공통 접지 |
| SCK / CLK | GPIO12 | 클록 |
| MISO / DO | GPIO13 | SD→ESP32 |
| MOSI / DI | GPIO11 | ESP32→SD |
| CS / SS | GPIO10 | 장치 선택 |
| DHT11 VCC | 3V3 | 3.3V 동작 확인 |
| DHT11 GND | GND | 접지 |
| DHT11 OUT / DATA | GPIO4 | 데이터 |

GPIO19/20은 내장 USB, GPIO0/3/45/46은 부트·스트래핑, 35~37은 N16R8의 PSRAM/Flash 용도로 충돌할 수 있어 피한다. **Freenove 내장 microSD 슬롯은 기본적으로 SDMMC(GPIO38/39/40)를 사용**하므로 외장 SPI 모듈 없이 01~04번 코드는 작동하지 않는다.

## 준비 (15분)
1. PC에서 Arduino IDE 실행. File > Preferences > Additional Boards Manager URLs에 Espressif 공식 ESP32 package index URL 추가.
2. Tools > Board > Boards Manager에서 esp32 by Espressif Systems 설치.
3. Tools > Board에서 ESP32S3 Dev Module 선택.
4. Tools > Flash Size에서 16MB, PSRAM에서 OPI PSRAM(실물 제품 사양 확인) 설정.
5. USB CDC On Boot는 연결 포트에 따라 Enabled 시리얼 설정 시험. COM 포트 선택 후 예제 업로드.
6. 라이브러리 관리자에서 'DHT sensor library by Adafruit'와 'Adafruit Unified Sensor' 설치.
7. microSD 카드 FAT32 백업/포맷 후 USB를 빼고 배선.

## 실습 1: SD만 연결 (20분)
1. SD와 6선 연결. USB는 연결하지 않는다.
2. 01_sd_spi_test.ino 열기/업로드.
3. Serial Monitor를 115200으로 설정. 카드 크기와 'Hello from ESP32-S3 SPI!'를 캡처한다.
4. 리셋 후 문자열이 1행 더 추가됐는지 확인한다.
5. 실패하면 SD 모듈의 전원 정격, CS, DI/DO 방향, FAT32, 접지를 점검한다.

## 실습 2: DHT11만 연결 (20분)
1. USB 분리. DHT11 3선을 연결한다.
2. 02_dht11_test.ino 업로드.
3. 2초 간격 출력과 측정변화 캡처.

## 실습 3: 통합 데이터 로거 (30분)
1. USB를 제거하고 SD 6선+DHT11 3선을 연결한다.
2. 03_csv_logger.ino 업로드.
3. 5초마다 'LOG ...' 메시지가 출력되는지 확인한다.
4. 12행 이상 기록(약 1분). 전원을 끈 후 카드를 제거한다.
5. PC에서 'dht11.csv'를 열고 헤더와 측정행을 확인한다.
6. elapsed_ms와 temperature_c, humidity_pct 2개 그래프를 그린다.

## 실습 4: 저장 데이터 읽기 (15분)
1. SD 카드를 보드에 다시 넣는다(전원 차단 후).
2. 04_read_csv.ino 업로드.
3. 파일의 전체 CSV 출력과 PC의 CSV 내용이 같은지 비교한다.

## 선택: 내장 슬롯은 SDMMC, SPI가 아님
FNK0083 계열이면서 microSD 슬롯이 보드 뒷면에 있는 경우 05_onboard_sdmmc_optional.ino를 실행하여 SD_MMC 코드의 차이를 관찰한다. 복수 보드 모델에서는 핀을 확인하고 실행한다.

## 과제
- 로깅 주기를 5초에서 10초로 변경하고 1분간 데이터 수 비교.
- 온도 30°C 이상 HOT, 미만 NORMAL을 status 열로 추가.
- 센서 읽기 실패 및 SD 제거 시 어떤 오류가 발생하는지 논의(동작 중 SD 제거 실험은 금지).

## 결과 제출
회로 실물 사진 1장, Serial Monitor 화면 2장, CSV 원본(12행 이상), 그래프, 변경된 코드, 오류와 해결, 다섯 가지 학습 질문에 대한 답.

**검증범위:** 코드·배선 설계 자료이며 특정 현물 보드/SD 모듈에서의 실제 구동은 미검증.
