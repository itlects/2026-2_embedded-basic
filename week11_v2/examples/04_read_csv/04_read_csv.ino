#include <SPI.h>
#include <SD.h>
constexpr int SCK_PIN = 12, MISO_PIN = 13, MOSI_PIN = 11, CS_PIN = 10;

void setup() {
  Serial.begin(115200);
  delay(1500);
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  if (!SD.begin(CS_PIN, SPI, 4000000)) {
    Serial.println("ERROR: SD mount failed.");
    return;
  }
  File file = SD.open("/dht11.csv", FILE_READ);
  if (!file) { Serial.println("ERROR: /dht11.csv not found."); return; }
  Serial.println("--- contents of /dht11.csv ---");
  while (file.available()) Serial.write(file.read());
  file.close();
  Serial.println("--- END ---");
}
void loop() {}
