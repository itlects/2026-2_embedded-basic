// 예제 2 : 릴레이 모듈로 워터펌프 2초 구동
// 시리얼 모니터에 w 입력(또는 버튼1) -> 2초 급수,  s 입력 -> 즉시 정지

const int RELAY_PIN = 23;     // 릴레이 모듈 IN(S) -> GPIO23
const int BTN_PIN   = 32;     // 버튼1 -> GPIO32, 반대쪽 다리 -> GND

// 릴레이 트리거 방식 : HIGH 트리거 모듈이면 HIGH, LOW 트리거 모듈이면 LOW
const int RELAY_ON  = HIGH;
const int RELAY_OFF = (RELAY_ON == HIGH) ? LOW : HIGH;

const unsigned long RUN_MS = 2000;   // 1회 급수 시간(ms)

bool pumpOn = false;
unsigned long startMs = 0;

void pumpStart() {
  digitalWrite(RELAY_PIN, RELAY_ON);
  pumpOn = true;
  startMs = millis();
  Serial.println("PUMP ON");
}

void pumpStop() {
  digitalWrite(RELAY_PIN, RELAY_OFF);
  if (pumpOn) Serial.println("PUMP OFF");
  pumpOn = false;
}

void setup() {
  Serial.begin(115200);
  digitalWrite(RELAY_PIN, RELAY_OFF);   // 출력으로 바꾸기 전에 OFF 값을 먼저 지정
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);       // 누르면 LOW
  Serial.println("w = water 2s, s = stop");
}

void loop() {
  // 1) 시리얼 명령
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'w' && !pumpOn) pumpStart();
    if (c == 's') pumpStop();
  }

  // 2) 버튼1 : 누르는 순간 한 번만 동작
  static int lastBtn = HIGH;
  int btn = digitalRead(BTN_PIN);
  if (lastBtn == HIGH && btn == LOW && !pumpOn) pumpStart();
  lastBtn = btn;

  // 3) 2초가 지나면 자동 정지 (delay 없이 millis로 시간 확인)
  if (pumpOn && millis() - startMs >= RUN_MS) pumpStop();

  delay(10);
}
