#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
const int SOIL=34,DHTPIN=16,SDA_PIN=21,SCL_PIN=22,RELAY=23,BUZZ=27;
const int LEDS[]={4,13,14,18}, BTNS[]={32,33,25,26};

struct Sample {float t,h;int soil,leds,buttons,pump,mode;};
void startNetwork(){}
void sendSample(Sample s){Serial.println("sim sample");}

bool observed[4]={HIGH,HIGH,HIGH,HIGH},stable[4]={HIGH,HIGH,HIGH,HIGH};
unsigned long changed[4]={0,0,0,0};
void action(int i);
void pollButtons(){for(int i=0;i<4;i++){bool v=digitalRead(BTNS[i]);if(v!=observed[i]){observed[i]=v;changed[i]=millis();}if(v!=stable[i]&&millis()-changed[i]>=40){stable[i]=v;if(!v)action(i);}}}
int buttonMask(){int mask=0;for(int i=0;i<4;i++)if(!stable[i])mask|=1<<i;return mask;}
int ledMask(){int mask=0;for(int i=0;i<4;i++)if(digitalRead(LEDS[i]))mask|=1<<i;return mask;}

const bool CALIBRATED=true,PUMP_CONNECTED=true;
// Set measured values and CALIBRATED=true after soil experiment.
// PUMP_CONNECTED=true only after instructor supervised relay/pump test.
const int DRY=3000,WET=1200;
const unsigned long MAX_RUN=3000,REST=20000;
DHT dht(DHTPIN,DHT22);LiquidCrystal_I2C lcd(0x27,16,2);
bool pump=false,autoMode=false,valid=false;
int threshold=35,moisture=-1,raw=0;
float t=NAN,h=NAN;
unsigned long started=0,lastStop=0,lastSample=0,lastLCD=0;
void stopPump(){digitalWrite(RELAY,LOW);if(pump){pump=false;lastStop=millis();}}
void startPump(){if(PUMP_CONNECTED&&valid&&!pump&&millis()-lastStop>=REST){started=millis();pump=true;digitalWrite(RELAY,HIGH);}}
void action(int i){if(i==0){autoMode=!autoMode;stopPump();}if(i==1){autoMode=false;startPump();}if(i==2){autoMode=false;stopPump();}if(i==3)threshold=threshold>=60?20:threshold+5;}
void sample(){long sum=0;for(int i=0;i<16;i++)sum+=analogRead(SOIL);raw=sum/16;valid=CALIBRATED&&abs(DRY-WET)>100&&raw>15&&raw<4080;
moisture=valid?constrain((int)(100.0*(raw-DRY)/(WET-DRY)),0,100):-1;t=dht.readTemperature();h=dht.readHumidity();if(!valid){autoMode=false;stopPump();}}
void setup(){digitalWrite(RELAY,LOW);pinMode(RELAY,OUTPUT);Serial.begin(115200);analogReadResolution(12);for(int i=0;i<4;i++){pinMode(LEDS[i],OUTPUT);pinMode(BTNS[i],INPUT_PULLUP);}pinMode(BUZZ,OUTPUT);dht.begin();Wire.begin(SDA_PIN,SCL_PIN);lcd.init();lcd.backlight();lastStop=millis();startNetwork();}
void loop(){
  pollButtons();unsigned long now=millis();
  if(pump&&(now-started>=MAX_RUN||!valid||(autoMode&&moisture>=threshold+10)))stopPump();
  if(now-lastSample>=3000){lastSample=now;sample();if(!isnan(t)&&!isnan(h))sendSample({t,h,moisture,ledMask(),buttonMask(),pump,autoMode});Serial.printf("ADC=%d soil=%d pump=%d\n",raw,moisture,pump);}
  if(autoMode&&valid&&moisture<threshold)startPump();
  bool alarm=!valid||isnan(t)||isnan(h);
  digitalWrite(LEDS[0],true);
  digitalWrite(LEDS[1],valid&&moisture<threshold);
  digitalWrite(LEDS[2],pump);digitalWrite(LEDS[3],autoMode);
  // Short alarm beep. Keep loop free from network waits.
  if(alarm&&now%3000<120)tone(BUZZ,1200);else noTone(BUZZ);
  if(now-lastLCD>=1000){lastLCD=now;char row[17];lcd.setCursor(0,0);lcd.print("                ");lcd.setCursor(0,0);if(valid){snprintf(row,sizeof(row),"Soil:%3d%% %s",moisture,pump?"ON":"OFF");lcd.print(row);}else lcd.print("CALIBRATE FIRST");lcd.setCursor(0,1);lcd.print("                ");lcd.setCursor(0,1);if(isnan(t)||isnan(h))lcd.print("DHT error");else{snprintf(row,sizeof(row),"%4.1fC %4.1f%%",t,h);lcd.print(row);}}
}
