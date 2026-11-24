const int SOIL=34,DHTPIN=16,SDA_PIN=21,SCL_PIN=22,RELAY=23,BUZZ=27;
const int LEDS[]={4,13,14,18}, BTNS[]={32,33,25,26};
// GPIO HIGH -> NPN -> confirmed active-LOW relay. Pump disconnected first.
bool running=false;unsigned long started=0;
void setup(){digitalWrite(RELAY,LOW);pinMode(RELAY,OUTPUT);Serial.begin(115200);Serial.println("w=2s, s=stop");}
void loop(){if(Serial.available()){char c=Serial.read();if(c=='w'&&!running){started=millis();running=true;digitalWrite(RELAY,HIGH);}if(c=='s'){running=false;digitalWrite(RELAY,LOW);}}
if(running&&millis()-started>=2000){digitalWrite(RELAY,LOW);running=false;Serial.println("OFF");}}
