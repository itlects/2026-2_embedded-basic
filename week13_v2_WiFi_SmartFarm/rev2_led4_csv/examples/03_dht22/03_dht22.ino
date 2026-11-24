#include <DHT.h>
const int SOIL=34,DHTPIN=16,SDA_PIN=21,SCL_PIN=22,RELAY=23,BUZZ=27;
const int LEDS[]={4,13,14,18}, BTNS[]={32,33,25,26};

DHT dht(DHTPIN,DHT22);
void setup(){Serial.begin(115200);dht.begin();}
void loop(){float t=dht.readTemperature(),h=dht.readHumidity();if(isnan(t)||isnan(h))Serial.println("DHT error");else Serial.printf("%.1f C / %.1f %%\n",t,h);delay(2500);}
