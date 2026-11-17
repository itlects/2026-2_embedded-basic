# 워크북 2 · PC로 센서값 전송·모바일 관제

워크북 1의 회로를 그대로 사용합니다. PC·ESP32·휴대폰을 같은 공유기에 연결합니다. ESP32는 **2.4GHz Wi-Fi**를 사용합니다.

[전체 실습 ZIP 다운로드](https://drive.google.com/file/d/1WpO4QQ1IhXnyxXSASvCc1Mx113N7q-kV/view)

![회로 연결](https://raw.githubusercontent.com/itlects/2026-2_embedded-basic/83096682d7244f83f9951221d0c35e58e7164ad9/week12/images/03_pc_wiring.png)

## 1. PowerShell 열기

1. 파일 탐색기에서 압축을 푼 `week12` 폴더를 엽니다.
2. 주소 표시줄에 `powershell`을 입력하고 Enter를 누릅니다.
3. 다음 명령으로 Python 3.13을 확인합니다.

```powershell
py -3.13 --version
```

`py`가 없다면 `python --version`을 입력합니다. 3.13이면 아래 명령의 `py -3.13`을 `python`으로 바꿔 실행합니다.

## 2. PC 서버 실행하기

```powershell
py -3.13 server/server.py
```

PC 브라우저에서 `http://127.0.0.1:8000`을 엽니다. **아직 수신값 없음** 또는 **수신 대기**가 표시되면 성공입니다. PowerShell 창은 닫지 않습니다.

## 3. 샘플 데이터로 확인하기

1. `week12` 폴더에서 PowerShell 창을 하나 더 엽니다.
2. 다음 명령을 실행합니다. 샘플 CSV는 ZIP에 들어 있습니다.

```powershell
py -3.13 server/replay.py
```

3. PC 화면에 온습도가 보이는지 확인합니다.
4. 첫 번째 서버 창에서 **Ctrl+C**를 누른 후 다시 실행합니다. 이제 실제 센서값을 받을 준비가 되었습니다.

```powershell
py -3.13 server/server.py
```

## 4. PC 주소 확인하기

두 번째 PowerShell 창에서 실행합니다.

```powershell
ipconfig
```

**무선 LAN 어댑터 Wi-Fi → IPv4 주소**를 적습니다. 예: `192.168.0.10`.

## 5. 모바일 접속 허용하기

1. Windows 11의 **설정 → 네트워크 및 인터넷 → Wi-Fi → 연결한 네트워크 속성**을 엽니다.
2. 신뢰하는 실습용 Wi-Fi의 네트워크 프로필을 **개인**으로 설정합니다.
3. **시작 버튼 우클릭 → 터미널(관리자)**을 엽니다. 관리자 암호가 필요하면 교수자에게 요청합니다.
4. PowerShell 탭에서 아래 명령을 한 번 실행합니다.

```powershell
New-NetFirewallRule -DisplayName "Embedded12 HTTP" -Direction Inbound -Action Allow -Protocol TCP -LocalPort 8000 -Profile Private -RemoteAddress LocalSubnet
```

5. 휴대폰을 ESP32-Team01에서 해제하고 **PC와 같은 공유기 Wi-Fi**에 연결합니다.
6. 휴대폰 브라우저에서 `http://PC의_IPv4:8000`을 엽니다. 예: `http://192.168.0.10:8000`.

## 6. ESP32 코드 설정하기

1. `firmware/03_pc_client/03_pc_client.ino`를 엽니다.
2. 다음 네 항목을 실제 값으로 바꿉니다.

|항목|입력할 값|
|---|---|
|DEVICE_ID|팀 ID, 예: team02|
|SSID|공유기의 2.4GHz Wi-Fi 이름|
|PASSWORD|해당 Wi-Fi 암호|
|BASE_URL|PC 주소, 예: http://192.168.0.10:8000|

3. **확인(✓) → 업로드(→)**를 누릅니다.
4. 시리얼 모니터를 `115200`으로 열고 다음 출력을 확인합니다.

```text
STA IP: 192.168.0.20
POST: 201
```

IP는 달라도 됩니다. `POST: 201`이 반복되면 전송 성공입니다.

### PC 전송 코드

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
#include <HTTPClient.h>
const char* SSID = "YOUR_2G_WIFI";
const char* PASSWORD = "YOUR_WIFI_PASSWORD";
const char* BASE_URL = "http://192.168.0.10:8000"; // PC IPv4로 수정
unsigned long lastSend = 0, lastReconnect = 0;
bool wasConnected = false;
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  dht.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  Serial.println("Connecting to Wi-Fi...");
}
void loop() {
  sampleSensor();
  if (WiFi.status() != WL_CONNECTED) {
    wasConnected = false;
    if (millis() - lastReconnect >= 10000) {
      lastReconnect = millis();
      Serial.println("Wi-Fi disconnected; retrying...");
      WiFi.reconnect();
    }
    return;
  }
  if (!wasConnected) {
    wasConnected = true;
    Serial.print("STA IP: ");
    Serial.println(WiFi.localIP());
  }
  if (millis() - lastSend < 3000) return;
  lastSend = millis();
  WiFiClient client;
  HTTPClient http;
  http.setConnectTimeout(1000);
  http.setTimeout(1000);
  if (!isnan(temperature) && !isnan(humidity)) {
    http.begin(client, String(BASE_URL) + "/api/readings");
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(sensorJson());
    Serial.printf("POST: %d\n", code);
    http.end();
  } else {
    Serial.println("DHT read failed: check power/DATA/pull-up");
  }
  http.begin(client, String(BASE_URL) + "/api/led?device=" + DEVICE_ID);
  int code = http.GET();
  if (code == 200) {
    String body = http.getString();
    body.trim();
    if (body == "0" || body == "1") digitalWrite(LED_PIN, body == "1" ? HIGH : LOW);
  }
  http.end();
}
```

## 7. 모바일에서 확인·제어하기

1. 모바일 화면의 **장치** 칸에 코드의 `DEVICE_ID`와 같은 값을 넣습니다.
2. 약 3초 기다려 온습도를 확인합니다.
3. 센서 가까이에 손을 대고 값이 변하는지 확인합니다.
4. **LED 켜기 → LED 끄기**를 누르고 약 3초 기다려 실물 LED를 확인합니다.
5. PC의 `server/received.csv`를 열어 자신의 팀 ID로 기록된 측정값 10개를 확인합니다. 앞의 샘플 기록과 구별합니다.

## 8. 선택 실습: MySQL에 저장하기

1. MySQL Workbench에서 `root` 계정으로 접속합니다. 실습 암호는 `1234`입니다.
2. **File → Open SQL Script**에서 `server/schema.sql`을 엽니다.
3. 전체 실행(번개 아이콘)을 누릅니다.
4. 서버 PowerShell에서 **Ctrl+C**를 누릅니다.
5. 같은 창에서 아래 명령을 실행합니다. 암호 입력 안내가 나오면 MySQL 암호를 입력합니다.

```powershell
py -3.13 -m pip install mysql-connector-python
$env:MYSQL_ENABLED="1"
$env:MYSQL_HOST="127.0.0.1"
$env:MYSQL_USER="root"
$env:MYSQL_PASSWORD=Read-Host "MySQL 암호"
py -3.13 server/server.py
```

6. ESP32에서 `POST: 201`이 나오면 Workbench에서 다음 쿼리를 실행합니다.

```sql
USE embedded12;
SELECT * FROM readings ORDER BY id DESC LIMIT 20;
```

자신의 팀 ID로 새 행이 생기면 성공입니다. 이 단계는 MySQL이 **수집 서버와 같은 PC에 설치된 경우**를 기준으로 합니다.

## 안 될 때

|증상|할 일|
|---|---|
|Wi-Fi 재시도 로그만 나옴|SSID·암호·2.4GHz 확인|
|POST가 음수|서버 실행·BASE_URL·개인 네트워크·방화벽 확인|
|POST: 503|MySQL 활성화 상태·암호·테이블 확인|
|휴대폰만 접속 안 됨|PC와 같은 Wi-Fi인지 확인, localhost 대신 PC IPv4 사용|
|다른 팀 값 표시|코드의 DEVICE_ID와 화면 장치 ID 일치 확인|

## 완료·종료

- [ ] POST: 201 반복 확인
- [ ] 모바일 온습도·LED 제어 확인
- [ ] 실측 CSV 10행 확인

서버 창에서 **Ctrl+C**를 눌러 종료합니다. USB 전원을 분리합니다.

## 서버 파일 보기

서버 파일은 ZIP에 포함되어 있습니다. 수정 없이 실행해도 됩니다.

- `server/server.py`: 센서 수신 서버
- `server/dashboard.html`: 모바일 화면
- `server/replay.py`: 샘플 전송
- `server/schema.sql`: DB 테이블 생성

[전체 소스 보기](https://github.com/itlects/2026-2_embedded-basic/tree/main/week12/server)
