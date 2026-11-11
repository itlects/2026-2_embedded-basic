# 10주차 Wokwi 시뮬레이션 워크북 (ESP32-DevKitC V4)

Wokwi에는 **DHT11 부품이 없으므로 DHT22(AM2302)** 로 대체한다. 두 센서는 같은 단일 DATA선 타이밍 프로토콜을 쓰고, 숫자 해석(정수 vs 16비트×0.1)과 시작 신호 길이(18 ms vs 약 1 ms)만 다르다.

| 폴더 | 내용 | 핵심 학습 |
| --- | --- | --- |
| `WK1_DHT22_Alarm` | DHT22 + 부저(IO25) + LED(IO26) 경보 | 실물 실습 2·3과 같은 회로·논리를 슬라이더로 검증 |
| `WK2_DHT_Timing_LogicAnalyzer` | 라이브러리 없이 40비트 직접 해독 + 로직 분석기 | 시작 신호·응답 80/80µs·비트 0/1 펄스 폭 측정 |
| `WK3_OneWire_DS18B20_I2C_LCD` | DS18B20 2개(IO13, 1-Wire) + LCD1602(I²C 21/22) + DHT22(IO4) | 진짜 1-Wire ROM 주소, I²C 주소 스캔, DHT와 비교 |

## 프로젝트 여는 방법 (공통)

1. https://wokwi.com 접속 → 로그인(무료) → **New Project → ESP32** 선택.
2. 왼쪽 편집기의 `sketch.ino` 내용을 지우고 이 폴더의 `sketch.ino`를 붙여 넣는다.
3. `diagram.json` 탭을 열고 전체를 이 폴더의 `diagram.json`으로 바꾼다. (부품과 배선이 자동으로 배치됨)
4. 편집기 탭 옆 **▼ → New file → `libraries.txt`** 를 만들고 이 폴더 내용을 붙여 넣는다. (Library Manager에서 추가해도 됨)
5. ▶(Start Simulation) → 시리얼 모니터 확인.
6. **Save** 후 **Share** 링크를 과제 제출물에 첨부한다.

> 보드 부품 이름은 `board-esp32-devkit-c-v4`, 핀 이름은 `esp:4`, `esp:3V3`, `esp:GND.1`, `esp:5V` 형식이다.
> 부품이 겹쳐 보이면 드래그로 옮겨도 연결은 유지된다.

## 검증 상태

- 스케치는 호스트 컴파일러 + 스텁 헤더로 문법 검사만 수행했다. ESP32 툴체인 빌드와 Wokwi 실행은 강의 전 교수자가 한 번 확인해야 한다.
- WK2의 µs 값은 Wokwi CPU 속도 때문에 실제와 수 µs 다를 수 있다. 로직 분석기 파형 폭을 기준으로 삼는다.
