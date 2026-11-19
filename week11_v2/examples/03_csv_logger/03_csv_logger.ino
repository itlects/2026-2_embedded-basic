#include <SPI.h>
#include <SD.h>
#include <DHT.h>

constexpr int PIN_SCK = 12;
constexpr int PIN_MISO = 13;
constexpr int PIN_MOSI = 11;
constexpr int PIN_CS = 10;
constexpr int DHT_PIN = 4;
constexpr uint32_t PERIOD_MS = 5000;
constexpr char CSV_PATH[] = "/dht11.csv";

DHT dht(DHT_PIN, DHT11);
bool sdOK = false;
uint32_t lastSample = 0;

bool ensureHeader() {
  if (SD.exists(CSV_PATH)) return true;
  File file = SD.open(CSV_PATH, FILE_WRITE);
  if (!file) return false;
  size_t n = file.println("elapsed_ms,temperature_c,humidity_pct");
  file.close();
  return n > 0;
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  dht.begin();
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);

  sdOK = SD.begin(PIN_CS, SPI, 4000000);
  if (!sdOK) {
    Serial.println("ERROR: SD card mount failed.");
    return;
  }
  sdOK = ensureHeader();
  if (!sdOK) { Serial.println("ERROR: CSV header creation failed."); return; }
  lastSample = millis();
  Serial.println("Logger ready; elapsed_ms is NOT calendar time.");
}

void loop() {
  if (!sdOK) return;
  uint32_t now = millis();
  if (now - lastSample < PERIOD_MS) return;
  lastSample = now;

  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(t) || isnan(h)) {
    Serial.println("ERROR: sensor reading invalid, row skipped.");
    return;
  }

  File file = SD.open(CSV_PATH, FILE_APPEND);
  if (!file) { Serial.println("ERROR: CSV append failed."); return; }
  file.print(now);
  file.print(',');
  file.print(t, 1);
  file.print(',');
  file.println(h, 1);
  file.flush();
  file.close();
  Serial.printf("LOG %lu,%.1f,%.1f\n", (unsigned long)now, t, h);
}
