const int LED_PIN=25; const int LEVELS[]={0,32,64,128,192,255}; unsigned long last=0; int i=0;
void setup(){Serial.begin(115200);pinMode(LED_PIN,OUTPUT);}
void loop(){if(millis()-last>=1000){last=millis();int pwm=LEVELS[i];analogWrite(LED_PIN,pwm);Serial.println(pwm);i=(i+1)%6;}}
