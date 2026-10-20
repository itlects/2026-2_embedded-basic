// ESP32 DevKit: GPIO18 INC, GPIO19 RESET, GPIO21 PAUSE; buttons to GND.
// Uses ESP32 Arduino core, 115200 serial. No external libraries.
const uint8_t BTN[3]={18,19,21};
volatile uint32_t edgeBits=0;
portMUX_TYPE mux=portMUX_INITIALIZER_UNLOCKED;
void ARDUINO_ISR_ATTR isrInc(){portENTER_CRITICAL_ISR(&mux);edgeBits|=1U;portEXIT_CRITICAL_ISR(&mux);}
void ARDUINO_ISR_ATTR isrReset(){portENTER_CRITICAL_ISR(&mux);edgeBits|=2U;portEXIT_CRITICAL_ISR(&mux);}
void ARDUINO_ISR_ATTR isrPause(){portENTER_CRITICAL_ISR(&mux);edgeBits|=4U;portEXIT_CRITICAL_ISR(&mux);}
bool awaiting[3]={false,false,false};
uint32_t lowSince[3]={0,0,0};
uint32_t tickAt=0,secondsElapsed=0,count=0;
bool paused=false;
void setup(){
 Serial.begin(115200);
 for(uint8_t i=0;i<3;i++)pinMode(BTN[i],INPUT_PULLUP);
 attachInterrupt(digitalPinToInterrupt(BTN[0]),isrInc,FALLING);
 attachInterrupt(digitalPinToInterrupt(BTN[1]),isrReset,FALLING);
 attachInterrupt(digitalPinToInterrupt(BTN[2]),isrPause,FALLING);
 tickAt=millis();
 Serial.println("ESP32: GPIO18 increment, GPIO19 reset, GPIO21 pause/resume");
}
void loop(){
 uint32_t now=millis();
 while((uint32_t)(now-tickAt)>=1000UL){
  tickAt+=1000UL;++secondsElapsed;
  uint32_t hh=(secondsElapsed/3600UL)%24UL,mm=(secondsElapsed/60UL)%60UL,ss=secondsElapsed%60UL;
  Serial.printf("TIME %02lu:%02lu:%02lu\n",(unsigned long)hh,(unsigned long)mm,(unsigned long)ss);
 }
 uint32_t edges;
 portENTER_CRITICAL(&mux);edges=edgeBits;edgeBits=0;portEXIT_CRITICAL(&mux);
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
  Serial.printf("COUNT %lu PAUSE %s\n",(unsigned long)count,paused?"ON":"OFF");
 }
}
