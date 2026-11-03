# Week 11 — SPI 저장장치와 DHT11 CSV 데이터 로거

**보드:** ESP32-DevKitC V4 (ESP32-WROOM 계열). **프로젝트 목표:** DHT11 온습도 값을 5초마다 microSD카드의 `/dht11.csv`에 기록.

## 회로
- microSD SPI: SCK=GPIO18, MISO=GPIO19, MOSI=GPIO23, CS=GPIO5, GND=GND, VCC=3V3(3.3V 호환 모듈만).
- DHT11 모듈: DATA=GPIO27, VCC=3V3, GND=GND.
- GPIO 5는 부팅 스트래핑 핀이기도 하므로 부팅 문제 발생 시 CS 핀/모듈 회로를 재검토하고 GPIO 13 등으로 이동한 뒤 코드와 배선을 함께 변경할 것.
- GPIO는 3.3V 신호만 사용; 5V 출력형 SD모듈을 직접 연결 금지. **USB 연결 전 배선 점검**.
- 3.3V 전원출력의 전류 여유가 충분한지 모듈 사양 확인.

## 수업 절차
1. SPI 원리와 핀 설명 → 2. SD 입출력 → 3. DHT11 측정 → 4. CSV 데이터 로깅 → 5. 저장값 확인 → 6. 학생 워크북.
각 예제는 독립 스케치이며, 새 스케치 업로드 시 기존 실행은 중단된다. 로그 파일은 일반 FAT 파일로 유지된다.

## 코드
- [01_sd_test](examples/01_sd_test/01_sd_test.ino)
- [02_dht11_test](examples/02_dht11_test/02_dht11_test.ino)
- [03_csv_logger](examples/03_csv_logger/03_csv_logger.ino)
- [04_read_csv](examples/04_read_csv/04_read_csv.ino)
- [학생 워크북](workbooks/workbook.md)

## 실행 조건과 결과 판정
Arduino IDE에서 ESP32 보드 패키지, **Adafruit DHT sensor library**, **Adafruit Unified Sensor** 설치. microSD카드는 FAT32로 준비. Serial Monitor 115200bps.
CSV 헤더: `elapsed_ms,temperature_c,humidity_pct`. `elapsed_ms`는 부팅 이후 밀리초이며 실제 일시가 아님. RTC 또는 NTP 없이 실제 날짜/시간은 기록 불가.
실물 보드·센서·카드 조합에서의 실행 검증은 별도 수행 필요.

## 참고
- [Espressif DevKitC V4](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/user_guide.html)
- [Espressif Arduino SD](https://github.com/espressif/arduino-esp32/blob/master/libraries/SD/README.md)
- [Adafruit DHT Library](https://github.com/adafruit/DHT-sensor-library)
