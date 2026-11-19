#if !defined(CONFIG_IDF_TARGET_ESP32S3)
#error "Select ESP32S3 Dev Module for this v2 sketch"
#endif

#include <WiFi.h>
#include <DHT.h>
#include <math.h>
const char* DEVICE_ID = "team01";
DHT dht(4, DHT11);
const int LED_PIN = 1;
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
  delay(500);
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
