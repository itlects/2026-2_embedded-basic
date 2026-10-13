const int BUZZER=11; const int NOTES[]={262,294,330,349,392,440,494,523};unsigned long last=0;int i=0;
void setup(){Serial.begin(115200);}
void loop(){if(millis()-last>=500){last=millis();tone(BUZZER,NOTES[i],300);Serial.println(NOTES[i]);i=(i+1)%8;}}
