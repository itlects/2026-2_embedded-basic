// UNO ATmega328P, pin-change interrupt: D2 INC, D3 RESET, D4 PAUSE
// Each tactile switch: signal pin <-> switch <-> GND. Serial: 115200.
#include <avr/interrupt.h>
const uint8_t BTN[3]={2,3,4};
volatile uint8_t lastPortD=0;
volatile uint8_t edgeBits=0;
ISR(PCINT2_vect){
 uint8_t current=PIND;
 uint8_t falling=(uint8_t)(lastPortD & (uint8_t)~current);
 edgeBits |= (falling >> 2) & 0x07;
 lastPortD=current;
}
bool awaiting[3]={false,false,false};
uint32_t lowSince[3]={0,0,0};
uint32_t tickAt=0,secondsElapsed=0,count=0;
bool paused=false;
void setup(){
 Serial.begin(115200);
 for(uint8_t i=0;i<3;i++)pinMode(BTN[i],INPUT_PULLUP);
 noInterrupts();
 lastPortD=PIND;
 PCMSK2 |= _BV(PCINT18)|_BV(PCINT19)|_BV(PCINT20); // D2,D3,D4
 PCIFR |= _BV(PCIF2); // clear preexisting pending interrupt
 PCICR |= _BV(PCIE2);
 interrupts();
 tickAt=millis();
 Serial.println("UNO: D2 increment, D3 reset, D4 pause/resume");
}
void loop(){
 uint32_t now=millis();
 while((uint32_t)(now-tickAt)>=1000UL){
  tickAt+=1000UL;
  secondsElapsed++;
  uint32_t hh=(secondsElapsed/3600UL)%24UL,mm=(secondsElapsed/60UL)%60UL,ss=secondsElapsed%60UL;
  char line[24];snprintf(line,sizeof(line),"TIME %02lu:%02lu:%02lu",(unsigned long)hh,(unsigned long)mm,(unsigned long)ss);
  Serial.println(line);
 }
 uint8_t edges;
 noInterrupts();edges=edgeBits;edgeBits=0;interrupts();
 for(uint8_t i=0;i<3;i++){
  if(edges & (1U<<i)){awaiting[i]=true;lowSince[i]=0;}
  if(!awaiting[i])continue;
  if(digitalRead(BTN[i])==HIGH){lowSince[i]=0;continue;}
  if(lowSince[i]==0)lowSince[i]=now;
  if((uint32_t)(now-lowSince[i])<30UL)continue;
  awaiting[i]=false;lowSince[i]=0;
  if(i==0 && !paused)++count;
  else if(i==1)count=0;
  else if(i==2)paused=!paused;
  Serial.print("COUNT ");Serial.print(count);Serial.print(" PAUSE ");Serial.println(paused?"ON":"OFF");
 }
}
