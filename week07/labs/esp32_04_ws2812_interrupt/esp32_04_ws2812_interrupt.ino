#include <Adafruit_NeoPixel.h>
// ESP32: button GPIO18,19,21 (each to GND). WS2812 DIN on GPIO23,GPIO22.
// Use common ground, external regulated 5V LED supply, 5V logic level shifter.
Adafruit_NeoPixel bar(8,23,NEO_GRB+NEO_KHZ800),ring(12,22,NEO_GRB+NEO_KHZ800);
const byte btn[]={18,19,21}; volatile uint32_t flags=0;
portMUX_TYPE mux=portMUX_INITIALIZER_UNLOCKED;
void ARDUINO_ISR_ATTR incISR(){portENTER_CRITICAL_ISR(&mux);flags|=1;portEXIT_CRITICAL_ISR(&mux);}
void ARDUINO_ISR_ATTR rstISR(){portENTER_CRITICAL_ISR(&mux);flags|=2;portEXIT_CRITICAL_ISR(&mux);}
void ARDUINO_ISR_ATTR pauseISR(){portENTER_CRITICAL_ISR(&mux);flags|=4;portEXIT_CRITICAL_ISR(&mux);}
bool waitLow[3]={},paused=false;uint32_t lowAt[3]={},secondAt=0,seconds=0;byte level=0;
void render(){
 bar.clear();ring.clear();
 for(byte i=0;i<level&&i<8;i++)bar.setPixelColor(i,bar.Color(0,30,0));
 for(byte i=0;i<level&&i<12;i++)ring.setPixelColor(i,ring.Color(0,0,30));
 if(paused){bar.setPixelColor(7,bar.Color(30,0,0));ring.setPixelColor(11,ring.Color(30,0,0));}
 bar.show();ring.show();
}
void setup(){
 Serial.begin(115200);
 for(byte i=0;i<3;i++)pinMode(btn[i],INPUT_PULLUP);
 bar.begin();ring.begin();bar.setBrightness(50);ring.setBrightness(50);render();
 attachInterrupt(digitalPinToInterrupt(btn[0]),incISR,FALLING);
 attachInterrupt(digitalPinToInterrupt(btn[1]),rstISR,FALLING);
 attachInterrupt(digitalPinToInterrupt(btn[2]),pauseISR,FALLING);
 secondAt=millis();
}
void loop(){
 uint32_t now=millis();
 while((uint32_t)(now-secondAt)>=1000){secondAt+=1000;++seconds;Serial.printf("CLOCK %lu seconds\n",(unsigned long)seconds);}
 uint32_t event;
 portENTER_CRITICAL(&mux);event=flags;flags=0;portEXIT_CRITICAL(&mux);
 for(byte i=0;i<3;i++){
  if(event&(1U<<i)){waitLow[i]=true;lowAt[i]=0;}
  if(!waitLow[i])continue;
  if(digitalRead(btn[i])==HIGH){lowAt[i]=0;continue;}
  if(!lowAt[i])lowAt[i]=now;
  if((uint32_t)(now-lowAt[i])<30)continue;
  waitLow[i]=false;lowAt[i]=0;
  if(i==0&&!paused)level=(level+1)%13;
  if(i==1)level=0;
  if(i==2)paused=!paused;
  render();Serial.printf("LEVEL %u PAUSE %u\n",level,paused);
 }
}
