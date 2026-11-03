#include <SPI.h>
#include <SD.h>
void setup(){
 Serial.begin(115200);delay(1000);SPI.begin(18,19,23,5);
 if(!SD.begin(5,SPI,4000000)){Serial.println("SD FAILED");return;}
 File f=SD.open("/dht11.csv",FILE_READ);
 if(!f){Serial.println("CSV not found");return;}
 while(f.available()) Serial.write(f.read());
 f.close();
}
void loop(){}
