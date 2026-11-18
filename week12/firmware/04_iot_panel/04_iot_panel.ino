#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <math.h>

const char* AP_SSID = "ESP32-IoT-Team01";
const char* AP_PASSWORD = "esp32wifi";
const uint8_t LED_PINS[4] = {16, 17, 18, 19};
const uint8_t BUTTON_PINS[4] = {32, 33, 25, 26};
const uint8_t BUZZER_PIN = 23;
const uint8_t DHT_PIN = 27;
const unsigned long DEBOUNCE_MS = 30;
const unsigned long LONG_PRESS_MS = 800;

WebServer server(80);
DHT dht(DHT_PIN, DHT11);
bool ledState[4] = {false, false, false, false};
bool rawButton[4] = {HIGH, HIGH, HIGH, HIGH};
bool stableButton[4] = {HIGH, HIGH, HIGH, HIGH};
bool longHandled[4] = {false, false, false, false};
unsigned long changedAt[4] = {0, 0, 0, 0};
unsigned long pressedAt[4] = {0, 0, 0, 0};
float temperature = NAN;
float humidity = NAN;
unsigned long lastSensorRead = 0;
unsigned long lastGoodRead = 0;
bool buzzerReady = false;

// Melody 1: Twinkle Twinkle Little Star (opening).
const uint16_t SONG1[] = {
  262, 262, 392, 392, 440, 440, 392,
  349, 349, 330, 330, 294, 294, 262
};
const uint8_t BEATS1[] = {1,1,1,1,1,1,2, 1,1,1,1,1,1,2};
// Melody 2: Ode to Joy (opening).
const uint16_t SONG2[] = {
  330, 330, 349, 392, 392, 349, 330, 294,
  262, 262, 294, 330, 330, 294, 294
};
const uint8_t BEATS2[] = {1,1,1,1,1,1,1,1, 1,1,1,1,1,1,2};
const uint16_t* currentSong = nullptr;
const uint8_t* currentBeats = nullptr;
size_t songLength = 0;
size_t noteIndex = 0;
uint8_t playingSong = 0;
unsigned long noteStarted = 0;
unsigned long noteDuration = 0;
bool noteMuted = false;

void stopMusic() {
  if (buzzerReady) {
    ledcWriteTone(BUZZER_PIN, 0);
  }
  playingSong = 0;
  currentSong = nullptr;
}

void startNote() {
  noteStarted = millis();
  noteDuration = 300UL * currentBeats[noteIndex];
  noteMuted = false;
  ledcWriteTone(BUZZER_PIN, currentSong[noteIndex]);
}

void startMusic(uint8_t song) {
  stopMusic();
  if (!buzzerReady) return;
  if (song == 1) {
    currentSong = SONG1;
    currentBeats = BEATS1;
    songLength = sizeof(SONG1) / sizeof(SONG1[0]);
  } else if (song == 2) {
    currentSong = SONG2;
    currentBeats = BEATS2;
    songLength = sizeof(SONG2) / sizeof(SONG2[0]);
  } else {
    return;
  }
  playingSong = song;
  noteIndex = 0;
  startNote();
}

void updateMusic() {
  if (playingSong == 0) return;
  unsigned long elapsed = millis() - noteStarted;
  if (!noteMuted && elapsed >= noteDuration * 85UL / 100UL) {
    ledcWriteTone(BUZZER_PIN, 0);
    noteMuted = true;
  }
  if (elapsed >= noteDuration) {
    noteIndex++;
    if (noteIndex >= songLength) {
      stopMusic();
    } else {
      startNote();
    }
  }
}

void setLed(uint8_t index, bool state) {
  ledState[index] = state;
  digitalWrite(LED_PINS[index], state ? HIGH : LOW);
}

void allOff() {
  stopMusic();
  for (uint8_t i = 0; i < 4; i++) {
    setLed(i, false);
  }
}

void updateButtons() {
  unsigned long now = millis();
  for (uint8_t i = 0; i < 4; i++) {
    bool reading = digitalRead(BUTTON_PINS[i]);
    if (reading != rawButton[i]) {
      rawButton[i] = reading;
      changedAt[i] = now;
    }
    if (now - changedAt[i] >= DEBOUNCE_MS &&
        reading != stableButton[i]) {
      stableButton[i] = reading;
      if (reading == LOW) {
        pressedAt[i] = now;
        longHandled[i] = false;
      } else if (!longHandled[i]) {
        setLed(i, !ledState[i]);
      }
    }
    if (stableButton[i] == LOW && !longHandled[i] &&
        now - pressedAt[i] >= LONG_PRESS_MS) {
      longHandled[i] = true;
      if (i == 0) startMusic(1);
      if (i == 1) startMusic(2);
      if (i == 2) stopMusic();
      if (i == 3) allOff();
    }
  }
}

void updateSensor() {
  if (millis() - lastSensorRead < 2500) return;
  lastSensorRead = millis();
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  if (!isnan(temperature) && !isnan(humidity)) {
    lastGoodRead = millis();
    Serial.printf("T=%.1f C, H=%.1f %%\n", temperature, humidity);
  } else {
    Serial.println("DHT FAIL: check GPIO27 / VCC / GND / pull-up");
  }
}

String stateJson() {
  bool valid = !isnan(temperature) && !isnan(humidity);
  String json = "{\"valid\":";
  json += valid ? "true" : "false";
  json += ",\"temperature\":";
  json += valid ? String(temperature, 1) : "null";
  json += ",\"humidity\":";
  json += valid ? String(humidity, 1) : "null";
  json += ",\"ageSeconds\":";
  json += valid ? String((millis() - lastGoodRead) / 1000UL) : "null";
  json += ",\"leds\":[";
  for (uint8_t i = 0; i < 4; i++) {
    if (i) json += ",";
    json += ledState[i] ? "true" : "false";
  }
  json += "],\"buttons\":[";
  for (uint8_t i = 0; i < 4; i++) {
    if (i) json += ",";
    json += stableButton[i] == LOW ? "true" : "false";
  }
  json += "],\"song\":" + String(playingSong);
  json += ",\"buzzerReady\":";
  json += buzzerReady ? "true" : "false";
  json += "}";
  return json;
}

const char PAGE[] PROGMEM = R"HTML(
<!doctype html>
<html lang="ko">
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>IoT 통합 제어</title>
<style>
body{font:18px sans-serif;background:#eef4fa;margin:0;padding:20px}
main{max-width:650px;margin:auto}
section{background:white;padding:18px;margin:14px 0;border-radius:14px}
button{font:inherit;padding:14px;margin:5px;border:1px solid #99b0c0;
 border-radius:9px;background:#edf4fb;cursor:pointer}
.on{background:#126547;color:white}
#leds{display:grid;grid-template-columns:1fr 1fr}
#sensor{white-space:pre-line;font-size:24px}
</style>
<main>
<h1>Wi-Fi IoT 제어판</h1>
<p id="connection">연결 중...</p>
<section><h2>온습도</h2><div id="sensor">센서 준비 중</div></section>
<section><h2>LED 4개</h2><div id="leds"></div></section>
<section><h2>멜로디</h2>
<button onclick="command('/api/music?track=1')">작은 별</button>
<button onclick="command('/api/music?track=2')">환희의 송가</button>
<button onclick="command('/api/music?track=0')">정지</button>
<p id="music"></p>
</section>
<section><h2>물리 버튼</h2><p id="buttons"></p>
<p>짧게 눌렀다 떼기: 해당 LED 전환<br>
0.8초 이상 누르기: 1번=작은 별, 2번=환희의 송가,
3번=음악 정지, 4번=전체 끄기</p>
</section>
<button onclick="command('/api/off')">LED·음악 전체 끄기</button>
</main>
<script>
const ledBox = document.getElementById('leds');
for (let i = 1; i <= 4; i++) {
  const b = document.createElement('button');
  b.id = 'led' + i;
  b.textContent = 'LED ' + i;
  b.onclick = () => command('/api/led?id=' + i + '&state=toggle');
  ledBox.appendChild(b);
}
let busy = false;
async function refresh() {
  if (busy) return;
  busy = true;
  try {
    const response = await fetch('/api/state', {cache:'no-store'});
    if (!response.ok) throw new Error('state');
    const d = await response.json();
    document.getElementById('connection').textContent = 'ESP32 연결됨';
    document.getElementById('sensor').textContent = d.valid
      ? `온도 ${d.temperature} °C / 습도 ${d.humidity} %\n${d.ageSeconds}초 전 측정`
      : '센서 준비 중 또는 읽기 실패';
    d.leds.forEach((on, i) => {
      const b = document.getElementById('led' + (i + 1));
      b.textContent = `LED ${i + 1}: ${on ? '켜짐' : '꺼짐'}`;
      b.className = on ? 'on' : '';
    });
    document.getElementById('music').textContent = !d.buzzerReady
      ? '부저 초기화 오류: 시리얼 확인'
      : ['정지','작은 별 재생 중','환희의 송가 재생 중'][d.song];
    document.getElementById('buttons').textContent = d.buttons
      .map((pressed,i) => `${i+1}번 ${pressed ? '누름' : '놓음'}`).join(' / ');
  } catch (e) {
    document.getElementById('connection').textContent = 'Wi-Fi 연결을 확인하세요';
  } finally {
    busy = false;
  }
}
async function command(url) {
  try {
    const r = await fetch(url, {method:'POST'});
    if (!r.ok) throw new Error('command');
    await refresh();
  } catch(e) {
    document.getElementById('connection').textContent = '명령 전송 실패';
  }
}
setInterval(refresh, 500);
refresh();
</script>
</html>
)HTML";

void setup() {
  Serial.begin(115200);
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }
  dht.begin();
  buzzerReady = ledcAttach(BUZZER_PIN, 2000, 10);
  if (buzzerReady) {
    ledcWriteTone(BUZZER_PIN, 0);
  } else {
    Serial.println("Buzzer LEDC init failed");
  }
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(AP_SSID, AP_PASSWORD)) {
    Serial.println("AP start failed");
  }
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html; charset=utf-8", PAGE);
  });
  server.on("/api/state", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", stateJson());
  });
  server.on("/api/led", HTTP_POST, []() {
    String id = server.arg("id");
    String state = server.arg("state");
    if (id != "1" && id != "2" && id != "3" && id != "4") {
      server.send(400, "text/plain", "id must be 1..4");
      return;
    }
    if (state != "0" && state != "1" && state != "toggle") {
      server.send(400, "text/plain", "state must be 0, 1 or toggle");
      return;
    }
    uint8_t index = id.toInt() - 1;
    setLed(index, state == "toggle" ? !ledState[index] : state == "1");
    server.send(200, "text/plain", "OK");
  });
  server.on("/api/music", HTTP_POST, []() {
    String track = server.arg("track");
    if (track != "0" && track != "1" && track != "2") {
      server.send(400, "text/plain", "track must be 0, 1 or 2");
      return;
    }
    if (track == "0") {
      stopMusic();
    } else if (!buzzerReady) {
      server.send(503, "text/plain", "buzzer unavailable");
      return;
    } else {
      startMusic(track.toInt());
    }
    server.send(200, "text/plain", "OK");
  });
  server.on("/api/off", HTTP_POST, []() {
    allOff();
    server.send(200, "text/plain", "OK");
  });
  server.begin();
}

void loop() {
  server.handleClient();
  updateButtons();
  updateMusic();
  updateSensor();
}
