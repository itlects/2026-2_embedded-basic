# 12주차 v2 · ESP32-S3 N16R8 Wi-Fi

Windows 11 Home / ESP32S3 Dev Module / Arduino-ESP32 3.3.2 / Flash 16MB / OPI PSRAM 8MB / Python 3.13.

1. docs/lecture.md: 학습 질문·이론·보드 설정·메모리 확인·Hello·LED 2개.
2. docs/workbook1.md: DHT11 단독 측정 → 모바일 AP 모니터링.
3. docs/workbook2.md: PC 샘플 수신 → STA 실측 전송 → CSV·모바일 제어.

배선: DHT11 GPIO4, LED1 GPIO1, LED2 GPIO2. 카메라·microSD 분리. 메모리35~37, USB19/20 및 부팅 핀을 외부 배선에 사용하지 않음. 기존 ESP32용 week12와 독립 폴더.

서버 실행: week12_v2 폴더에서 `py -3.13 server/server.py`.
사진 출처: images/SOURCES.md. 검증 범위: verification/VALIDATION.md.
