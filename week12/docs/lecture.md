# 12주차 · Wi-Fi 온습도 모니터 만들기

오늘은 휴대폰으로 DHT11 온습도를 보고 LED를 켜고 끕니다. 이어서 PC로 측정값을 전송합니다.

## 1. 준비물

Windows 11 Home PC, ESP32 DevKitC V4, 데이터 USB 케이블, 3.3V 지원 DHT11 모듈, LED 1개, 330Ω 저항 1개, 브레드보드·점퍼선, 휴대폰.

## 2. 실습 파일 받기

[전체 실습 ZIP 다운로드](https://drive.google.com/file/d/1WpO4QQ1IhXnyxXSASvCc1Mx113N7q-kV/view)

1. ZIP을 다운로드합니다.
2. 파일을 우클릭 → **모두 압축 풀기**를 누릅니다.
3. 압축을 푼 폴더 안의 `week12`를 엽니다.

## 3. Arduino IDE 설정

1. Arduino IDE 2.x를 실행합니다.
2. **파일 → 기본 설정 → 추가 보드 관리자 URL**에 다음 주소를 넣습니다.

```text
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

3. **보드 관리자**에서 `esp32 by Espressif Systems` **3.3.2**를 설치합니다.
4. **라이브러리 관리자**에서 다음 두 라이브러리를 설치합니다.

|라이브러리|버전|
|---|---|
|DHT sensor library by Adafruit|1.4.6|
|Adafruit Unified Sensor|1.1.15|

5. ESP32를 USB로 연결합니다.
6. **도구 → 보드 → ESP32 Dev Module**을 선택합니다.
7. **도구 → 포트**에서 ESP32의 COM 포트를 선택합니다.
8. **시리얼 모니터 속도**는 `115200`으로 맞춥니다.

## 첫 예제 · Wi-Fi Hello World

ESP32에 휴대폰을 연결해 브라우저에 **Hello, World!**를 표시합니다. 준비물은 ESP32 DevKitC V4, 데이터 USB 케이블, 휴대폰입니다.

### 1. USB 연결하기

ESP32를 PC에 USB로 연결합니다. 센서·LED 배선은 필요 없습니다.

### 2. 코드 업로드하기

1. Arduino IDE에서 새 스케치를 만듭니다.
2. 아래 전체 코드를 붙여넣습니다.
3. `ESP32-Hello-Team01`을 팀 이름으로 바꿉니다. 예: `ESP32-Hello-Team02`.
4. **ESP32 Dev Module → 해당 COM 포트**를 선택합니다.
5. 확인(✓) → 업로드(→)를 누릅니다.

```cpp
#include <WiFi.h>
#include <WebServer.h>

const char* WIFI_NAME = "ESP32-Hello-Team01";
const char* WIFI_PASSWORD = "esp32wifi";

WebServer server(80);

void setup() {
  Serial.begin(115200);

  // ESP32가 휴대폰이 접속할 Wi-Fi를 만듭니다.
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(WIFI_NAME, WIFI_PASSWORD)) {
    Serial.println("Wi-Fi start failed");
    return;
  }

  Serial.print("Wi-Fi name: ");
  Serial.println(WIFI_NAME);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // 브라우저에서 첫 화면을 요청하면 HTML을 보냅니다.
  server.on("/", HTTP_GET, []() {
    server.send(
      200,
      "text/html; charset=utf-8",
      "<!doctype html><html lang='ko'>"
      "<meta charset='utf-8'>"
      "<meta name='viewport' content='width=device-width,initial-scale=1'>"
      "<title>ESP32 Hello</title>"
      "<h1>Hello, World!</h1>"
      "<p>ESP32 Wi-Fi 연결 성공!</p>"
      "</html>"
    );
  });

  server.begin();
  Serial.println("Open http://192.168.4.1");
}

void loop() {
  // 브라우저 요청을 계속 처리합니다.
  server.handleClient();
}
```

### 3. 시리얼 출력 확인하기

시리얼 모니터를 `115200`으로 열고 ESP32의 **EN**을 누릅니다.

```text
Wi-Fi name: ESP32-Hello-Team01
IP address: 192.168.4.1
Open http://192.168.4.1
```

Wi-Fi 이름은 자신이 바꾼 팀 이름으로 나옵니다.

### 4. 휴대폰에서 열기

1. 휴대폰 Wi-Fi에서 자신의 `ESP32-Hello-Team01`을 선택합니다.
2. 암호 `esp32wifi`를 입력합니다.
3. 인터넷 연결 없음 안내가 나오면 **연결 유지**를 선택합니다.
4. 브라우저 주소창에 `http://192.168.4.1`을 입력합니다.
5. 아래 문구가 표시되면 성공입니다.

```text
Hello, World!
ESP32 Wi-Fi 연결 성공!
```

Windows 11 Home PC에서도 같은 ESP32 Wi-Fi에 연결한 뒤 Edge 또는 Chrome으로 같은 주소를 열 수 있습니다.

### 코드 핵심

|코드|역할|
|---|---|
|WiFi.softAP()|ESP32에 접속할 Wi-Fi 생성|
|server.on("/", ...)|첫 화면 요청 처리|
|server.send()|브라우저에 HTML 전송|
|server.handleClient()|접속 요청을 반복 처리|

### 안 될 때

|증상|조치|
|---|---|
|Wi-Fi 이름이 안 보임|업로드·USB 전원 확인 후 EN|
|휴대폰이 다른 Wi-Fi로 바뀜|ESP32 Wi-Fi 연결 유지|
|페이지가 안 열림|https 대신 http 사용, 모바일 데이터 잠시 끄고 재접속|

## 두 번째 예제 · Wi-Fi LED 2개 제어

휴대폰에서 LED 1·2를 각각 켜고 끕니다. DevKitC V4의 전원 LED 대신 **외부 LED 2개**를 연결합니다.

### 1. 회로 연결하기

1. USB를 뽑고 앞 실습의 센서·버튼 배선을 분리합니다.
2. LED 2개와 330Ω 저항 2개를 아래처럼 연결합니다.

![LED 2개 실제 연결](../images/00_wifi_two_led.png)

|부품|연결|
|---|---|
|LED 1|GPIO25 → 330Ω → LED 긴 다리(A), 짧은 다리(K) → GND|
|LED 2|GPIO26 → 330Ω → LED 긴 다리(A), 짧은 다리(K) → GND|

그림의 좌표: LED A/K는 b10/b11과 b18/b19, 저항은 e10-f10과 e18-f18, GPIO25/26은 j10/j18에 연결합니다. d11·d19를 상단 − 레일에 연결하고 해당 레일을 ESP32 GND에 연결합니다.

3. 배선을 확인하고 USB를 꽂습니다.

### 2. 코드 업로드하기

1. 새 스케치를 열고 아래 전체 코드를 붙여넣습니다. 다운로드한 파일은 `firmware/00_wifi_two_led/00_wifi_two_led.ino`입니다.
2. `ESP32-LED-Team01`을 팀 이름으로 바꿉니다.
3. **ESP32 Dev Module → 해당 COM 포트**를 선택합니다.
4. 확인(✓) → 업로드(→)를 누릅니다.
5. 시리얼 모니터를 `115200`으로 열고 EN을 누릅니다. `IP address: 192.168.4.1`을 확인합니다.

```cpp
#include <WiFi.h>
#include <WebServer.h>

const char* WIFI_NAME = "ESP32-LED-Team01";
const char* WIFI_PASSWORD = "esp32wifi";
const uint8_t LED_PINS[2] = {25, 26};
bool ledOn[2] = {false, false};
WebServer server(80);

const char PAGE[] PROGMEM = R"HTML(
<!doctype html><html lang="ko">
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>LED 2개 제어</title>
<style>
body{font:20px sans-serif;background:#eef4fa;padding:20px;max-width:600px;margin:auto}
section{background:white;padding:20px;margin:15px 0;border-radius:12px}
button{font:inherit;padding:15px;margin:5px}
</style>
<h1>ESP32 LED 제어</h1><p id="status">연결 중</p>
<section><h2>LED 1 · GPIO25</h2><p id="led1">확인 중</p>
<button onclick="setLed(1,1)">켜기</button>
<button onclick="setLed(1,0)">끄기</button></section>
<section><h2>LED 2 · GPIO26</h2><p id="led2">확인 중</p>
<button onclick="setLed(2,1)">켜기</button>
<button onclick="setLed(2,0)">끄기</button></section>
<script>
let busy = false;
async function refresh(){
  if(busy) return;
  busy = true;
  try{
    const r = await fetch('/state',{cache:'no-store'});
    if(!r.ok) throw new Error('state');
    const d = await r.json();
    document.getElementById('led1').textContent = d.led1 ? '켜짐' : '꺼짐';
    document.getElementById('led2').textContent = d.led2 ? '켜짐' : '꺼짐';
    document.getElementById('status').textContent = 'ESP32 연결됨';
  }catch(e){
    document.getElementById('status').textContent = 'Wi-Fi 연결 확인';
  }finally{busy = false;}
}
async function setLed(id,state){
  try{
    const r = await fetch('/led?id='+id+'&state='+state,{method:'POST'});
    if(!r.ok) throw new Error('command');
    await refresh();
  }catch(e){document.getElementById('status').textContent = '명령 전송 실패';}
}
setInterval(refresh,1000);
refresh();
</script></html>
)HTML";

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 2; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(WIFI_NAME, WIFI_PASSWORD)) {
    Serial.println("Wi-Fi start failed");
    return;
  }
  Serial.print("Wi-Fi name: ");
  Serial.println(WIFI_NAME);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html; charset=utf-8", PAGE);
  });
  server.on("/state", HTTP_GET, []() {
    String json = "{\"led1\":";
    json += ledOn[0] ? "true" : "false";
    json += ",\"led2\":";
    json += ledOn[1] ? "true" : "false";
    json += "}";
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", json);
  });
  server.on("/led", HTTP_POST, []() {
    String id = server.arg("id");
    String state = server.arg("state");
    if ((id != "1" && id != "2") ||
        (state != "0" && state != "1")) {
      server.send(400, "text/plain", "Invalid id or state");
      return;
    }
    int index = id.toInt() - 1;
    ledOn[index] = (state == "1");
    digitalWrite(LED_PINS[index], ledOn[index] ? HIGH : LOW);
    server.send(200, "text/plain", "OK");
  });
  server.begin();
}

void loop() {
  server.handleClient();
}
```

### 3. 휴대폰으로 접속하기

1. 휴대폰 Wi-Fi를 자신의 `ESP32-LED-Team01`에 연결합니다. Hello 예제의 Wi-Fi 이름과 다릅니다.
2. 암호 `esp32wifi`를 입력합니다. 인터넷 연결 없음이 나와도 연결을 유지합니다.
3. 브라우저에서 `http://192.168.4.1`을 엽니다.
4. **LED 1 켜기 → 끄기**를 눌러 첫 번째 LED를 확인합니다.
5. **LED 2 켜기 → 끄기**를 눌러 두 번째 LED를 확인합니다.
6. 두 LED를 모두 켠 뒤 하나씩 끕니다. 화면의 켜짐/꺼짐 표시와 실물이 같은지 확인합니다.

### 안 될 때

|증상|조치|
|---|---|
|페이지가 Hello World로 보임|LED 예제 업로드 확인, 새 Wi-Fi 이름에 접속 후 새로고침|
|화면은 바뀌는데 LED가 안 켜짐|GPIO25/26·LED 극성·330Ω·GND 확인|
|페이지가 안 열림|ESP32-LED Wi-Fi 연결과 http 주소 확인|

### 완료 확인

- [ ] LED 1만 켜기
- [ ] LED 2만 켜기
- [ ] 두 LED 함께 켜기·모두 끄기
- [ ] 화면 표시와 실물 상태 일치

## 4. 실습 순서

1. **워크북 1:** 배선 → 센서 확인 → Wi-Fi 검색 → 휴대폰 접속·LED 제어
2. **워크북 2:** PC 서버 실행 → 센서값 전송 → 휴대폰 관제
3. 시간이 남으면 워크북 2의 **선택 실습: MySQL 저장**을 진행합니다.

## 실습 전에 알아둘 것

|항목|이번 실습에서의 의미|
|---|---|
|AP|휴대폰이 ESP32에 직접 접속하는 방식|
|STA|ESP32가 공유기에 접속하는 방식|
|GET / POST|값 조회 / 센서값·제어 명령 전송|
|millis()|일정 간격으로 센서를 읽을 때 사용|
|isnan()|센서 읽기 실패 확인|
|GPIO27 / GPIO26|DHT11 데이터 / LED 출력|

## 제출

배선 사진 1장, 모바일 화면 1장, `POST: 201` 시리얼 화면 1장을 제출합니다.

## 추가 응용 실습

워크북 3: DHT11·LED 4개·버튼 4개·수동 피에조 부저를 연결해 웹 제어와 멜로디 2곡을 실습합니다.
