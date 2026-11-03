// Week09 Example 0: 보드·업로드·시리얼 확인 (부품 연결 없음)
// 업로드 후 시리얼 모니터 115200 baud. 보드 정보가 1초마다 출력되면 개발환경 준비 완료.
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== Week09 ESP32-DevKitC V4 board check ===");
  Serial.printf("Chip model   : %s (rev %d)\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("CPU cores    : %d, CPU freq: %lu MHz\n", ESP.getChipCores(), (unsigned long)ESP.getCpuFreqMHz());
  Serial.printf("Flash size   : %lu bytes\n", (unsigned long)ESP.getFlashChipSize());
  Serial.printf("Free heap    : %lu bytes\n", (unsigned long)ESP.getFreeHeap());
  uint64_t mac = ESP.getEfuseMac();
  Serial.printf("eFuse MAC    : %04X%08lX\n", (uint16_t)(mac >> 32), (unsigned long)mac);
}

void loop() {
  static unsigned long last = 0;
  unsigned long now = millis();
  if (now - last >= 1000) {
    last = now;
    Serial.printf("uptime %lu ms\n", now);   // 이 줄이 계속 나오면 loop()가 동작 중
  }
}
