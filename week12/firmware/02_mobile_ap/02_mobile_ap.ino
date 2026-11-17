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
