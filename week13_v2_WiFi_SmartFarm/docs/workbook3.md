# 워크북 3 · DHT22와 LCD1602

## 최종 결과
LCD 첫 줄 온도, 둘째 줄 습도. 2.5초마다 갱신합니다.

1. DHT22 VCC→3V3, GND→GND, DATA→GPIO16(DevKitC)/GPIO4(S3). DATA→3V3에 10kΩ 풀업(모듈 내장 시 생략).
2. 5V I2C LCD는 양방향 레벨시프터 사용. HV=5V/LV=3V3/GND 공통, SDA·SCL 각각 양방향 채널. DevKitC 21/22, S3 8/9.
3. [03_lcd_dht.ino](https://github.com/itlects/2026-2_embedded-basic/blob/main/week13_v2_WiFi_SmartFarm/examples/03_lcd_dht/03_lcd_dht.ino) 업로드. 화면 미표시: 대비 가변저항, 주소 0x27/0x3F, I2C 연결 확인. DHT error: 종류 DHT22, 배선, 풀업 확인.

```cpp
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#if CONFIG_IDF_TARGET_ESP32S3
const int SOIL=1,DHTPIN=4,SDA_PIN=8,SCL_PIN=9,RELAY=17,BUZZ=16;
const int LEDS[]={5,6,7,10,11}, BTNS[]={12,13,14,15};
#else
const int SOIL=34,DHTPIN=16,SDA_PIN=21,SCL_PIN=22,RELAY=23,BUZZ=27;
const int LEDS[]={4,13,14,18,19}, BTNS[]={32,33,25,26};
#endif
DHT dht(DHTPIN,DHT22);LiquidCrystal_I2C lcd(0x27,16,2);
void setup(){Serial.begin(115200);Wire.begin(SDA_PIN,SCL_PIN);lcd.init();lcd.backlight();dht.begin();}
void loop(){float t=dht.readTemperature(),h=dht.readHumidity();lcd.clear();if(isnan(t)||isnan(h)){lcd.print("DHT error");}else{lcd.print("Temp: ");lcd.print(t,1);lcd.print(" C");lcd.setCursor(0,1);lcd.print("Hum: ");lcd.print(h,1);lcd.print(" %");}delay(2500);}

```

온도·습도를 3회 기록하고 주변 환경에 따른 변화를 확인합니다.