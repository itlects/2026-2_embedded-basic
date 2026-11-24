// 워크북 : Wi-Fi 스마트팜 (예제 1~4 통합)
// 토양수분 + DHT22 + 릴레이/펌프 + LCD(I2C) + LED 4 + 버튼 4 + 수동부저 + 웹 + CSV
// 라이브러리 : DHT sensor library, Adafruit Unified Sensor, LiquidCrystal I2C (Frank de Brabander)

#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <time.h>

// ================= 1. 설정 (수정하는 부분) =================
const char* WIFI_SSID = "YOUR_WIFI";          // Wokwi: Wokwi-GUEST
const char* WIFI_PASS = "YOUR_PASSWORD";      // Wokwi: ""
const char* AP_SSID   = "SmartFarm-team01";
const char* AP_PASS   = "12345678";

int DRY_RAW = 3500;          // 예제1에서 측정한 마른 흙 ADC
int WET_RAW = 1500;          // 예제1에서 측정한 젖은 흙 ADC
int threshold = 40;          // 이 값(%)보다 건조하면 급수

const int RELAY_ON  = HIGH;  // 예제2에서 확인한 릴레이 트리거 방식
const unsigned long PUMP_MAX_MS  = 3000;    // 1회 급수 최대 3초
const unsigned long PUMP_REST_MS = 20000;   // 급수 후 20초 동안 재급수 금지

// ================= 2. 핀 =================
const int SOIL_AO = 34, SOIL_DO = 35, DHT_PIN = 19;
const int RELAY_PIN = 23, BUZZ_PIN = 27;
const int LED_PINS[4] = {4, 13, 14, 18};    // LED1 Wi-Fi, LED2 건조, LED3 펌프, LED4 AUTO
const int BTN_PINS[4] = {32, 33, 25, 26};   // BTN1 AUTO, BTN2 급수, BTN3 정지, BTN4 임계값
const int RELAY_OFF = (RELAY_ON == HIGH) ? LOW : HIGH;

const char* CSV_PATH = "/farm.csv";
const unsigned long LOG_MS = 10000;
const size_t CSV_MAX = 100 * 1024;

// ================= 3. 전역 변수 =================
WebServer server(80);
DHT dht(DHT_PIN, DHT22);
LiquidCrystal_I2C lcd(0x27, 16, 2);          // 주소 0x27 (안 되면 0x3F)

float tempC = NAN, humi = NAN;
int soilRaw = 0, moisture = 0, soilDo = HIGH;
bool autoMode = false, pumpOn = false, apMode = false;
unsigned long pumpStartMs = 0, pumpStopMs = 0;
unsigned long lastRead = 0, lastLog = 0, lastLcd = 0, lastAlarm = 0;
int lastBtn[4] = {HIGH, HIGH, HIGH, HIGH};
unsigned long lastBtnMs[4] = {0, 0, 0, 0};

// ================= 4. 웹페이지 =================
const char PAGE[] PROGMEM = R"HTML(
<!doctype html><html lang="ko"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Wi-Fi 스마트팜</title>
<style>
body{font-family:sans-serif;background:#eef5ee;margin:0;padding:14px;max-width:680px;margin:auto}
.card{background:#fff;border-radius:12px;padding:14px;margin:10px 0}
.grid{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;text-align:center}
.big{font-size:30px;font-weight:bold}
button{font-size:16px;padding:12px 0;width:100%;border:0;border-radius:8px;background:#d7dde3}
.on{background:#2a9d4b;color:#fff}
.warn{background:#e9a400;color:#fff}
table{width:100%;border-collapse:collapse;font-size:12px}
td,th{border-bottom:1px solid #ddd;padding:3px;text-align:center}
img{width:100%;border-radius:8px}
</style></head><body>
<h2>🌱 Wi-Fi 스마트팜</h2>
<div id="st">연결 중...</div>

<div class="card grid">
  <div>토양수분<div class="big" id="m">-</div>%</div>
  <div>온도<div class="big" id="t">-</div>°C</div>
  <div>습도<div class="big" id="h">-</div>%</div>
</div>

<div class="card">
  ADC <b id="raw">-</b> · 임계값 <b id="th">-</b>% · 펌프 <b id="p">-</b>
  <div class="grid" style="margin-top:8px">
    <button id="bA" onclick="cmd('auto')">AUTO</button>
    <button onclick="cmd('water')">급수 3초</button>
    <button onclick="cmd('stop')">정지</button>
  </div>
  <div style="margin-top:8px"><button onclick="cmd('th')">임계값 +10 (30~60)</button></div>
</div>

<div class="card"><b>CSV 기록 (최근 10행)</b>
  <a href="/data.csv" download="farm.csv">[CSV 다운로드]</a>
  <table id="tb"></table>
</div>

<div class="card"><b>(선택) ESP32-S3-CAM 화면</b><br>
  CAM IP <input id="cam" placeholder="192.168.0.50" size="14">
  <button style="width:auto;padding:6px 12px" onclick="showCam()">보기</button>
  <img id="camimg" alt="">
</div>

<script>
async function refresh(){
  try{
    const d = await (await fetch('/api/state')).json();
    document.getElementById('m').textContent = d.m;
    document.getElementById('t').textContent = d.t===null ? 'ERR' : d.t.toFixed(1);
    document.getElementById('h').textContent = d.h===null ? 'ERR' : d.h.toFixed(1);
    document.getElementById('raw').textContent = d.raw;
    document.getElementById('th').textContent = d.th;
    document.getElementById('p').textContent = d.pump ? 'ON' : 'OFF';
    document.getElementById('bA').className = d.auto ? 'on' : '';
    document.getElementById('m').style.color = d.m < d.th ? '#c46a00' : '#1b7a36';
    document.getElementById('st').textContent = '연결됨 · ' + d.time;
  }catch(e){ document.getElementById('st').textContent = '연결 끊김'; }
}
async function cmd(c){
  await fetch('/api/cmd?c=' + c, {method:'POST'});
  refresh();
}
async function loadCsv(){
  try{
    const lines = (await (await fetch('/data.csv')).text()).trim().split('\n');
    const rows = lines.slice(1).slice(-10).reverse();
    let html = '<tr><th>' + lines[0].split(',').join('</th><th>') + '</th></tr>';
    for(const r of rows) html += '<tr><td>' + r.split(',').join('</td><td>') + '</td></tr>';
    document.getElementById('tb').innerHTML = html;
  }catch(e){}
}
function showCam(){
  const ip = document.getElementById('cam').value.trim();
  localStorage.setItem('camip', ip);
  document.getElementById('camimg').src = 'http://' + ip + ':81/stream';
}
const saved = localStorage.getItem('camip');
if(saved){ document.getElementById('cam').value = saved; }
setInterval(refresh, 1000);
setInterval(loadCsv, 5000);
refresh(); loadCsv();
</script>
</body></html>
)HTML";

// ================= 5. 보조 함수 =================
void beep(int freq, int ms) {
  tone(BUZZ_PIN, freq, ms);                  // 수동 부저 : 주파수(Hz)로 소리
}

void timeString(char* buf, size_t len) {
  struct tm tmv;
  if (getLocalTime(&tmv, 10)) strftime(buf, len, "%Y-%m-%d %H:%M:%S", &tmv);
  else snprintf(buf, len, "boot+%lus", millis() / 1000);
}

void newCsv() {
  File f = LittleFS.open(CSV_PATH, FILE_WRITE);
  f.println("time,temp_c,humi_pct,soil_raw,moisture_pct,threshold,auto,pump,event");
  f.close();
}

void logRow(const char* event) {
  File f = LittleFS.open(CSV_PATH, FILE_APPEND);
  if (!f) return;
  if (f.size() > CSV_MAX) {
    f.close();
    newCsv();
    f = LittleFS.open(CSV_PATH, FILE_APPEND);
  }
  char ts[24];
  timeString(ts, sizeof(ts));
  f.printf("%s,%.1f,%.1f,%d,%d,%d,%d,%d,%s\n",
           ts, tempC, humi, soilRaw, moisture, threshold, autoMode, pumpOn, event);
  f.close();
}

// ================= 6. 펌프 (안전 규칙 포함) =================
bool pumpStart(const char* why) {
  if (pumpOn) return false;
  if (pumpStopMs != 0 && millis() - pumpStopMs < PUMP_REST_MS) {   // 휴지시간 확인
    Serial.println("Pump blocked: rest time");
    return false;
  }
  digitalWrite(RELAY_PIN, RELAY_ON);
  pumpOn = true;
  pumpStartMs = millis();
  beep(1000, 300);
  logRow(why);
  Serial.printf("PUMP ON (%s)\n", why);
  return true;
}

void pumpStop(const char* why) {
  digitalWrite(RELAY_PIN, RELAY_OFF);
  if (pumpOn) {
    pumpOn = false;
    pumpStopMs = millis();
    logRow(why);
    Serial.printf("PUMP OFF (%s)\n", why);
  }
}

// ================= 7. 명령 (버튼과 웹이 같은 함수 사용) =================
void doCommand(const String& c, bool fromWeb) {
  if (c == "auto") {
    autoMode = !autoMode;
    logRow(autoMode ? "auto_on" : "auto_off");
  } else if (c == "water") {
    autoMode = false;                          // 수동 급수 시 AUTO 해제
    pumpStart(fromWeb ? "water_web" : "water_btn");
  } else if (c == "stop") {
    autoMode = false;
    pumpStop("stop");
  } else if (c == "th") {
    threshold = (threshold >= 60) ? 30 : threshold + 10;
    logRow("threshold");
  }
}

// ================= 8. 센서 =================
void readSensors() {
  long sum = 0;
  for (int i = 0; i < 10; i++) sum += analogRead(SOIL_AO);
  soilRaw = sum / 10;
  moisture = constrain(map(soilRaw, DRY_RAW, WET_RAW, 0, 100), 0, 100);
  soilDo = digitalRead(SOIL_DO);

  float t = dht.readTemperature();
  float h = dht.readHumidity();
  tempC = t;                                   // 실패하면 NaN 그대로 저장 -> 경보
  humi = h;
}

// ================= 9. LCD =================
void updateLcd() {
  char line[17];
  lcd.setCursor(0, 0);
  if (isnan(tempC)) snprintf(line, sizeof(line), "M:%3d%% T:ERR    ", moisture);
  else snprintf(line, sizeof(line), "M:%3d%% T:%4.1fC  ", moisture, tempC);
  lcd.print(line);

  lcd.setCursor(0, 1);
  snprintf(line, sizeof(line), "H:%3.0f%% %-4s %-4s",
           isnan(humi) ? 0.0 : humi, autoMode ? "AUTO" : "MAN", pumpOn ? "PUMP" : "----");
  lcd.print(line);
}

// ================= 10. 웹 처리 =================
void handleState() {
  char ts[24];
  timeString(ts, sizeof(ts));
  String j = "{";
  j += "\"m\":" + String(moisture);
  j += ",\"raw\":" + String(soilRaw);
  j += ",\"t\":" + (isnan(tempC) ? String("null") : String(tempC, 1));
  j += ",\"h\":" + (isnan(humi) ? String("null") : String(humi, 1));
  j += ",\"th\":" + String(threshold);
  j += ",\"auto\":" + String(autoMode);
  j += ",\"pump\":" + String(pumpOn);
  j += ",\"time\":\"" + String(ts) + "\"}";
  server.send(200, "application/json", j);
}

void handleCmd() {
  doCommand(server.arg("c"), true);
  server.send(200, "text/plain", "OK");
}

void handleCsv() {
  File f = LittleFS.open(CSV_PATH, FILE_READ);
  if (!f) {
    server.send(404, "text/plain", "no csv");
    return;
  }
  server.streamFile(f, "text/csv");
  f.close();
}

void startWiFi() {
  lcd.clear();
  lcd.print("WiFi connecting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) delay(500);

  IPAddress ip;
  if (WiFi.status() == WL_CONNECTED) {
    ip = WiFi.localIP();
    configTime(9 * 3600, 0, "pool.ntp.org", "time.google.com");
  } else {
    apMode = true;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    ip = WiFi.softAPIP();
  }
  Serial.printf("%s mode. Open http://%s\n", apMode ? "AP" : "STA", ip.toString().c_str());
  lcd.clear();
  lcd.print(apMode ? "AP mode" : "WiFi OK");
  lcd.setCursor(0, 1);
  lcd.print(ip.toString());                    // 휴대폰으로 접속할 주소를 LCD에 표시
  delay(4000);
  lcd.clear();
}

// ================= 11. setup / loop =================
void setup() {
  Serial.begin(115200);
  digitalWrite(RELAY_PIN, RELAY_OFF);          // 부팅 직후 펌프 OFF 보장
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SOIL_DO, INPUT);
  analogReadResolution(12);
  for (int i = 0; i < 4; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    pinMode(BTN_PINS[i], INPUT_PULLUP);
  }
  dht.begin();
  lcd.init();
  lcd.backlight();

  if (!LittleFS.begin(true)) Serial.println("LittleFS mount failed");
  if (!LittleFS.exists(CSV_PATH)) newCsv();

  startWiFi();
  server.on("/", HTTP_GET, []() { server.send(200, "text/html; charset=utf-8", PAGE); });
  server.on("/api/state", HTTP_GET, handleState);
  server.on("/api/cmd", HTTP_POST, handleCmd);
  server.on("/data.csv", HTTP_GET, handleCsv);
  server.begin();
  readSensors();
  logRow("boot");
}

void loop() {
  server.handleClient();
  unsigned long now = millis();

  // (1) 버튼 4개
  const char* names[4] = {"auto", "water", "stop", "th"};
  for (int i = 0; i < 4; i++) {
    int b = digitalRead(BTN_PINS[i]);
    if (lastBtn[i] == HIGH && b == LOW && now - lastBtnMs[i] > 50) {
      lastBtnMs[i] = now;
      beep(2000, 40);
      doCommand(names[i], false);
    }
    lastBtn[i] = b;
  }

  // (2) 센서 : 2초마다
  if (now - lastRead >= 2000) {
    lastRead = now;
    readSensors();
  }

  // (3) 펌프 자동 정지 / 자동 급수
  if (pumpOn && now - pumpStartMs >= PUMP_MAX_MS) pumpStop("time_up");
  bool rested = (pumpStopMs == 0) || (now - pumpStopMs >= PUMP_REST_MS);
  if (autoMode && !pumpOn && rested && moisture < threshold) pumpStart("water_auto");

  // (4) LED 표시
  bool wifiLed = apMode ? ((now / 500) % 2) : (WiFi.status() == WL_CONNECTED);
  digitalWrite(LED_PINS[0], wifiLed);                // AP 모드면 깜빡임
  digitalWrite(LED_PINS[1], moisture < threshold);   // 건조
  digitalWrite(LED_PINS[2], pumpOn);                 // 펌프 작동
  digitalWrite(LED_PINS[3], autoMode);               // AUTO

  // (5) DHT22 오류 경보 : 5초마다 짧은 경고음
  if (isnan(tempC) && now - lastAlarm >= 5000) {
    lastAlarm = now;
    beep(3000, 200);
  }

  // (6) LCD 0.5초마다, CSV 10초마다
  if (now - lastLcd >= 500) {
    lastLcd = now;
    updateLcd();
  }
  if (now - lastLog >= LOG_MS) {
    lastLog = now;
    logRow("periodic");
  }
}
