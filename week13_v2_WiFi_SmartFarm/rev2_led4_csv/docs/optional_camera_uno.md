## 선택 실습 · Arduino UNO 및 ESP32-S3 CAM

UNO는 5V 보드로 토양 AO 단독 측정 비교에 사용할 수 있습니다(센서 정격 확인, ESP32와 GPIO 직접 연결 금지). 필수 프로젝트의 센서/구동은 DevKitC V4 한 대에서 처리합니다.

ESP32-S3 CAM은 별도 장치로 식물/화분 영상 감시를 추가할 수 있습니다. microSD가 없으므로 영상 저장 실습은 하지 않습니다. ESP32-S3 CAM의 카메라 센서 표기(사용자가 적은 OVS2660은 모델 불명, OV2640/OV3660 가능성)와 제조사·정확한 핀맵·PSRAM을 먼저 확인합니다. Arduino IDE → File → Examples → ESP32 → Camera → CameraWebServer 열기, board_config.h에서 실제 모델과 일치하는 항목 하나만 선택 혹은 제조사 핀 설정 사용. SSID/PASS를 변경, 제조사 권장 USB 및 PSRAM 설정, 프로그래밍 후 시리얼 IP 확인, 같은 LAN 브라우저에서 http://CAM_IP 접속해 Start Stream. SD 카드 필요 없음. 실제 핀맵을 확인하지 않은 상태에서 특정 모델 코드를 강제로 선택하지 않습니다. [Espressif 카메라 예제](https://github.com/espressif/arduino-esp32/tree/master/libraries/ESP32/examples/Camera/CameraWebServer) / [지원 센서 목록](https://github.com/espressif/esp32-camera). 카메라 영상을 PC CSV에 저장하지 않으며, 스트리밍 화면은 별도 접속합니다.
