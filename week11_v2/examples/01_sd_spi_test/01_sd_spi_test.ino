#include <SPI.h>
#include <SD.h>

// ESP32-S3 N16R8: external 3.3V SPI microSD module
constexpr int PIN_SCK = 12;
constexpr int PIN_MISO = 13;
constexpr int PIN_MOSI = 11;
constexpr int PIN_CS = 10;

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("EX01 SPI microSD test");
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);

  if (!SD.begin(PIN_CS, SPI, 4000000)) {
    Serial.println("ERROR: SD.begin() failed. Check wiring, voltage and FAT32.");
    return;
  }
  if (SD.cardType() == CARD_NONE) {
    Serial.println("ERROR: No SD card.");
    return;
  }
  Serial.printf("Card size: %llu MB\n", SD.cardSize() / (1024ULL * 1024ULL));

  File w = SD.open("/hello.txt", FILE_APPEND);
  if (!w) { Serial.println("ERROR: cannot open hello.txt"); return; }
  w.println("Hello from ESP32-S3 SPI!");
  w.close();

  File r = SD.open("/hello.txt", FILE_READ);
  if (!r) { Serial.println("ERROR: cannot read hello.txt"); return; }
  while (r.available()) Serial.write(r.read());
  r.close();
  Serial.println("\nEX01 PASS if message appears.");
}
void loop() {}
