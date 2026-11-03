# Week 11 — ESP32 microSD CSV Logger 학생 워크북

## 목표
SPI 연결을 직접 구성하고 DHT11 온·습도 데이터를 5초 간격으로 microSD에 CSV로 저장한다.

## 준비물과 주의
ESP32-DevKitC V4 1개, **3.3V 전원/3.3V 논리 호환 확인된 SPI microSD 모듈** 1개, FAT32 microSD 1개, DHT11 모듈 1개, 브레드보드, 점퍼선, USB 케이블.
**모듈의 실제 실크스크린과 데이터시트를 반드시 대조**한다. 모든 GND를 공통 연결한다. ESP32 GPIO는 5V 비허용. 전원을 끄고 배선한다.

| microSD SPI 모듈 | ESP32 GPIO |
|---|---|
| VCC (3.3V 지원 확인) | 3V3 |
| GND | GND |
| CLK/SCK | GPIO18 |
| DO/MISO | GPIO19 |
| DI/MOSI | GPIO23 |
| CS | GPIO5 |

| DHT11 모듈 | ESP32 |
|---|---|
| VCC (3.3V 호환 확인) | 3V3 |
| GND | GND |
| OUT/DATA | GPIO27 |

벌크형 DHT11 4핀 센서는 별도 4.7k~10kΩ DATA 풀업이 필요할 수 있다. 모듈에는 보통 탑재되어 있다. GPIO6~11은 내부 Flash에 사용, GPIO34~39 입력전용, GPIO0/2/12/15 부트 스트래핑에 주의한다.

## Part A — SD만 시험
1. USB 분리, SPI 신호/전원 연결, SD카드 FAT32 확인.
2. Arduino IDE: ESP32 board package, Board 'ESP32 Dev Module' (실물 모델 점검), 시리얼 115200.
3. examples/01_sd_test/01_sd_test.ino 실행.
4. SD 용량과 test.txt 쓰기·읽기 출력 캡처.
5. 실패하면 CS/전원/카드포맷/MISO MOSI 뒤바뀜 확인.

## Part B — DHT11만 시험
1. 전원 분리 후 GPIO27 DATA 배선.
2. Library Manager에서 Adafruit DHT sensor library와 Adafruit Unified Sensor 설치.
3. examples/02_dht11_test/02_dht11_test.ino 실행.
4. 2초마다 온도·습도 출력 캡처; 센서에 가까이 숨을 불어 습도 변화를 관찰.

## Part C — CSV 로거
1. examples/03_csv_logger/03_csv_logger.ino 업로드.
2. 5초 주기로 최소 12행 생성; 업로드 후 로그 파일은 /dht11.csv.
3. 전원 분리 전 마지막 기록을 확인하고 보드의 전원을 끈 후 카드 제거.
4. 컴퓨터에서 CSV 파일을 열고 elapsed_ms,temperature_c,humidity_pct 열 확인.
5. 04_read_csv 예제로 보드에서 데이터 재읽기.

## Part D — 확장
1. INTERVAL_MS를 10000으로 바꾸고 전후 파일 크기/행 수 비교.
2. 온도 30°C 이상이면 별도 status 열에 HOT, 그렇지 않으면 NORMAL 출력.
3. 오류 발생 시 행을 건너뛰는 이유와 데이터 손실 가능성 토론.

## 제출 양식
회로 연결 사진, 시리얼 모니터 2장, CSV 파일, 데이터 표(최소 12행), 실행 코드, 오류 해결 내역, 결론을 제출한다.

| 질문 | 답변 |
|---|---|
| SPI에서 MOSI와 MISO의 방향은? | |
| CS를 LOW로 만드는 목적은? | |
| CSV 헤더는 왜 한 번만 생성하는가? | |
| millis()를 시간 기준으로 쓰면 실제 날짜가 되는가? | |
| 센서 읽기 NaN일 때 어떻게 처리하는가? | |
