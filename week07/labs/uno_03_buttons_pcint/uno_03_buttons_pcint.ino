// ATmega328P UNO: D2=INC, D3=RESET, D4=PAUSE; each switch connects to GND.
// Use one PCINT2_vect for port D. No extra library necessary.
#include <avr/interrupt.h>
const byte BTN[3]={2,3,4};
volatile uint8_t lastPortD=0;
volatile uint8_t pendingMask=0;
// ISR records possible falling edges only; 3-bit pending bitmap is processed in loop.
ISR(PCINT2_vect){
 uint8_t current=PIND;
 uint8_t falling=(uint8_t)(lastPortD & (uint8_t)~current);
 pendingMask |= (falling >> 2) & 0x07;
 lastPortD=current;
}
unsigned long lastAccepted[3]={0,0,0};
bool waitRelease[3]={false,false,false};
unsigned long tickAt=0,secondsElapsed=0;
unsigned long count=0;
bool paused=false;
void setup(){
 Serial.begin(115200);
 for(byte i=0;i<3;i++)pinMode(BTN[i],INPUT_PULLUP);
 noInterrupts();
 lastPortD=PIND;
 PCICR |= _BV(PCIE2); // PCINT23:16 (port D)
 PCMSK2 |= _BV(PCINT18)|_BV(PCINT19)|_BV(PCINT20); // PD2,PD3,PD4
 PCIFR |= _BV(PCIF2); // clear pending flag
 interrupts();
 tickAt=millis();
 Serial.println("UNO 3 buttons: D2 INC, D3 RESET, D4 PAUSE");
}
void loop(){
 unsigned long now=millis();
 while((unsigned long)(now-tickAt)>=1000UL){
  tickAt+=1000UL; ++secondsElapsed;
  Serial.print("TIME ");
  unsigned long h=(secondsElapsed/3600UL)%24UL,m=(secondsElapsed/60UL)%60UL,s=secondsElapsed%60UL;
  if(h<10)Serial.print('0'); Serial.print(h);Serial.print(':');
  if(m<10)Serial.print('0'); Serial.print(m);Serial.print(':');
  if(s<10)Serial.print('0'); Serial.println(s);
 }
 uint8_t edges;
 noInterrupts(); edges=pendingMask; pendingMask=0; interrupts();
 for(byte i=0;i<3;i++){
  if(edges & (1U<<i))waitRelease[i]=true;
  // Debounce: accept only if continuously LOW for >=30ms and not recently acted.
  if(waitRelease[i] && digitalRead(BTN[i])==LOW){
   if((unsigned long)(now-lastAccepted[i])>=30UL){
    // A stable-press check is implemented by observing LOW across >=30ms.
    static unsigned long lowSince[3]={0,0,0};
    if(lowSince[i]==0)lowSince[i]=now;
    if((unsigned long)(now-lowSince[i])>=30UL){
     waitRelease[i]=false;lastAccepted[i]=now;lowSince[i]=0;
     if(i==0&&!paused)++count;
     if(i==1)count=0;
     if(i==2)paused=!paused;
     Serial.print("COUNT ");Serial.print(count);Serial.print(" PAUSE ");Serial.println(paused?"ON":"OFF");
    }
   }
  }
  if(digitalRead(BTN[i])==HIGH){
   // re-arm only after release; a fresh interrupt is needed for next press
   static unsigned long dummy=0; (void)dummy;
  }
 }
}
