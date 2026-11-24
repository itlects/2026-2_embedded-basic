const int SOIL=34,DHTPIN=16,SDA_PIN=21,SCL_PIN=22,RELAY=23,BUZZ=27;
const int LEDS[]={4,13,14,18}, BTNS[]={32,33,25,26};
void setup(){Serial.begin(115200);analogReadResolution(12);}
void loop(){long sum=0;for(int i=0;i<16;i++)sum+=analogRead(SOIL);Serial.println(sum/16);delay(1000);}
