#include <Arduino.h>
#if !defined(CONFIG_IDF_TARGET_ESP32S3)
#error "Select ESP32S3 Dev Module"
#endif
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.print("Chip: ");
  Serial.println(ESP.getChipModel());
  Serial.printf("Flash: %u MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("PSRAM: %u MB\n", ESP.getPsramSize() / (1024 * 1024));
  Serial.println("Expected: ESP32-S3 / Flash 16 MB / PSRAM 8 MB");
}
void loop() { delay(10); }
