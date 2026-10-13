/*
 * 5주차 종합과제  스마트 온습도 경보 무드등  — 완성 코드 (Arduino UNO)
 * 센서 선택 (SENSOR_MODE) : 0 DHT11(실물) / 1 DHT22(Wokwi) / 2 TMP36(Tinkercad, A1)
 * UNO 의 tone() 은 Timer2 를 사용하므로 D3·D11 의 PWM 은 사용 불가 → 버튼/부저로 배정
 * I2C LCD 16x2 : SDA A4, SCL A5, 주소 0x27 (Tinkercad 는 Adafruit_LiquidCrystal, 주소 0x20)
 */
#define SENSOR_MODE 1          // 0: DHT11(실물)  1: DHT22(Wokwi)  2: TMP36(Tinkercad)

#if SENSOR_MODE == 0 || SENSOR_MODE == 1
  #include <DHT.h>
  #define DHT_PIN 4
  #if SENSOR_MODE == 0
    DHT dht(DHT_PIN, DHT11);
  #else
    DHT dht(DHT_PIN, DHT22);
  #endif
#else
  const int TMP36_PIN = A1;
#endif

// ---------- I2C LCD 16x2 ----------
#include <Wire.h>
#if SENSOR_MODE == 2
  #include <Adafruit_LiquidCrystal.h>
  Adafruit_LiquidCrystal lcd(0);          // Tinkercad I2C LCD (주소 0x20)
#else
  #include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd(0x27, 16, 2);     // 화면이 안 나오면 주소를 0x3F 로 바꿔 본다
#endif

// ---------- 핀 ----------
const int POT_PIN = A0;
const int LED_R = 9, LED_G = 6, LED_B = 5;
const int BAR_PINS[3] = {7, 8, 13};
const int BUZZER_PIN = 11;
const int BTN_MODE = 2, BTN_ALARM = 3, BTN_REPORT = 10, BTN_RESET = 12;

// ---------- 설정값 ----------
const int   ADC_MAX = 1023;
const float COLD_T = 20.0;              // 이 온도 미만 = 저온(파랑)
const float DEFAULT_THRESHOLD = 28.0;   // 고온 경보 임계값 기본
const float TH_MIN = 20.0, TH_MAX = 35.0;
const unsigned long SENSOR_MS = 2000;   // DHT11 은 1초 이상 간격 필요
const unsigned long LCD_MS = 500;       // LCD 갱신 주기 (I2C 전송에 수 ms 걸리므로 매 loop 갱신 X)
const unsigned long MSG_MS = 1500;      // 버튼 메시지 표시 시간

// ---------- 상태 ----------
enum Mode { MODE_NORMAL, MODE_SET };
enum EnvState { ENV_ERROR, ENV_COLD, ENV_COMFORT, ENV_HOT };
const char* ENV_NAME[] = {"센서오류", "저온", "쾌적", "고온"};
const char* ENV_CODE[] = {"ERR", "COLD", "OK", "HOT!"};   // LCD 는 한글 표시 불가 → 영문

Mode mode = MODE_NORMAL;
EnvState env = ENV_ERROR;
float temperature = NAN, humidity = NAN;
float threshold = DEFAULT_THRESHOLD;
bool alarmEnabled = true;
int potAdc = 0, brightness = 255;

unsigned long lastSensorMs = 0, lastPotMs = 0, lastBlinkMs = 0, lastSirenMs = 0;
bool blinkOn = false, sirenHigh = false, sirenActive = false;

unsigned long lastLcdMs = 0, msgStartMs = 0;
bool lcdDirty = true, msgActive = false;
char msgText[17] = "";

// ---------- 버튼 (디바운스) ----------
struct Button { int pin; bool stable; bool lastRead; unsigned long changedMs; };
Button buttons[4] = {
  {BTN_MODE, HIGH, HIGH, 0}, {BTN_ALARM, HIGH, HIGH, 0},
  {BTN_REPORT, HIGH, HIGH, 0}, {BTN_RESET, HIGH, HIGH, 0}
};
bool pressed(Button &b) {
  bool r = digitalRead(b.pin);
  if (r != b.lastRead) { b.lastRead = r; b.changedMs = millis(); }
  if (millis() - b.changedMs > 30 && r != b.stable) {
    b.stable = r;
    return b.stable == LOW;
  }
  return false;
}

// ---------- 센서 ----------
void readSensor() {
#if SENSOR_MODE == 2
  int a = analogRead(TMP36_PIN);
  float v = a * 5.0 / 1024.0;
  temperature = (v - 0.5) * 100.0;          // TMP36: 10 mV/°C, 0.5V = 0°C
  humidity = 50.0;
#else
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
#endif
  if (isnan(temperature) || isnan(humidity)) env = ENV_ERROR;
  else if (temperature < COLD_T)             env = ENV_COLD;
  else if (temperature < threshold)          env = ENV_COMFORT;
  else                                       env = ENV_HOT;
}

void report() {
  Serial.print(F("T="));
  if (isnan(temperature)) Serial.print(F("--")); else Serial.print(temperature, 1);
  Serial.print(F("C H="));
  if (isnan(humidity)) Serial.print(F("--")); else Serial.print(humidity, 0);
  Serial.print(F("% | 상태=")); Serial.print(ENV_NAME[env]);
  Serial.print(F(" | 임계=")); Serial.print(threshold, 1);
  Serial.print(F("C | 경보=")); Serial.print(alarmEnabled ? F("ON") : F("OFF"));
  Serial.print(F(" | 모드=")); Serial.print(mode == MODE_NORMAL ? F("NORMAL") : F("SET"));
  Serial.print(F(" | POT=")); Serial.print(potAdc);
  Serial.print(F(" 밝기PWM=")); Serial.println(brightness);
}

// ---------- 출력 ----------
void setRGB(int r, int g, int b) {
  analogWrite(LED_R, r); analogWrite(LED_G, g); analogWrite(LED_B, b);
}

void updateStatusLed() {
  if (mode == MODE_SET) {                       // 설정 모드: 보라색(R+B) 빠른 점멸
    setRGB(blinkOn ? 255 : 0, 0, blinkOn ? 255 : 0);
    return;
  }
  switch (env) {
    case ENV_ERROR:   setRGB(0, 0, blinkOn ? brightness : 0); break;
    case ENV_COLD:    setRGB(0, 0, brightness);               break;
    case ENV_COMFORT: setRGB(0, brightness, 0);               break;
    case ENV_HOT:     setRGB(blinkOn ? brightness : 0, 0, 0); break;
  }
}

void updateHumidityBar() {
  int n = 0;
  if (!isnan(humidity)) n = (humidity < 40) ? 1 : (humidity <= 60) ? 2 : 3;
  for (int i = 0; i < 3; i++) digitalWrite(BAR_PINS[i], i < n ? HIGH : LOW);
}

void updateSiren(unsigned long now) {
  bool need = (mode == MODE_NORMAL && env == ENV_HOT && alarmEnabled);
  if (!need) {
    if (sirenActive) { noTone(BUZZER_PIN); sirenActive = false; }
    return;
  }
  if (!sirenActive || now - lastSirenMs >= 300) {   // 880/660 Hz 교대 사이렌
    lastSirenMs = now;
    sirenHigh = !sirenHigh;
    tone(BUZZER_PIN, sirenHigh ? 880 : 660);
    sirenActive = true;
  }
}

// ---------- LCD 표시 ----------
void lcdBegin() {
#if SENSOR_MODE == 2
  lcd.begin(16, 2); lcd.setBacklight(1);
#else
  lcd.init(); lcd.backlight();
#endif
}

void lcdLine(uint8_t row, const char* text) {     // 16칸을 공백으로 채워 덮어쓰기 (clear 깜빡임 방지)
  char buf[17];
  snprintf(buf, sizeof(buf), "%-16s", text);
  lcd.setCursor(0, row);
  lcd.print(buf);
}

void showMessage(const char* text) {              // 2번째 줄에 1.5초간 메시지
  strncpy(msgText, text, 16); msgText[16] = '\0';
  msgActive = true; msgStartMs = millis(); lcdDirty = true;
}

void updateLcd(unsigned long now) {
  if (msgActive && now - msgStartMs >= MSG_MS) { msgActive = false; lcdDirty = true; }
  if (!lcdDirty && now - lastLcdMs < LCD_MS) return;
  lastLcdMs = now; lcdDirty = false;

  char l1[17], l2[17], t[8], th[8];
  dtostrf(threshold, 4, 1, th);
  if (mode == MODE_SET) {                         // SET 모드 화면
    snprintf(l1, sizeof(l1), "SET THRESHOLD");
    snprintf(l2, sizeof(l2), "Th:%sC P:%d", th, potAdc);
  } else {                                        // NORMAL 모드 화면
    if (isnan(temperature) || isnan(humidity)) {
      snprintf(l1, sizeof(l1), "T--.-C H--%% %s", ENV_CODE[env]);
    } else {
      dtostrf(temperature, 4, 1, t);
      snprintf(l1, sizeof(l1), "T%sC H%d%% %s", t, (int)humidity, ENV_CODE[env]);
    }
    snprintf(l2, sizeof(l2), "Th%s ALM:%s", th, alarmEnabled ? "ON" : "OFF");
  }
  lcdLine(0, l1);
  lcdLine(1, msgActive ? msgText : l2);
}

void resetAll() {
  mode = MODE_NORMAL;
  threshold = DEFAULT_THRESHOLD;
  alarmEnabled = true;
  noTone(BUZZER_PIN); sirenActive = false;
  readSensor();
  Serial.println(F("# 초기화 완료"));
  report();
  showMessage("RESET Th 28.0");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_R, OUTPUT); pinMode(LED_G, OUTPUT); pinMode(LED_B, OUTPUT);
  for (int i = 0; i < 3; i++) pinMode(BAR_PINS[i], OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(buttons[i].pin, INPUT_PULLUP);
#if SENSOR_MODE != 2
  dht.begin();
#endif
  Serial.println(F("# 5주차 종합과제: 스마트 온습도 경보 무드등 (UNO)"));
  lcdBegin();
  lcdLine(0, "Smart Env Lamp");
  lcdLine(1, "Week5 ADC+DHT");
  delay(1000);                 // 센서 안정화 + 시작 화면 (setup 에서만 사용)
  readSensor(); report();
}

void loop() {
  unsigned long now = millis();

  // 1) 버튼
  if (pressed(buttons[0])) {
    mode = (mode == MODE_NORMAL) ? MODE_SET : MODE_NORMAL;
    Serial.print(F("# 모드 -> ")); Serial.println(mode == MODE_NORMAL ? F("NORMAL") : F("SET"));
    if (mode == MODE_NORMAL) { readSensor(); report(); showMessage("SAVED"); }   // 새 임계값으로 재판정
    lcdDirty = true;
  }
  if (pressed(buttons[1])) {
    alarmEnabled = !alarmEnabled;
    Serial.print(F("# 경보음 ")); Serial.println(alarmEnabled ? F("ON") : F("OFF"));
    showMessage(alarmEnabled ? "ALARM ON" : "ALARM OFF");
  }
  if (pressed(buttons[2])) { readSensor(); report(); lastSensorMs = now; showMessage("MEASURED"); }
  if (pressed(buttons[3])) resetAll();

  // 2) 가변저항(ADC) : 모드에 따라 역할이 달라짐
  if (now - lastPotMs >= 50) {
    lastPotMs = now;
    potAdc = analogRead(POT_PIN);
    if (mode == MODE_NORMAL) {
      brightness = map(potAdc, 0, ADC_MAX, 20, 255);
    } else {
      // 20.0 ~ 35.0 °C, 0.5 °C 단위 → ADC 를 31단계로 다시 양자화
      float newTh = TH_MIN + (long)potAdc * 31 / (ADC_MAX + 1) * 0.5;
      if (newTh != threshold) {
        threshold = newTh;
        Serial.print(F("# 임계값 = ")); Serial.print(threshold, 1); Serial.println(F(" C"));
        lcdDirty = true;
      }
    }
  }

  // 3) 센서 주기 측정
  if (now - lastSensorMs >= SENSOR_MS) {
    lastSensorMs = now;
    readSensor();
    report();
    lcdDirty = true;
  }

  // 4) 점멸 타이밍 (설정모드 250ms, 그 외 500ms → 1초 주기)
  unsigned long blinkMs = (mode == MODE_SET) ? 250 : 500;
  if (now - lastBlinkMs >= blinkMs) { lastBlinkMs = now; blinkOn = !blinkOn; }

  // 5) 출력 (LED · 습도바 · 사이렌 · LCD)
  updateStatusLed();
  updateHumidityBar();
  updateSiren(now);
  updateLcd(now);
}
