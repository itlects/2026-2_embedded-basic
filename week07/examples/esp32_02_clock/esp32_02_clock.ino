// Same nonblocking software clock for ESP32
unsigned long previous=0,secondsElapsed=0;
void setup(){Serial.begin(115200);previous=millis();}
void loop(){
 unsigned long now=millis();
 while((unsigned long)(now-previous)>=1000UL){
  previous+=1000UL; ++secondsElapsed;
  unsigned long h=(secondsElapsed/3600UL)%24UL,m=(secondsElapsed/60UL)%60UL,s=secondsElapsed%60UL;
  if(h<10)Serial.print('0'); Serial.print(h);Serial.print(':');
  if(m<10)Serial.print('0'); Serial.print(m);Serial.print(':');
  if(s<10)Serial.print('0'); Serial.println(s);
 }
}
