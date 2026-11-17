# 워크북 1 · 공유기 없이 모바일 온습도 모니터·LED 제어

예상 65분. 완료 목표: ESP32 AP에 모바일을 연결하고 센서값 3회 확인, LED 켜기·끄기 성공.

![연결도](../images/02_ap_wiring.png)

## 공통 연결 순서

1. USB를 뽑는다. ESP32 실크의 3V3, GND, 27, 26을 찾는다.
2. DHT11 VCC를 3V3, GND를 GND, DATA를 GPIO27에 연결한다. 모듈 핀 라벨 기준으로 연결한다.
3. GPIO26 → 330Ω → LED 긴 다리 A → LED 짧은 다리 K → GND 순서로 연결한다.
4. 풀업이 없는 DHT11에는 DATA와 3V3 사이 4.7~10kΩ을 추가한다.
5. 전원·GND 단락이 없는지 확인한 뒤 USB만 연결한다.

## 실습 결과 기록

|항목|학생 기록|
|---|---|
|학번/이름/팀| |
|보드·ESP32 패키지·라이브러리 버전| |
|사용 핀·모듈 핀 순서| |
|접속 주소·시리얼 로그| |
|측정값 3회(온도/습도/시각)| |
|LED 켜기·끄기 결과 및 지연| |
|발생 오류·원인·해결| |

배선 사진, 모바일 화면, 시리얼/서버 로그를 첨부한다. Wi-Fi 비밀번호가 보이는 화면은 제외한다.

## A. 예제 1 · 주변 Wi-Fi 검색

![USB 연결](../images/01_scan.png)

1. 외부 배선 없이 USB만 연결해도 실행 가능하다.
2. `firmware/01_scan/01_scan.ino`를 연다. 보드 ESP32 Dev Module, 해당 COM 포트를 선택한다.
3. 확인(체크) → 업로드 → 시리얼 모니터 115200 baud.
4. 리셋 EN을 눌러 SSID와 RSSI 목록을 확인한다. 가장 강한 신호와 가장 약한 신호를 기록한다.
5. 아무것도 검색되지 않으면 주변에 2.4GHz AP가 있는지 확인한다.

## B. 예제 2 · ESP32 자체 웹서버

1. 위 공통 배선을 완료한다.
2. `firmware/02_mobile_ap/02_mobile_ap.ino`를 연다.
3. `ESP32-Team01`을 팀 고유 이름으로 바꾼다. 암호 `esp32wifi`는 8자 이상이다.
4. 업로드 후 시리얼에서 AP IP를 확인한다. 기본 설정에서는 192.168.4.1이다.
5. 모바일 Wi-Fi 설정 → 해당 SSID → 암호 입력. 인터넷 연결 없음 안내가 나와도 이 Wi-Fi 연결을 유지한다.
6. 모바일 브라우저에 `http://192.168.4.1`을 직접 입력한다. HTTPS가 아니다.
7. 최초 약 2.5초 뒤 값이 표시된다. 센서에 손을 가까이해 3회 변화를 기록한다. 물을 직접 묻히지 않는다.
8. LED 켜기/끄기를 각각 눌러 실물 점등을 확인한다.
9. `/data` 주소를 열어 JSON과 화면 숫자가 같은지 확인한다.

## 코드 읽기 포인트

`server.on`은 URL과 동작을 연결한다. `/`는 화면, `/data`는 센서 JSON, `/led`는 POST 상태 변경이다. `loop()`는 HTTP 요청을 처리하고 2.5초마다 센서를 읽는다. 페이지의 JavaScript는 같은 장치에 GET을 반복하므로 별도 앱 설치가 필요 없다.

## 고장 찾기와 과제

- SSID가 안 보임: 전원·업로드·EN 재시작 확인. 스캔 스케치가 아니라 AP 스케치인지 확인.
- 페이지 안 열림: 모바일이 해당 ESP32 Wi-Fi를 유지하는지 확인. 자동으로 모바일 데이터로 전환되면 잠시 모바일 데이터를 끄고 재시험.
- 값 없음/읽기 실패: DATA 핀·전원·GND·DHT11 종류·풀업 확인. 충분히 기다린 뒤 재시험.
- LED 반응 없음: 26번 핀·극성·저항·GND 확인. 보드 내장 LED와 외부 LED를 구별한다.
- 도전 과제: 화면에 팀명과 온도 28°C 이상 경고 문구를 추가한다. 코드에서 바꾼 부분을 설명한다.

## 통과 체크

- [ ] SSID/RSSI 기록
- [ ] 모듈 핀 라벨을 확인한 배선 사진
- [ ] 모바일에 온습도 3회 표시
- [ ] LED 켜기/끄기 실제 확인
- [ ] AP와 서버 역할을 설명

## 전체 코드 · firmware/01_scan/01_scan.ino

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

## 전체 코드 · firmware/02_mobile_ap/02_mobile_ap.ino

```cpp
#include <WiFi.h>
#include <DHT.h>
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
  return "{\"valid\":true,\"device\":\"team01\",\"temperature\":" +
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
  WiFi.softAP("ESP32-Team01", "esp32wifi");
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
