# 13주차 영상 인터페이스 — OV2640 + microSD
동서울대학교 IT융합학과 | 3시간 | Windows + Arduino IDE 2.x

## 오늘의 질문
- 640×480 픽셀의 RGB565 데이터가 CSV보다 얼마나 클까?
- 전원이 꺼져도 JPEG 사진을 다시 볼 수 있을까?

## 준비
Freenove ESP32-S3-WROOM N16R8 (카메라 FPC, 내장 microSD 슬롯), 호환 OV2640, USB 데이터 케이블, FAT32 microSD(8~32GB 권장), 리더기, 버튼, 점퍼 2개. 모델이 다르면 핀맵을 먼저 대조한다. 일반 S3 DevKitC-1에는 카메라/SD 슬롯이 없을 수 있다.

Arduino IDE 보드 매니저 esp32 by Espressif Systems 3.x 설치, ESP32S3 Dev Module, Flash 16MB, PSRAM OPI PSRAM, USB-UART 포트(COM), 115200 baud. 전원을 뽑은 상태에서 FPC 접점 방향과 잠금 레버 확인 후 장착, microSD 삽입. GPIO는 3.3V, 5V 입력 금지. GPIO35~37은 OPI PSRAM, GPIO19/20은 native USB, GPIO0/3/45/46은 strapping 주의. 카메라 GPIO4~13,15~18 및 SD GPIO38~40 중복 금지.

## 실습 순서
1. [카메라](01_CameraCapture/01_CameraCapture.ino): [배선도](diagrams/01_camera.svg). 업로드 후 Camera init: 0x0, JPEG 640 x 480, N bytes 확인. 카메라 내부 연결은 FPC를 통해 구성한다.
2. [SD 카드](02_SDCheck/02_SDCheck.ino): [연결도](diagrams/02_sd.svg). SD 용량과 Week13 SD OK 출력 확인. 전원을 끄고 PC 리더기에서 week13.txt 확인.
3. [사진 저장](03_PhotoSave/03_PhotoSave.ino): [연결도](diagrams/03_button.svg). USB를 뽑고 버튼의 서로 다른 접점을 GPIO2 및 GND에 연결한다. 업로드 후 Ready 표시 확인. 버튼을 3회 눌러 각각 놓거나 시리얼 c 입력. Saved /camera/IMG_00001.jpg 등의 로그 확인. 이전 번호는 덮어쓰지 않는다. 기록이 끝나면 전원을 끄고 SD를 빼서 Windows 사진 앱에서 3개 JPEG 확인.
4. [선택: DevKitC V4](04_DevKitC_SPI_SD/04_DevKitC_SPI_SD.ino): [SPI 배선도](diagrams/04_devkitc_spi.svg). Arduino IDE ESP32 Dev Module, 외장 3.3V SD 모듈 사용. 이 예제는 저장만 검증하며 카메라 FPC를 직접 연결하지 않는다. 일반 V4에는 카메라 전용 커넥터가 없고 WROOM-32에는 PSRAM이 없으므로 카메라 하드웨어는 별도 확인 필요.

## 이론
OV2640은 렌즈로 들어온 빛을 CMOS 센서에서 샘플링하고 내부 영상처리/압축 후 DVP 8-bit 병렬 데이터를 전달한다. SCCB는 설정용, XCLK는 센서 기준 클록, PCLK는 픽셀 데이터 샘플 타이밍, VSYNC/HREF는 프레임/행 경계를 표시한다. 640×480 RGB565는 614400B(600KiB), JPEG는 내용 및 품질에 따라 달라진다. PSRAM은 임시 프레임 버퍼, SD는 비휘발성 저장소. esp_camera_fb_get() 후 항상 esp_camera_fb_return() 호출. SD_MMC.setPins(39,38,40)로 1-bit 지정 후 FAT 파일 시스템을 마운트한다.

## 결과 보고
카메라 해상도와 JPEG 바이트, SD 용량, 저장 파일 3개의 이름· 크기, 회로 사진, 시리얼 출력, 사진 앱 확인 화면을 제출. RGB565 예상 크기와 JPEG 실제 크기 비교. 카메라 init 실패: FPC 방향·PSRAM·모델 핀 확인. SD mount 실패: 카드 FAT32/접점/핀 확인. 업로드 실패: 데이터 케이블/COM/드라이버 확인. 실물 컴파일과 실행은 수업 전 실제 보드에서 확인 필요.
 
## 출처
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/get-started/index.html
- https://docs.freenove.com/projects/fnk0083/en/latest/fnk0083/codes/C/Preface.html
- https://docs.freenove.com/projects/fnk0083/en/latest/fnk0083/codes/C/32_Camera_Web_Server.html
- https://github.com/Freenove/Freenove_ESP32_S3_WROOM_Board/blob/main/C/Sketches/Sketch_07.3_Camera_SDcard/camera_pins.h
