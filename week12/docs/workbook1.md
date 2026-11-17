# 워크북 1 · 휴대폰으로 온습도 확인·LED 제어

[전체 실습 ZIP 다운로드](https://drive.google.com/file/d/1WpO4QQ1IhXnyxXSASvCc1Mx113N7q-kV/view)

## 1. 회로 연결하기

USB를 뽑고 아래 그림대로 연결합니다. DHT11 핀 순서는 제품마다 다르므로 모듈의 VCC·DATA·GND 표시를 확인합니다.

![실제 연결 위치](https://raw.githubusercontent.com/itlects/2026-2_embedded-basic/83096682d7244f83f9951221d0c35e58e7164ad9/week12/images/02_ap_wiring.png)

|순서|연결|
|---|---|
|1|ESP32 GND → 상단 − 레일|
|2|ESP32 3V3 → 상단 + 레일|
|3|DHT11 VCC → a6, DATA → a7, GND → a8|
|4|상단 + 레일 → e6, 상단 − 레일 → e8|
|5|GPIO27 → e7|
|6|GPIO26 → a16|
|7|330Ω 저항 → e16과 e20 사이|
|8|LED 긴 다리(A) → b20, 짧은 다리(K) → b21|
|9|a21 → 상단 − 레일|

같은 번호의 a~e는 내부로 연결됩니다. 그림과 센서 핀 순서가 다르면 핀 이름에 맞춰 연결합니다. 풀업이 없는 센서는 DATA와 3V3 사이에 4.7~10kΩ을 추가합니다. 배선 확인 후 USB를 연결합니다.

## 2. 센서 확인하기

1. `firmware/00_dht_check/00_dht_check.ino`를 엽니다.
2. **확인(✓) → 업로드(→)**를 누릅니다.
3. 시리얼 모니터를 열고 `115200`을 선택합니다.
4. 아래처럼 온습도가 3회 나오면 다음 단계로 갑니다. 숫자는 달라도 됩니다.

```text
T=24.0 C, H=52.0 %
```

### 센서 확인 코드

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

## 3. 주변 Wi-Fi 검색하기

1. `firmware/01_scan/01_scan.ino`를 엽니다.
2. 업로드하고 시리얼 모니터를 엽니다.
3. 보드의 **EN** 버튼을 누릅니다.
4. Wi-Fi 이름과 신호 세기가 나오면 성공입니다.

### Wi-Fi 검색 코드

```cpp
#include <WiFi.h>
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    Serial.printf("%d: %s (%d dBm)\n", i + 1,
                  WiFi.SSID(i).c_str(), WiFi.RSSI(i));
  }
  WiFi.scanDelete();
}
void loop() { delay(10); }
```

## 4. 모바일 웹서버 올리기

1. `firmware/02_mobile_ap/02_mobile_ap.ino`를 엽니다.
2. 코드의 `ESP32-Team01`을 팀 이름으로 바꿉니다. 예: `ESP32-Team02`.
3. 코드를 업로드합니다.
4. 시리얼 모니터에서 `AP IP: 192.168.4.1`을 확인합니다.

### 모바일 웹서버 코드

```cpp
#include <WiFi.h>
#include <DHT.h>
#include <math.h>
const char* DEVICE_ID = "team01";
DHT dht(27, DHT11);
const int LED_PIN = 26;
float temperature = NAN, humidity = NAN;
unsigned long lastRead = 0;
void sampleSensor() {
  if (millis() - lastRead < 2500) return;
  lastRead = millis();
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}
String sensorJson() {
  if (isnan(temperature) || isnan(humidity)) return "{\"valid\":false}";
  return String("{\"valid\":true,\"device\":\"") + DEVICE_ID +
    "\",\"temperature\":" +
    String(temperature, 1) + ",\"humidity\":" + String(humidity, 1) + "}";
}
#include <WebServer.h>
WebServer server(80);
const char PAGE[] PROGMEM = R"HTML(
<!doctype html><html lang="ko"><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 센서</title><h1>온습도 모니터</h1>
<pre id="out">센서 준비 중</pre>
<button onclick="led(1)">LED 켜기</button>
<button onclick="led(0)">LED 끄기</button>
<script>
async function led(v){await fetch('/led?state='+v,{method:'POST'});}
async function poll(){try{let d=await(await fetch('/data')).json();
document.getElementById('out').textContent=d.valid?
`온도 ${d.temperature} °C / 습도 ${d.humidity} %`:'센서 읽기 실패 또는 준비 중';
}catch(e){document.getElementById('out').textContent='연결 확인';}}
setInterval(poll,2500);poll();</script></html>
)HTML";
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  dht.begin();
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP("ESP32-Team01", "esp32wifi")) {
    Serial.println("AP start failed");
  }
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", HTTP_GET, [](){server.send(200,"text/html; charset=utf-8",PAGE);});
  server.on("/data", HTTP_GET, [](){server.send(200,"application/json",sensorJson());});
  server.on("/led", HTTP_POST, [](){
    String v = server.arg("state");
    if (v != "0" && v != "1") {server.send(400,"text/plain","bad state");return;}
    digitalWrite(LED_PIN, v == "1" ? HIGH : LOW);
    server.send(200,"text/plain","OK");
  });
  server.begin();
}
void loop() {
  server.handleClient();
  sampleSensor();
}
```

## 5. 휴대폰으로 접속하기

1. 휴대폰 Wi-Fi에서 앞에서 정한 `ESP32-Team01`을 선택합니다.
2. 암호 `esp32wifi`를 입력합니다.
3. **인터넷 연결 없음**이 나와도 연결을 유지합니다.
4. 브라우저 주소창에 `http://192.168.4.1`을 입력합니다.
5. 약 3초 기다려 온습도를 확인합니다.
6. **LED 켜기 → LED 끄기**를 눌러 실제 LED가 바뀌는지 확인합니다.

## 안 될 때

|증상|할 일|
|---|---|
|센서에 FAIL 표시|VCC·GND·GPIO27, DHT11 종류와 풀업 확인|
|휴대폰에서 페이지가 안 열림|ESP32 Wi-Fi 연결 유지, 주소의 http 확인|
|LED가 안 켜짐|GPIO26·330Ω·LED 긴 다리/짧은 다리 확인|

## 완료 확인

- [ ] 센서값 3회 출력
- [ ] 휴대폰에 온습도 표시
- [ ] LED 켜기·끄기 확인

이제 워크북 2로 이동합니다.
