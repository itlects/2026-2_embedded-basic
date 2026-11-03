#include <SPI.h>
#include <SD.h>
#include <DHT.h>
constexpr int PIN_CS=5, PIN_DHT=27;
constexpr uint32_t INTERVAL_MS=5000;
DHT dht(PIN_DHT,DHT11);
bool sdReady=false;
uint32_t lastSample=0;
void setup(){
 Serial.begin(115200);delay(1000);dht.begin();
 SPI.begin(18,19,23,5);
 sdReady=SD.begin(PIN_CS,SPI,4000000);
 if(!sdReady){Serial.println("SD initialization FAILED");return;}
 if(!SD.exists("/dht11.csv")){
   File f=SD.open("/dht11.csv",FILE_WRITE);
   if(!f){sdReady=false;Serial.println("CSV create FAILED");return;}
   f.println("elapsed_ms,temperature_c,humidity_pct");f.close();
 }
 Serial.println("CSV logger ready. elapsed_ms is NOT wall-clock time.");
}
void loop(){
 if(!sdReady) return;
 uint32_t now=millis();
 if(now-lastSample<INTERVAL_MS) return;
 lastSample=now;
 float h=dht.readHumidity(),t=dht.readTemperature();
 if(isnan(h)||isnan(t)){Serial.println("DHT read FAILED: row skipped");return;}
 File f=SD.open("/dht11.csv",FILE_APPEND);
 if(!f){Serial.println("CSV append FAILED");return;}
 f.print(now);f.print(',');f.print(t,1);f.print(',');f.println(h,1);
 f.flush();f.close();
 Serial.printf("%lu,%.1f,%.1f\n",(unsigned long)now,t,h);
}
