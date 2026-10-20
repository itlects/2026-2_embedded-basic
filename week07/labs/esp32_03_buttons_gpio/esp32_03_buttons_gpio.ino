// ESP32 DevKit: GPIO18=INC GPIO19=RESET GPIO21=PAUSE. Button other contact -> GND.
const uint8_t BTN[3]={18,19,21};
volatile uint32_t pendingMask=0;
portMUX_TYPE mux=portMUX_INITIALIZER_UNLOCKED;
void ARDUINO_ISR_ATTR onInc(){portENTER_CRITICAL_ISR(&mux);pendingMask|=1U;portEXIT_CRITICAL_ISR(&mux);}
void ARDUINO_ISR_ATTR onReset(){portENTER_CRITICAL_ISR(&mux);pendingMask|=2U;portEXIT_CRITICAL_ISR(&mux);}
void ARDUINO_ISR_ATTR onPause(){portENTER_CRITICAL_ISR(&mux);pendingMask|=4U;portEXIT_CRITICAL_ISR(&mux);}
uint32_t lowSince[3]={0,0,0};
bool waiting[3]={false,false,false};
unsigned long tickAt=0,secondsElapsed=0;
unsigned long count=0;
bool paused=false;
void setup(){
 Serial.begin(115200);
 for(int i=0;i<3;i++)pinMode(BTN[i],INPUT_PULLUP);
 attachInterrupt(digitalPinToInterrupt(BTN[0]),onInc,FALLING);
 attachInterrupt(digitalPinToInterrupt(BTN[1]),onReset,FALLING);
 attachInterrupt(digitalPinToInterrupt(BTN[2]),onPause,FALLING);
 tickAt=millis();
 Serial.println("ESP32 3 buttons, clock, and counter");
}
void loop(){
 unsigned long now=millis();
 while((unsigned long)(now-tickAt)>=1000UL){
  tickAt+=1000UL;++secondsElapsed;
  unsigned long h=(secondsElapsed/3600UL)%24UL,m=(secondsElapsed/60UL)%60UL,s=secondsElapsed%60UL;
  Serial.printf("TIME %02lu:%02lu:%02lu\n",h,m,s);
 }
 uint32_t edges;
 portENTER_CRITICAL(&mux);edges=pendingMask;pendingMask=0;portEXIT_CRITICAL(&mux);
 for(int i=0;i<3;i++){
  if((edges&(1U<<i))!=0){waiting[i]=true;lowSince[i]=0;}
  if(waiting[i]){
   if(digitalRead(BTN[i])==LOW){
    if(lowSince[i]==0)lowSince[i]=now;
    if((unsigned long)(now-lowSince[i])>=30UL){
     waiting[i]=false;lowSince[i]=0;
     if(i==0&&!paused)++count;
     else if(i==1)count=0;
     else if(i==2)paused=!paused;
     Serial.printf("COUNT %lu PAUSE %s\n",count,paused?"ON":"OFF");
    }
   }else lowSince[i]=0;
  }
 }
}
