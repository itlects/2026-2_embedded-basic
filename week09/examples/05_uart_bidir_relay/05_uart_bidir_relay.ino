// Week09 Example 5: UART 양방향 통신 — 센서값 송신 + PC 명령 수신으로 릴레이 제어
// 보드: ESP32-DevKitC V4 또는 Arduino UNO (같은 코드, 핀만 자동 선택)
// 시리얼 모니터: 115200 baud, 줄 끝 = "새 줄(Newline)" 또는 "Both NL & CR"
//
// [MCU → PC] 주기마다 CSV 한 줄:  DATA,<ms>,<adc>,<percent>,<relay>,<mode>
// [PC → MCU] 명령 한 줄 (대소문자 무관):
//   ON / OFF          릴레이 수동 ON/OFF (AUTO 해제)
//   AUTO <0~100>      센서 % 가 임계값 이상이면 자동 ON (예: AUTO 60)
//   PERIOD <ms>       송신 주기 변경 (100~10000)
//   STATUS            현재 상태 1회 출력
//   HELP              명령 목록
#if defined(ESP32)
const int SENSOR_PIN = 34;        // 가변저항 와이퍼 (ADC1_CH6)
const int RELAY_PIN  = 32;        // 릴레이 모듈 IN
const int ADC_MAX    = 4095;
#else
const int SENSOR_PIN = A0;
const int RELAY_PIN  = 7;
const int ADC_MAX    = 1023;
#endif
// 대부분의 1채널 릴레이 모듈(옵토커플러형)은 IN=LOW일 때 동작(Active-LOW).
// 모듈의 점퍼/설명서를 보고 맞게 바꾼다.
const bool RELAY_ACTIVE_LOW = true;

bool relayOn = false;
bool autoMode = false;
int  autoThreshold = 60;          // %
unsigned long period = 1000;      // ms
unsigned long lastSend = 0;
char line[32];                    // 수신 버퍼(명령 한 줄)
byte lineLen = 0;

void relayWrite(bool on) {
  relayOn = on;
#if defined(ESP32)
  if (RELAY_ACTIVE_LOW) {
    // 5V 전원 Active-LOW 모듈 + 3.3V GPIO: HIGH(3.3V)로는 완전히 꺼지지 않을 수 있다.
    // 끌 때는 핀을 INPUT(고임피던스)으로 만들어 모듈 내부 풀업이 끄게 한다.
    if (on) { pinMode(RELAY_PIN, OUTPUT); digitalWrite(RELAY_PIN, LOW); }
    else    { pinMode(RELAY_PIN, INPUT); }
    return;
  }
#endif
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, (on ^ RELAY_ACTIVE_LOW) ? HIGH : LOW);
}

int readPercent(int &raw) {
  raw = analogRead(SENSOR_PIN);
  return (int)((long)raw * 100 / ADC_MAX);
}

void printStatus() {
  int raw; int pct = readPercent(raw);
  Serial.print(F("STATUS relay=")); Serial.print(relayOn ? F("ON") : F("OFF"));
  Serial.print(F(" mode="));        Serial.print(autoMode ? F("AUTO") : F("MANUAL"));
  Serial.print(F(" threshold="));   Serial.print(autoThreshold);
  Serial.print(F("% period="));     Serial.print(period);
  Serial.print(F("ms sensor="));    Serial.print(pct); Serial.println('%');
}

void printHelp() {
  Serial.println(F("Commands: ON | OFF | AUTO <0-100> | PERIOD <ms> | STATUS | HELP"));
}

void handleCommand(char *cmd) {
  for (char *p = cmd; *p; p++) if (*p >= 'a' && *p <= 'z') *p -= 32;   // 대문자로
  if (strcmp(cmd, "ON") == 0)          { autoMode = false; relayWrite(true);  Serial.println(F("OK RELAY ON")); }
  else if (strcmp(cmd, "OFF") == 0)    { autoMode = false; relayWrite(false); Serial.println(F("OK RELAY OFF")); }
  else if (strncmp(cmd, "AUTO", 4) == 0) {
    int v = (cmd[4] == ' ') ? atoi(cmd + 5) : autoThreshold;
    if (v < 0 || v > 100) { Serial.println(F("ERR AUTO 0~100")); return; }
    autoThreshold = v; autoMode = true;
    Serial.print(F("OK AUTO ")); Serial.println(autoThreshold);
  }
  else if (strncmp(cmd, "PERIOD ", 7) == 0) {
    long v = atol(cmd + 7);
    if (v < 100 || v > 10000) { Serial.println(F("ERR PERIOD 100~10000")); return; }
    period = v; Serial.print(F("OK PERIOD ")); Serial.println(period);
  }
  else if (strcmp(cmd, "STATUS") == 0) printStatus();
  else if (strcmp(cmd, "HELP") == 0)   printHelp();
  else if (cmd[0] != '\0') { Serial.print(F("ERR unknown: ")); Serial.println(cmd); }
}

void setup() {
  Serial.begin(115200);
#if defined(ESP32)
  analogReadResolution(12);
  analogSetPinAttenuation(SENSOR_PIN, ADC_11db);
#endif
  relayWrite(false);
  Serial.println(F("# Week09 UART bidirectional demo"));
  printHelp();
  Serial.println(F("# DATA,ms,adc,percent,relay,mode"));
}

void loop() {
  // 1) 수신: 들어온 바이트를 줄 단위로 모은다 (블로킹 없음)
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') { line[lineLen] = '\0'; handleCommand(line); lineLen = 0; }
    else if (lineLen < sizeof(line) - 1) line[lineLen++] = c;
  }
  // 2) 송신: 주기마다 센서값 CSV
  unsigned long now = millis();
  if (now - lastSend >= period) {
    lastSend = now;
    int raw; int pct = readPercent(raw);
    if (autoMode) {
      bool want = relayOn ? (pct >= autoThreshold - 5) : (pct >= autoThreshold);  // 5% 히스테리시스
      if (want != relayOn) relayWrite(want);
    }
    Serial.print(F("DATA,")); Serial.print(now); Serial.print(',');
    Serial.print(raw); Serial.print(','); Serial.print(pct); Serial.print(',');
    Serial.print(relayOn ? 1 : 0); Serial.print(',');
    Serial.println(autoMode ? F("AUTO") : F("MANUAL"));
  }
}
