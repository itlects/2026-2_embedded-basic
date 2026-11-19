#include <Arduino.h>
#include "FS.h"
#include "SD_MMC.h"
bool initSD() {
  if (!SD_MMC.setPins(39,38,40)) return false;
  if (!SD_MMC.begin("/sdcard",true)) return false;
  if (SD_MMC.cardType()==CARD_NONE) return false;
  Serial.printf("SD: %llu MiB\n", (unsigned long long)(SD_MMC.cardSize()/1048576));
  return true;
}
void setup() {
  Serial.begin(115200); delay(1000);
  if (!initSD()) { Serial.println("SD mount failed"); return; }
  File f=SD_MMC.open("/week13.txt",FILE_APPEND);
  if (!f) { Serial.println("Open failed"); return; }
  f.println("Week13 SD OK"); f.close();
  f=SD_MMC.open("/week13.txt",FILE_READ);
  if (!f) { Serial.println("Read failed"); return; }
  while(f.available()) Serial.write(f.read());
  f.close();
}
void loop(){ delay(1000); }
