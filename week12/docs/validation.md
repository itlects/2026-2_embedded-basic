# 검증 기록 · 2026-11-18 보완판

## 실제 ESP32 타깃 컴파일

Arduino CLI 1.6.0 / Arduino-ESP32 3.3.2 / FQBN esp32:esp32:esp32 / DHT sensor library 1.4.6 / Adafruit Unified Sensor 1.1.15.

|스케치|결과|Flash 사용|RAM 사용|
|---|---|---|---|
|00_dht_check|통과|283295 bytes|20792 bytes|
|01_scan|통과|883759 bytes|43360 bytes|
|02_mobile_ap|통과|937247 bytes|45056 bytes|
|03_pc_client|통과|941463 bytes|46024 bytes|

`verification/compile_*.txt`에 컴파일 요약을 포함한다. 정적 검토만 한 것이 아니라 ESP32용 컴파일러로 바이너리 링크까지 완료했다. 초기 시도에서 다운로드·디스크 공간 제약이 있었으나 classic ESP32 빌드에 필요한 패키지를 설치해 검증을 완료했다.

## PC 및 자료 검증

- Python 3.12.14에서 HTTP 실제 요청 기반 서버 테스트 5종 재통과: 수신·CSV·최신값, 장치별 명령, 입력 오류, 화면/404, 저장 실패 503.
- PC dashboard 및 ESP32 내장 페이지의 JavaScript 구문 검사 통과.
- GitHub 원본과 Notion 워크북의 기존 코드 대조 후 수정 코드를 동기화했다.
- 새 배선도를 렌더링하고 GPIO27/26, a6/a7/a8, 저항 e16-e20, LED b20/b21, 공통 GND의 연결을 검토했다.

## 미검증 범위와 현장 확인

실물 ESP32·DHT11의 전기적 동작, 모바일 Wi-Fi 접속, MySQL Community 8.0.46 연결은 이 환경에 없어 실물 실행을 완료하지 못했다. Python 3.13에서는 수업 PC에서 아래 명령으로 확인한다. 컴파일 성공이 센서 핀 순서·접촉 불량·공유기 격리·DB 계정까지 보장하지는 않는다.

```powershell
py -3.13 server/test_server.py
```

1. 보유 모듈의 VCC/DATA/GND와 3.3V 지원·풀업을 확인한다.
2. 00_dht_check의 온습도 3회 → AP 모바일 접속·LED → PC POST 201·CSV 순서로 확인한다.
3. 선택 MySQL 활성화 후 SELECT에 새 행이 생기는지 확인한다.

## 재컴파일 명령

```text
arduino-cli compile --fqbn esp32:esp32:esp32 week12/firmware/00_dht_check
arduino-cli compile --fqbn esp32:esp32:esp32 week12/firmware/01_scan
arduino-cli compile --fqbn esp32:esp32:esp32 week12/firmware/02_mobile_ap
arduino-cli compile --fqbn esp32:esp32:esp32 week12/firmware/03_pc_client
```
