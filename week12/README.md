# 12주차 · ESP32 Wi-Fi IoT 실습

대상: 동서울대학교 IT융합학과 2학년 / 3시간 / ESP32 DevKitC V4 / 학생 PC: Windows 11 Home.

1. docs/lecture.md에서 이론을 읽습니다. docs/physical_wiring.md에서 실제 배선과 센서 단독 확인을 진행합니다.
2. docs/workbook1.md: 스캔 → SoftAP 웹서버 → 모바일 센서 확인·LED 제어.
3. docs/workbook2.md: STA → HTTP POST → PC 수집·모바일 관제 → 선택 MySQL 저장.

firmware의 각 폴더와 같은 이름의 .ino를 Arduino IDE에서 엽니다. 서버는 Python 3.13 표준 라이브러리만 사용합니다. 인터넷 데이터 다운로드는 필요 없고 data/sample_readings.csv를 미리 제공합니다. Arduino ESP32 보드 패키지와 DHT 라이브러리는 수업 전에 설치합니다.

[지정 Notion 강의](https://app.notion.com/p/40db2e6bf8078082b031e7dec65ac171)

검증 범위와 남은 실물 확인은 docs/validation.md에 기록합니다.

검증 버전: Arduino-ESP32 3.3.2 / DHT 1.4.6 / Unified Sensor 1.1.15. 전체 4개 스케치 타깃 컴파일 통과. docs/learning_review.md에 학습 적합성 검토를 포함합니다.
