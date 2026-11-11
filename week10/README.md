# 10주차 · 센서 인터페이스(DHT 단일선 / 1-Wire / I²C) — DHT11 온습도 측정과 부저 경보

**학습목표** 디지털 센서의 타이밍 기반 단일선 통신을 이해하고 온습도 데이터를 취득한다.
**실습** DHT11 온습도 읽기 및 시리얼 출력, 임계값 초과 시 부저 경고 (보드: ESP32-DevKitC V4)

## 사용 핀 (실제 헤더 위치)

| ESP32 핀 | 헤더 | 연결 |
| --- | --- | --- |
| 3V3 | J2-1 | DHT11 VCC |
| GND | J3-1 / J2-14 | 모든 GND |
| IO4 | J3-13 | DHT11 DATA (단품은 10kΩ 풀업 → 3V3) |
| IO25 | J2-9 | 수동 피에조 부저 + |
| IO26 | J2-10 | 330Ω → LED + |
| IO13 / IO21 / IO22 | J2-15 / J3-6 / J3-3 | Wokwi C: 1-Wire DQ / I²C SDA / SCL |

## 폴더

```
week10/
├─ examples/   실물 실습 코드 (Arduino IDE, ESP32 Dev Module, 115200 baud)
│   01_DHT11_Serial.ino                 온습도 시리얼 출력
│   02_DHT11_Buzzer_Alert.ino           임계값·히스테리시스 부저 경고
│   03_DHT11_Dashboard_Extension.ino    LED + CSV 로깅
│   04_DHT11_Advanced_State_Machine.ino 이동평균·3상태 상태기계(심화)
│   05_DHT_Raw_Timing_Decoder.ino       라이브러리 없이 40비트 직접 해독(이론 확인)
├─ wokwi/      Wokwi 시뮬레이션 3종 (sketch.ino + diagram.json + libraries.txt)
├─ images/     그림(SVG 원본 + PNG) 및 생성 스크립트(gen_wiring.py, gen_theory10.py)
├─ diagrams/   (구버전 개념도 — images/ 의 실체 배선도로 대체)
└─ workbooks/  학생 워크북(Markdown)
```

## 그림 목록 (images/)

| 파일 | 내용 |
| --- | --- |
| t0_devkitc_week10_pinmap_ko | DevKitC V4 실제 J2/J3 위치와 이번 주 사용 핀·핀 선택 원칙 |
| t1_dht11_anatomy_ko | DHT11 내부 구조, 4핀 단품·3핀 모듈, DHT11 vs DHT22 |
| t2_dht11_timing_ko | 시작–응답–40비트 전체 타이밍, 비트 0/1, 풀업·오픈드레인 원리 |
| t3_dht11_frame_ko | 40비트 프레임 해독·체크섬 계산 예 |
| t4_interface_compare_ko | DHT 단일선 · Dallas 1-Wire · I²C 비교 |
| t5_hysteresis_ko | 임계값과 히스테리시스 |
| t6_buzzer_ko | 능동/수동 부저, tone() 파형, NPN 드라이버 |
| t7_state_machine_ko | millis() 루프와 3상태 상태기계 |
| ex1/ex2/ex3_*_wiring_ko | 실습별 실체 배선도(실제 핀 위치) |
| ex1/ex3_*_schematic_ko | 회로도(풀업·바이패스 포함) |
| wk_a/b/c_*_ko | Wokwi 워크북 A·B·C 연결도 |

## 라이브러리

- 실물: **DHT sensor library** (Adafruit) + **Adafruit Unified Sensor**
- Wokwi: **DHT sensor library for ESPx**(DHTesp), OneWire, DallasTemperature, LiquidCrystal I2C

## 검증 상태

코드는 스텁 헤더로 문법 검사만 했다. 실제 ESP32 업로드·센서 측정·Wokwi 실행은 수업 전 확인 필요.
DHT11은 I²C 장치도 Dallas 1-Wire 장치도 아닌 **고유 단일선 프로토콜**이다(1-Wire 실습은 Wokwi C의 DS18B20).

[Espressif DevKitC V4 핀맵](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/user_guide.html) · [Wokwi DHT22](https://docs.wokwi.com/parts/wokwi-dht22) · [Wokwi DS18B20](https://docs.wokwi.com/parts/wokwi-ds18b20) · [Wokwi Logic Analyzer](https://docs.wokwi.com/parts/wokwi-logic-analyzer)
