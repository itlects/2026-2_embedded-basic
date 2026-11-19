#include <SD_MMC.h>
// Only for Freenove FNK0083 board with integrated SD slot.
// This example uses SDMMC 1-bit mode, NOT SPI.
constexpr int SDMMC_CLK = 39;
constexpr int SDMMC_CMD = 38;
constexpr int SDMMC_D0 = 40;

void setup() {
  Serial.begin(115200);
  delay(1500);
  SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_D0);
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("SDMMC mount failed; check FNK0083 board/card.");
    return;
  }
  File file = SD_MMC.open("/sdmmc_test.txt", FILE_APPEND);
  if (!file) { Serial.println("SDMMC open failed."); return; }
  file.println("SDMMC one-bit (not SPI)");
  file.close();
  Serial.println("SDMMC write success.");
}
void loop() {}
