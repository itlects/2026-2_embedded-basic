#include <DHT.h>
DHT dht(16,DHT22);
const int leds[]={4,13,14,18},buttons[]={32,33,25,26};bool last[4]={1,1,1,1},on[4]={0,0,0,0};unsigned long tick=0;
void setup(){Serial.begin(115200);dht.begin();for(int i=0;i<4;i++){pinMode(leds[i],OUTPUT);pinMode(buttons[i],INPUT_PULLUP);}}
void loop(){for(int i=0;i<4;i++){bool b=digitalRead(buttons[i]);if(!b&&last[i]){on[i]=!on[i];digitalWrite(leds[i],on[i]);}last[i]=b;}if(millis()-tick>=3000){tick=millis();Serial.printf("%.1f C / %.1f %%\n",dht.readTemperature(),dht.readHumidity());}delay(40);}
