// 중간고사 배선 테스트 스케치 (채점 대상 아님)
// 배선을 마친 뒤 먼저 업로드해서 LED·버튼·부저 연결이 맞는지 확인한다.
// 시리얼 모니터: 115200bps
//  1) 시작하면 LED1→LED2→LED3→LED4가 차례로 켜지고 부저가 "삑" 한 번 울린다.
//  2) 버튼1~3을 누르고 있는 동안 LED1~3이 켜진다. 버튼4를 누르고 있으면 부저가 울린다.
//  3) 버튼 상태가 바뀔 때마다 시리얼 모니터에 출력된다.  예) BTN3 PRESSED
// 이상하면: 켜지지 않는 LED는 극성(긴 다리 +)과 저항을, 반응 없는 버튼은 대각선 다리와 GND 점퍼를 확인한다.

#if defined(ARDUINO_ARCH_ESP32)
const int LED[4] = {16, 17, 18, 19};
const int BTN[4] = {21, 22, 23, 25};
const int BUZZER = 26;
#else
const int LED[4] = {3, 4, 5, 6};
const int BTN[4] = {7, 8, 9, 10};
const int BUZZER = 11;
#endif

int prev[4] = {HIGH, HIGH, HIGH, HIGH};

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 4; i++) {
    pinMode(LED[i], OUTPUT);
    pinMode(BTN[i], INPUT_PULLUP);
  }
  pinMode(BUZZER, OUTPUT);

  Serial.println("=== WIRING TEST ===");
  for (int i = 0; i < 4; i++) {          // LED 하나씩 확인
    digitalWrite(LED[i], HIGH);
    Serial.print("LED"); Serial.print(i + 1); Serial.println(" ON");
    delay(400);
    digitalWrite(LED[i], LOW);
  }
  tone(BUZZER, 440, 200);                // 부저 확인
  delay(300);
  Serial.println("Press buttons 1~4");
}

void loop() {
  for (int i = 0; i < 4; i++) {
    int now = digitalRead(BTN[i]);
    if (now != prev[i]) {
      prev[i] = now;
      Serial.print("BTN"); Serial.print(i + 1);
      Serial.println(now == LOW ? " PRESSED" : " released");
    }
  }
  for (int i = 0; i < 3; i++) digitalWrite(LED[i], prev[i] == LOW ? HIGH : LOW);
  if (prev[3] == LOW) tone(BUZZER, 440); else noTone(BUZZER);
  delay(10);
}
