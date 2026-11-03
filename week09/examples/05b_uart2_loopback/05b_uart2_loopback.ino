// Week09 Example 5B: ESP32 UART2 루프백 — TX2(GPIO17)와 RX2(GPIO16)를 점퍼선 1개로 연결
// 시리얼 모니터(UART0)에 쓴 글자 → UART2 TX2 → 점퍼 → RX2 → 다시 시리얼 모니터에 [UART2 RX] 로 표시
// 점퍼를 빼면 아무것도 돌아오지 않는다 → TX/RX가 '전선'이라는 것을 확인
// (ESP32 전용: UNO는 하드웨어 UART가 1개뿐이라 이 실험이 어렵다)
#if !defined(ESP32)
#error "This example needs ESP32 (second hardware UART)."
#endif
const int RX2_PIN = 16, TX2_PIN = 17;
long bauds[] = {9600, 115200};
int bi = 0;

void setup() {
  Serial.begin(115200);                                   // UART0 ↔ USB ↔ PC
  Serial2.begin(bauds[bi], SERIAL_8N1, RX2_PIN, TX2_PIN); // UART2, 8데이터비트·패리티없음·정지비트1
  Serial.println("Type text and press Enter. Send '#' to switch UART2 baud.");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '#') {
      bi = 1 - bi;
      Serial2.updateBaudRate(bauds[bi]);
      Serial.printf("UART2 baud = %ld\n", bauds[bi]);
      continue;
    }
    Serial2.write(c);                                     // UART2로 송신
  }
  while (Serial2.available()) {                           // UART2로 수신된 것
    char c = Serial2.read();
    Serial.print("[UART2 RX] 0x"); Serial.print((uint8_t)c, HEX);
    Serial.print(" '"); Serial.print(c >= 32 ? c : '.'); Serial.println("'");
  }
}
