#if CONFIG_IDF_TARGET_ESP32S3
const int SOIL=1,DHTPIN=4,SDA_PIN=8,SCL_PIN=9,RELAY=17,BUZZ=16;
const int LEDS[]={5,6,7,10,11}, BTNS[]={12,13,14,15};
#else
const int SOIL=34,DHTPIN=16,SDA_PIN=21,SCL_PIN=22,RELAY=23,BUZZ=27;
const int LEDS[]={4,13,14,18,19}, BTNS[]={32,33,25,26};
#endif
// HIGH turns on an external NPN open-collector driver for a verified active-LOW relay.
void setup(){digitalWrite(RELAY,LOW);pinMode(RELAY,OUTPUT);Serial.begin(115200);Serial.println("Type w for 2-second test");}
unsigned long start=0;bool running=false;
void loop(){if(Serial.available()){char c=Serial.read();if(c=='w'&&!running){digitalWrite(RELAY,HIGH);start=millis();running=true;}if(c=='s'){digitalWrite(RELAY,LOW);running=false;}}
if(running&&millis()-start>=2000){digitalWrite(RELAY,LOW);running=false;Serial.println("OFF");}}
