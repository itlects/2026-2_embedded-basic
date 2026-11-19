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
void setup() {
  Serial.begin(115200); delay(1000);
  if (!initCamera()) return;
  delay(1000);
  camera_fb_t *fb=esp_camera_fb_get();
  if (!fb) { Serial.println("Capture failed"); return; }
  Serial.printf("JPEG %u x %u, %u bytes\n",(unsigned)fb->width,(unsigned)fb->height,(unsigned)fb->len);
  esp_camera_fb_return(fb);
}
void loop() { delay(1000); }
