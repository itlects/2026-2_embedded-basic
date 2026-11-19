#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
void setup(){
  Serial.begin(115200); delay(1000);
  SPI.begin(18,19,23,5);
  if(!SD.begin(5,SPI,4000000)){Serial.println("SD failed");return;}
  File f=SD.open("/week13.txt",FILE_APPEND);
  if(!f){Serial.println("Open failed");return;}
  f.println("DevKitC SPI SD OK"); f.close();
  Serial.println("Saved /week13.txt");
}
void loop(){delay(1000);}
