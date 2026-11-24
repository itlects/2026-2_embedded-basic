// 예제 4 : Wi-Fi 웹페이지 - DHT22 + LED 4개 + 버튼 4개 + CSV 저장
// ESP32가 웹서버가 되고, 측정값을 ESP32 내부 저장소(LittleFS)의 data.csv에 저장합니다.
// 라이브러리 : DHT sensor library (Adafruit) + Adafruit Unified Sensor
//             (WiFi, WebServer, LittleFS는 ESP32 보드 패키지에 포함)

#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <DHT.h>
#include <time.h>

// ---------- 1. 설정 (본인 환경에 맞게 수정) ----------
const char* WIFI_SSID = "YOUR_WIFI";          // 공유기 이름 (Wokwi: Wokwi-GUEST)
const char* WIFI_PASS = "YOUR_PASSWORD";      // 공유기 암호 (Wokwi: 빈칸 "")
const char* AP_SSID   = "SmartFarm-team01";   // 공유기 연결 실패 시 ESP32가 만드는 Wi-Fi
const char* AP_PASS   = "12345678";           // 8자 이상

const int DHT_PIN = 19;
const int LED_PINS[4] = {14, 13, 4, 18};
const int BTN_PINS[4] = {32, 33, 25, 26};

const char* CSV_PATH = "/data.csv";
const unsigned long LOG_MS = 10000;           // 10초마다 CSV 1행 기록
const size_t CSV_MAX = 100 * 1024;            // 100KB가 넘으면 새 파일로 시작

// ---------- 2. 전역 변수 ----------
WebServer server(80);
DHT dht(DHT_PIN, DHT22);

bool ledOn[4]      = {false, false, false, false};
int  pressCount[4] = {0, 0, 0, 0};
int  lastBtn[4]    = {HIGH, HIGH, HIGH, HIGH};
unsigned long lastBtnMs[4] = {0, 0, 0, 0};

float tempC = NAN, humi = NAN;
unsigned long lastRead = 0, lastLog = 0;

// ---------- 3. 웹페이지(HTML) ----------
const char PAGE[] PROGMEM = R"HTML(
<!doctype html><html lang="ko"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 센서 웹</title>
<style>
body{font-family:sans-serif;background:#eef3f7;margin:0;padding:14px;max-width:640px;margin:auto}
.card{background:#fff;border-radius:12px;padding:14px;margin:10px 0}
.big{font-size:34px;font-weight:bold}
.row{display:flex;gap:8px}
.row>div{flex:1;text-align:center}
button{font-size:16px;padding:12px 0;width:100%;border:0;border-radius:8px;background:#ccc}
button.on{background:#f5b301}
.dot{display:inline-block;width:18px;height:18px;border-radius:50%;background:#bbb}
.dot.on{background:#2a9d4b}
table{width:100%;border-collapse:collapse;font-size:13px}
td,th{border-bottom:1px solid #ddd;padding:4px;text-align:center}
</style></head><body>
<h2>ESP32 센서 웹페이지</h2>
<div id="st">연결 중...</div>

<div class="card row">
  <div>온도<div class="big" id="t">-</div>°C</div>
  <div>습도<div class="big" id="h">-</div>%</div>
</div>

<div class="card"><b>LED 제어</b> (누르면 켜짐/꺼짐 전환)
  <div class="row" style="margin-top:8px">
    <div><button id="L1" onclick="led(1)">LED1</button></div>
    <div><button id="L2" onclick="led(2)">LED2</button></div>
    <div><button id="L3" onclick="led(3)">LED3</button></div>
    <div><button id="L4" onclick="led(4)">LED4</button></div>
  </div>
</div>

<div class="card"><b>버튼 상태</b> (초록=누르는 중, 숫자=누른 횟수)
  <div class="row" style="margin-top:8px">
    <div><span class="dot" id="B1"></span><br>BTN1 <span id="C1">0</span></div>
    <div><span class="dot" id="B2"></span><br>BTN2 <span id="C2">0</span></div>
    <div><span class="dot" id="B3"></span><br>BTN3 <span id="C3">0</span></div>
    <div><span class="dot" id="B4"></span><br>BTN4 <span id="C4">0</span></div>
  </div>
</div>

<div class="card"><b>CSV 기록 (최근 10행)</b>
  <a href="/data.csv" download="data.csv">[CSV 다운로드]</a>
  <table id="tb"></table>
</div>

<script>
async function refresh(){
  try{
    const d = await (await fetch('/api/state')).json();
    document.getElementById('t').textContent = d.t===null ? 'ERR' : d.t.toFixed(1);
    document.getElementById('h').textContent = d.h===null ? 'ERR' : d.h.toFixed(1);
    for(let i=0;i<4;i++){
      document.getElementById('L'+(i+1)).className = d.led[i] ? 'on' : '';
      document.getElementById('B'+(i+1)).className = d.btn[i] ? 'dot on' : 'dot';
      document.getElementById('C'+(i+1)).textContent = d.cnt[i];
    }
    document.getElementById('st').textContent = '연결됨 · ' + d.time;
  }catch(e){ document.getElementById('st').textContent = '연결 끊김'; }
}
async function led(n){
  await fetch('/api/led?n='+n, {method:'POST'});
  refresh();
}
async function loadCsv(){
  try{
    const txt = await (await fetch('/data.csv')).text();
    const lines = txt.trim().split('\n');
    const head = lines[0];
    const rows = lines.slice(1).slice(-10).reverse();
    let html = '<tr><th>' + head.split(',').join('</th><th>') + '</th></tr>';
    for(const r of rows) html += '<tr><td>' + r.split(',').join('</td><td>') + '</td></tr>';
    document.getElementById('tb').innerHTML = html;
  }catch(e){}
}
setInterval(refresh, 1000);
setInterval(loadCsv, 5000);
refresh(); loadCsv();
</script>
</body></html>
)HTML";

// ---------- 4. 시간 문자열 ----------
void timeString(char* buf, size_t len) {
  struct tm tmv;
  if (getLocalTime(&tmv, 10)) {                    // 인터넷 시간(NTP) 동기화된 경우
    strftime(buf, len, "%Y-%m-%d %H:%M:%S", &tmv);
  } else {                                         // AP 모드 등 시간 정보가 없을 때
    snprintf(buf, len, "boot+%lus", millis() / 1000);
  }
}

// ---------- 5. CSV 파일 ----------
void newCsv() {
  File f = LittleFS.open(CSV_PATH, FILE_WRITE);    // 덮어쓰기
  f.println("time,temp_c,humi_pct,led1,led2,led3,led4,btn1,btn2,btn3,btn4,event");
  f.close();
}

void logRow(const char* event) {
  File f = LittleFS.open(CSV_PATH, FILE_APPEND);   // 파일 끝에 추가
  if (!f) return;
  if (f.size() > CSV_MAX) {                        // 너무 커지면 새로 시작
    f.close();
    newCsv();
    f = LittleFS.open(CSV_PATH, FILE_APPEND);
  }
  char ts[24];
  timeString(ts, sizeof(ts));
  f.printf("%s,%.1f,%.1f,%d,%d,%d,%d,%d,%d,%d,%d,%s\n",
           ts, tempC, humi,
           ledOn[0], ledOn[1], ledOn[2], ledOn[3],
           pressCount[0], pressCount[1], pressCount[2], pressCount[3],
           event);
  f.close();
}

// ---------- 6. LED ----------
void setLed(int i, bool on) {
  ledOn[i] = on;
  digitalWrite(LED_PINS[i], on ? HIGH : LOW);
}

// ---------- 7. 웹 요청 처리 함수 ----------
void handleRoot() {
  server.send(200, "text/html; charset=utf-8", PAGE);
}

void handleState() {
  char ts[24];
  timeString(ts, sizeof(ts));
  String j = "{";
  j += "\"t\":" + (isnan(tempC) ? String("null") : String(tempC, 1));
  j += ",\"h\":" + (isnan(humi) ? String("null") : String(humi, 1));
  j += ",\"led\":[";
  for (int i = 0; i < 4; i++) j += String(ledOn[i]) + (i < 3 ? "," : "]");
  j += ",\"btn\":[";
  for (int i = 0; i < 4; i++) j += String(digitalRead(BTN_PINS[i]) == LOW) + (i < 3 ? "," : "]");
  j += ",\"cnt\":[";
  for (int i = 0; i < 4; i++) j += String(pressCount[i]) + (i < 3 ? "," : "]");
  j += ",\"time\":\"" + String(ts) + "\"}";
  server.send(200, "application/json", j);
}

void handleLed() {
  int n = server.arg("n").toInt();                 // /api/led?n=1 -> n=1
  if (n < 1 || n > 4) {
    server.send(400, "text/plain", "n must be 1~4");
    return;
  }
  setLed(n - 1, !ledOn[n - 1]);                    // 켜짐 <-> 꺼짐
  char ev[16];
  snprintf(ev, sizeof(ev), "web_LED%d", n);
  logRow(ev);                                      // 웹 조작도 CSV에 기록
  server.send(200, "text/plain", "OK");
}

void handleCsv() {
  File f = LittleFS.open(CSV_PATH, FILE_READ);
  if (!f) {
    server.send(404, "text/plain", "no csv");
    return;
  }
  server.streamFile(f, "text/csv");                // 파일 내용을 그대로 전송
  f.close();
}

// ---------- 8. Wi-Fi 연결 ----------
void startWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi connecting");
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("STA mode. Open http://");
    Serial.println(WiFi.localIP());
    configTime(9 * 3600, 0, "pool.ntp.org", "time.google.com");   // 한국시간(UTC+9)
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    Serial.printf("AP mode. Join Wi-Fi '%s' (pw %s), open http://", AP_SSID, AP_PASS);
    Serial.println(WiFi.softAPIP());                               // 192.168.4.1
  }
}

// ---------- 9. setup / loop ----------
void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 4; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    setLed(i, false);
    pinMode(BTN_PINS[i], INPUT_PULLUP);            // 누르면 LOW
  }
  dht.begin();

  if (!LittleFS.begin(true)) {                     // true : 처음이면 포맷 후 마운트
    Serial.println("LittleFS mount failed");
  }
  if (!LittleFS.exists(CSV_PATH)) newCsv();

  startWiFi();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/state", HTTP_GET, handleState);
  server.on("/api/led", HTTP_POST, handleLed);
  server.on("/data.csv", HTTP_GET, handleCsv);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();                           // 웹 요청 처리 (자주 호출해야 함)

  // 버튼 : 누르는 순간 해당 LED 전환 + 횟수 증가
  for (int i = 0; i < 4; i++) {
    int b = digitalRead(BTN_PINS[i]);
    if (lastBtn[i] == HIGH && b == LOW && millis() - lastBtnMs[i] > 50) {
      lastBtnMs[i] = millis();
      pressCount[i]++;
      setLed(i, !ledOn[i]);
      char ev[16];
      snprintf(ev, sizeof(ev), "button%d", i + 1);
      logRow(ev);
    }
    lastBtn[i] = b;
  }

  // DHT22 : 2초마다 측정
  if (millis() - lastRead >= 2000) {
    lastRead = millis();
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
      tempC = t;
      humi = h;
    }
  }

  // CSV : 10초마다 기록
  if (millis() - lastLog >= LOG_MS) {
    lastLog = millis();
    logRow("periodic");
    Serial.printf("T=%.1f H=%.1f LED=%d%d%d%d\n", tempC, humi,
                  ledOn[0], ledOn[1], ledOn[2], ledOn[3]);
  }
}
