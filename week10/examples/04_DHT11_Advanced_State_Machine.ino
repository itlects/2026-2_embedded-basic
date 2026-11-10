// Week 10 advanced ESP32-DHT11: finite-state alarm, average of 3 samples,
// hysteresis, consecutive sensor-failure handling, CSV logging, nonblocking beep.
// Requires Adafruit DHT sensor library (+ Unified Sensor dependency).
#include <Arduino.h>
#include <DHT.h>
#include <math.h>

constexpr uint8_t PIN_DHT = 4;
constexpr uint8_t PIN_BUZZER = 25;
constexpr uint8_t PIN_LED = 26;
constexpr uint32_t READ_INTERVAL_MS = 2000;
constexpr uint32_t BEEP_PERIOD_MS = 600;
constexpr uint32_t BEEP_ON_MS = 120;
constexpr float TEMP_ON = 30.0f, TEMP_OFF = 28.0f;
constexpr float HUM_ON = 75.0f, HUM_OFF = 70.0f;
constexpr int MAX_ERRORS = 3;
constexpr int WINDOW = 3;

DHT dht(PIN_DHT, DHT11);
enum class AlarmState { NORMAL, WARNING, SENSOR_ERROR };
AlarmState state = AlarmState::NORMAL;
float temps[WINDOW] = {}, hums[WINDOW] = {};
int filled = 0, nextSlot = 0, errors = 0;
uint32_t lastRead = 0, beepStarted = 0;
bool beepActive = false;

const char* stateName(AlarmState s) {
  switch (s) {
    case AlarmState::NORMAL: return "NORMAL";
    case AlarmState::WARNING: return "WARNING";
    default: return "SENSOR_ERROR";
  }
}
void setBuzzer(bool on) {
  if (on) tone(PIN_BUZZER, 2000);
  else noTone(PIN_BUZZER);
}
void updateActuators(uint32_t now) {
  if (state != AlarmState::WARNING) {
    digitalWrite(PIN_LED, state == AlarmState::SENSOR_ERROR ? (now / 250) % 2 : LOW);
    if (beepActive) { setBuzzer(false); beepActive = false; }
    return;
  }
  digitalWrite(PIN_LED, HIGH);
  if (!beepActive && (now - beepStarted >= BEEP_PERIOD_MS)) {
    setBuzzer(true);
    beepActive = true;
    beepStarted = now;
  } else if (beepActive && (now - beepStarted >= BEEP_ON_MS)) {
    setBuzzer(false);
    beepActive = false;
  }
}
void updateReading(uint32_t now) {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(t) || isnan(h) || t < 0 || t > 50 || h < 20 || h > 90) {
    errors++;
    if (errors >= MAX_ERRORS) state = AlarmState::SENSOR_ERROR;
    Serial.printf("%lu,nan,nan,nan,nan,%s,%d\n",
                  (unsigned long)now, stateName(state), errors);
    return;
  }
  errors = 0;
  temps[nextSlot] = t;
  hums[nextSlot] = h;
  nextSlot = (nextSlot + 1) % WINDOW;
  if (filled < WINDOW) filled++;
  float avgT = 0, avgH = 0;
  for (int i = 0; i < filled; i++) { avgT += temps[i]; avgH += hums[i]; }
  avgT /= filled;
  avgH /= filled;
  if (state != AlarmState::WARNING && (avgT >= TEMP_ON || avgH >= HUM_ON))
    state = AlarmState::WARNING;
  else if (state == AlarmState::WARNING && avgT <= TEMP_OFF && avgH <= HUM_OFF)
    state = AlarmState::NORMAL;
  else if (state == AlarmState::SENSOR_ERROR)
    state = AlarmState::NORMAL;
  Serial.printf("%lu,%.1f,%.1f,%.2f,%.2f,%s,%d\n",
                (unsigned long)now, t, h, avgT, avgH, stateName(state), errors);
}
void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  setBuzzer(false);
  dht.begin();
  lastRead = millis();
  beepStarted = millis() - BEEP_PERIOD_MS;
  Serial.println("ms,tempC,humidity,avgTempC,avgHumidity,state,errors");
}
void loop() {
  uint32_t now = millis();
  if ((uint32_t)(now - lastRead) >= READ_INTERVAL_MS) {
    lastRead = now;
    updateReading(now);
  }
  updateActuators(now);
}
