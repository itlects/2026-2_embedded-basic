# 실제 배선 가이드

![실제 배선](../images/02_ap_wiring.png)

## 실제 배선 위치 따라하기

그림의 ESP32는 38핀 DevKitC V4이며 USB가 아래쪽입니다. 보드가 브레드보드 구멍을 가리지 않도록 바깥에 둡니다. 그림의 선이 지나가는 위치가 아닌 선 끝의 연결점을 꽂습니다. 3핀 DHT11은 그림의 VCC-DATA-GND 순서 제품 기준이며 보유 모듈의 인쇄 라벨이 우선입니다.

|순서|한쪽 끝|다른 쪽 끝|확인|
|---|---|---|---|
|1|ESP32 GND|상단 − 전원 레일 1번 구멍|공통 접지|
|2|ESP32 3V3|상단 + 전원 레일 1번 구멍|5V 핀 사용 금지|
|3|DHT11 VCC / DATA / GND|a6 / a7 / a8|모듈 순서가 다르면 라벨대로 재배치|
|4|상단 + 전원 레일 6번|e6|VCC에 3.3V|
|5|상단 − 전원 레일 8번|e8|센서 접지|
|6|ESP32 GPIO27|e7|센서 데이터|
|7|ESP32 GPIO26|a16|LED 출력 시작|
|8|330Ω 저항 양 끝|e16 / e20|열16과 열20 사이 연결|
|9|LED A(긴 다리) / K(짧은 다리)|b20 / b21|서로 다른 열에 꽂기|
|10|a21|상단 − 전원 레일 28번|LED 접지|

a~e의 같은 번호 열은 내부로 연결되어 있습니다. f~j는 별도이며 중앙 홈을 건너 연결되지 않습니다. 모듈을 뽑은 상태에서 멀티미터 도통으로 a6-e6 연결, a6-a7 분리를 확인하면 이해에 도움이 됩니다. 전원 레일에 중간 단절이 있는 브레드보드는 사용 구간 도통을 확인합니다.

## 0단계 · 네트워크보다 먼저 센서 확인 (10분)

1. 아래 실제 배선 위치 표대로 연결합니다. USB를 분리한 상태에서 배선하고 전원·GND를 다시 점검합니다.
2. Arduino IDE의 라이브러리 관리자에서 DHT sensor library 1.4.6 및 Adafruit Unified Sensor 1.1.15를 설치합니다. 보드 패키지는 검증에 사용한 ESP32 by Espressif Systems 3.3.2를 선택할 수 있습니다.
3. `firmware/00_dht_check/00_dht_check.ino`를 열고 ESP32 Dev Module·COM 포트를 선택합니다. 확인(컴파일) 후 업로드합니다.
4. 시리얼 모니터 115200에서 약 2.5초마다 `T=24.0 C, H=52.0 %` 형태의 값을 3회 확인합니다. 숫자는 환경에 따라 다릅니다.
5. `FAIL`이 나오면 네트워크 단계로 넘어가지 말고 DHT11/DHT22 종류, DATA=GPIO27, 3.3V 지원 여부, GND, 풀업을 먼저 확인합니다.

```cpp
#include <DHT.h>
#include <math.h>
DHT dht(27, DHT11);
unsigned long lastSample = 0;
void setup() {
  Serial.begin(115200);
  dht.begin();
  Serial.println("DHT11 check: GPIO27, 3.3V, common GND");
}
void loop() {
  if (millis() - lastSample < 2500) return;
  lastSample = millis();
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(t) || isnan(h)) {
    Serial.println("FAIL: check DHT type, DATA, power and pull-up");
    return;
  }
  Serial.printf("T=%.1f C, H=%.1f %%\n", t, h);
}
```

통과 기준: 오류 문구 없이 온습도 숫자 3회 출력. 학생은 `isnan()`이 필요한 이유를 한 문장으로 작성합니다.

