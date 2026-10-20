#include <Adafruit_NeoPixel.h>
#include <avr/interrupt.h>
// Arduino UNO: buttons D2/D3/D4 -> GND; strip DIN D6; ring DIN D7.
// Power both WS2812 modules from suitable external regulated +5V; common GND.
constexpr uint8_t STRIP_PIN=6,RING_PIN=7,STRIP_N=8,RING_N=12;
constexpr uint8_t BTN[3]={2,3,4};
Adafruit_NeoPixel strip(STRIP_N,STRIP_PIN,NEO_GRB+NEO_KHZ800);
Adafruit_NeoPixel ring(RING_N,RING_PIN,NEO_GRB+NEO_KHZ800);
volatile uint8_t edgeBits=0,oldPortD=0;
ISR(PCINT2_vect){
 uint8_t now=PIND;
 edgeBits|=((oldPortD & (uint8_t)~now)>>2)&7;
 oldPortD=now;
}
bool pending[3]={false,false,false},paused=false;
uint32_t lowStart[3]={0,0,0},lastSecond=0;
uint32_t ticks=0;
uint8_t level=0;
void draw(){
 strip.clear();ring.clear();
 for(uint8_t i=0;i<level && i<STRIP_N;i++)strip.setPixelColor(i,strip.Color(0,32,0));
 for(uint8_t i=0;i<level && i<RING_N;i++)ring.setPixelColor(i,ring.Color(0,0,24));
 if(paused){strip.setPixelColor(7,strip.Color(40,0,0));ring.setPixelColor(11,ring.Color(40,0,0));}
 strip.show();ring.show();
}
void setup(){
 Serial.begin(115200);
 for(uint8_t i=0;i<3;i++)pinMode(BTN[i],INPUT_PULLUP);
 strip.begin();ring.begin();strip.setBrightness(55);ring.setBrightness(55);draw();
 noInterrupts();
 oldPortD=PIND;PCMSK2|=_BV(PCINT18)|_BV(PCINT19)|_BV(PCINT20);
 PCIFR|=_BV(PCIF2);PCICR|=_BV(PCIE2);interrupts();
 lastSecond=millis();
}
void loop(){
 uint32_t now=millis();
 while((uint32_t)(now-lastSecond)>=1000U){
  lastSecond+=1000U;ticks++;
  Serial.print("CLOCK seconds: ");Serial.println(ticks);
 }
 uint8_t edges;
 noInterrupts();edges=edgeBits;edgeBits=0;interrupts();
 for(uint8_t i=0;i<3;i++){
  if(edges&(1U<<i)){pending[i]=true;lowStart[i]=0;}
  if(!pending[i])continue;
  if(digitalRead(BTN[i])==HIGH){lowStart[i]=0;continue;}
  if(!lowStart[i])lowStart[i]=now;
  if((uint32_t)(now-lowStart[i])<30U)continue;
  pending[i]=false;lowStart[i]=0;
  if(i==0&&!paused)level=(level+1)%13; // 0..12; 8-LED bar saturates at 8
  else if(i==1)level=0;
  else if(i==2)paused=!paused;
  draw();
  Serial.print("LEVEL ");Serial.print(level);Serial.print(" PAUSED ");Serial.println(paused);
 }
}
