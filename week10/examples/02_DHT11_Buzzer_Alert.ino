#include <DHT.h>
constexpr uint8_t DHT_PIN=4, BUZZER_PIN=25;
constexpr float TEMP_ON=30, TEMP_OFF=28, HUM_ON=75, HUM_OFF=70;
DHT dht(DHT_PIN,DHT11);
unsigned long lastRead=0, lastBeep=0, beepStart=0;
bool alarm=false, beep=false;
void setup() {
  Serial.begin(115200); pinMode(BUZZER_PIN,OUTPUT); digitalWrite(BUZZER_PIN,LOW);
  dht.begin(); lastRead=millis();
}
void loop() {
  unsigned long now=millis();
  if(now-lastRead>=2000) {
    lastRead=now;
    float t=dht.readTemperature(), h=dht.readHumidity();
    if(isnan(t)||isnan(h)) {alarm=false; Serial.println("READ ERROR: alarm muted");}
    else {
      if(!alarm&&(t>=TEMP_ON||h>=HUM_ON)) alarm=true;
      else if(alarm&&t<=TEMP_OFF&&h<=HUM_OFF) alarm=false;
      Serial.printf("T=%.1f C, RH=%.1f %%, alarm=%s\n",t,h,alarm?"ON":"OFF");
    }
  }
  if(!alarm) {noTone(BUZZER_PIN); beep=false;}
  else if(!beep&&now-lastBeep>=500) {tone(BUZZER_PIN,2000); beepStart=now; lastBeep=now; beep=true;}
  else if(beep&&now-beepStart>=150) {noTone(BUZZER_PIN); beep=false;}
}
