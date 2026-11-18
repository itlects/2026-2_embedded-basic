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

