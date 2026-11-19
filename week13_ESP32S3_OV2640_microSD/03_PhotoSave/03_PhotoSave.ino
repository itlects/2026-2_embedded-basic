#include <Arduino.h>
#include "esp_camera.h"
bool initCamera() {
  if (!psramFound()) { Serial.println("Enable OPI PSRAM"); return false; }
  camera_config_t c = {};
  c.ledc_channel=LEDC_CHANNEL_0; c.ledc_timer=LEDC_TIMER_0;
  c.pin_d0=11; c.pin_d1=9; c.pin_d2=8; c.pin_d3=10;
  c.pin_d4=12; c.pin_d5=18; c.pin_d6=17; c.pin_d7=16;
  c.pin_xclk=15; c.pin_pclk=13; c.pin_vsync=6; c.pin_href=7;
  c.pin_sccb_sda=4; c.pin_sccb_scl=5;
  c.pin_pwdn=-1; c.pin_reset=-1;
  c.xclk_freq_hz=20000000; c.pixel_format=PIXFORMAT_JPEG;
  c.frame_size=FRAMESIZE_VGA; c.jpeg_quality=12;
  c.fb_count=1; c.fb_location=CAMERA_FB_IN_PSRAM;
  c.grab_mode=CAMERA_GRAB_WHEN_EMPTY;
  esp_err_t err=esp_camera_init(&c);
  Serial.printf("Camera init: 0x%x\n", (unsigned)err);
  return err==ESP_OK;
}
#include "FS.h"
#include "SD_MMC.h"
bool initSD() {
  if (!SD_MMC.setPins(39,38,40)) return false;
  if (!SD_MMC.begin("/sdcard",true)) return false;
  if (SD_MMC.cardType()==CARD_NONE) return false;
  Serial.printf("SD: %llu MiB\n", (unsigned long long)(SD_MMC.cardSize()/1048576));
  return true;
}
constexpr int BUTTON=2;
bool ready=false;
int nextIndex=1;
void capture() {
  char path[40];
  do {
    if(nextIndex>99999) { Serial.println("File limit"); return; }
    snprintf(path,sizeof(path),"/camera/IMG_%05d.jpg",nextIndex++);
  } while(SD_MMC.exists(path));
  camera_fb_t *fb=esp_camera_fb_get();
  if(!fb) { Serial.println("Capture failed"); return; }
  File f=SD_MMC.open(path,FILE_WRITE);
  size_t expected=fb->len;
  size_t written=f ? f.write(fb->buf,expected) : 0;
  if(f) f.close();
  esp_camera_fb_return(fb);
  File check=SD_MMC.open(path,FILE_READ);
  bool ok=check && check.size()==expected && written==expected;
  if(check) check.close();
  if(!ok) { SD_MMC.remove(path); Serial.println("Save failed"); return; }
  Serial.printf("Saved %s (%u bytes)\n",path,(unsigned)written);
}
void setup() {
  Serial.begin(115200); delay(1000);
  pinMode(BUTTON,INPUT_PULLUP);
  if(!initCamera()) return;
  if(!initSD()) { Serial.println("SD mount failed"); return; }
  if(!SD_MMC.exists("/camera") && !SD_MMC.mkdir("/camera")) return;
  ready=true;
  Serial.println("Ready: press external button or send c");
}
void loop() {
  if(!ready) { delay(100); return; }
  static bool armed=true;
  if(digitalRead(BUTTON)==LOW && armed) {
    delay(30);
    if(digitalRead(BUTTON)==LOW) { armed=false; capture(); }
  }
  if(digitalRead(BUTTON)==HIGH && !armed) {
    delay(30); if(digitalRead(BUTTON)==HIGH) armed=true;
  }
  if(Serial.available() && Serial.read()=='c') capture();
  delay(5);
}
