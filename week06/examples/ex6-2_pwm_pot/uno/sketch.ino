const int LED=9, ADC=A0; unsigned long last=0;
void setup(){Serial.begin(115200);pinMode(LED,OUTPUT);}
void loop(){if(millis()-last>=20){last=millis();int a=analogRead(ADC);int p=map(a,0,1023,0,255);analogWrite(LED,p);Serial.print("ADC=");Serial.print(a);Serial.print(" PWM=");Serial.println(p);}}
