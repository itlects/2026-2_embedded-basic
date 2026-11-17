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
#include <HTTPClient.h>
const char* SSID = "YOUR_2G_WIFI";
const char* PASSWORD = "YOUR_WIFI_PASSWORD";
const char* BASE_URL = "http://192.168.0.10:8000"; // PC IPv4로 수정
unsigned long lastSend = 0, lastReconnect = 0;
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  dht.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
}
void loop() {
  sampleSensor();
  if (WiFi.status() != WL_CONNECTED) {
    if (millis() - lastReconnect >= 10000) {
      lastReconnect = millis();
      WiFi.reconnect();
    }
    return;
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
  }
  http.begin(client, String(BASE_URL) + "/api/led?device=team01");
  int code = http.GET();
  if (code == 200) {
    String body = http.getString();
    body.trim();
    if (body == "0" || body == "1") digitalWrite(LED_PIN, body == "1" ? HIGH : LOW);
  }
  http.end();
}
