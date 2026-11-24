#if CONFIG_IDF_TARGET_ESP32S3
const int SOIL=1,DHTPIN=4,SDA_PIN=8,SCL_PIN=9,RELAY=17,BUZZ=16;
const int LEDS[]={5,6,7,10,11}, BTNS[]={12,13,14,15};
#else
const int SOIL=34,DHTPIN=16,SDA_PIN=21,SCL_PIN=22,RELAY=23,BUZZ=27;
const int LEDS[]={4,13,14,18,19}, BTNS[]={32,33,25,26};
#endif
void setup(){Serial.begin(115200); analogReadResolution(12);}
void loop(){int sum=0;for(int i=0;i<16;i++){sum+=analogRead(SOIL);delay(5);} Serial.println(sum/16);delay(1000);}
