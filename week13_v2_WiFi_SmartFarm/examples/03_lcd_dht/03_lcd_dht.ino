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
