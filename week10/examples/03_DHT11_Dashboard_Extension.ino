#include <DHT.h>
constexpr uint8_t DHT_PIN=4, BUZZER_PIN=25, LED_PIN=26;
DHT dht(DHT_PIN,DHT11);
unsigned long previous=0; bool alarm=false;
void setup(){Serial.begin(115200);pinMode(LED_PIN,OUTPUT);pinMode(BUZZER_PIN,OUTPUT);dht.begin();previous=millis();Serial.println("tempC,humidity,alert");}
void loop(){
  unsigned long now=millis(); if(now-previous<2000)return; previous=now;
  float t=dht.readTemperature(),h=dht.readHumidity();
  if(isnan(t)||isnan(h)){alarm=false;noTone(BUZZER_PIN);digitalWrite(LED_PIN,LOW);Serial.println("nan,nan,ERROR");return;}
  if(!alarm&&(t>=30||h>=75))alarm=true;
  else if(alarm&&t<=28&&h<=70)alarm=false;
  digitalWrite(LED_PIN,alarm?HIGH:LOW);
  if(alarm)tone(BUZZER_PIN,1700,100);else noTone(BUZZER_PIN);
  Serial.printf("%.1f,%.1f,%d\n",t,h,alarm?1:0);
}
