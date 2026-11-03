#include <SPI.h>
#include <SD.h>
constexpr int PIN_CS=5;
void setup() {
 Serial.begin(115200); delay(1000);
 SPI.begin(18,19,23,5); // SCK, MISO, MOSI, SS
 if (!SD.begin(PIN_CS,SPI,4000000)) { Serial.println("SD mount FAILED"); return; }
 Serial.printf("SD size: %llu MB\n",SD.cardSize()/(1024ULL*1024ULL));
 File f=SD.open("/test.txt",FILE_APPEND);
 if (!f) {Serial.println("open FAILED");return;}
 f.println("ESP32 SD SPI test");f.close();
 f=SD.open("/test.txt",FILE_READ);
 if(!f){Serial.println("read FAILED");return;}
 while(f.available()) Serial.write(f.read()); f.close();
}
void loop() {}
