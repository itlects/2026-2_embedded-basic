#include <DHT.h>
constexpr int PIN_DHT=27;
DHT dht(PIN_DHT,DHT11);
void setup(){Serial.begin(115200);dht.begin();}
void loop(){
 float h=dht.readHumidity(), t=dht.readTemperature();
 if(isnan(h)||isnan(t)) Serial.println("DHT read error");
 else Serial.printf("T=%.1f C, RH=%.1f %%\n",t,h);
 delay(2000);
}
