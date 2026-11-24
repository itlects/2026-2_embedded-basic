#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#if CONFIG_IDF_TARGET_ESP32S3
const int SOIL=1,DHTPIN=4,SDA_PIN=8,SCL_PIN=9,RELAY=17,BUZZ=16;
const int LEDS[]={5,6,7,10,11}, BTNS[]={12,13,14,15};
#else
const int SOIL=34,DHTPIN=16,SDA_PIN=21,SCL_PIN=22,RELAY=23,BUZZ=27;
const int LEDS[]={4,13,14,18,19}, BTNS[]={32,33,25,26};
#endif
DHT dht(DHTPIN,DHT22);
LiquidCrystal_I2C lcd(0x27,16,2);
WebServer server(80);
// Replace with classroom measurements AFTER dry/wet calibration.
const int DRY=3000,WET=1200;
const bool CALIBRATED=false; // set true only after calibration
const bool PUMP_CONNECTED=false; // true only after supervised relay-only and pump tests
int raw=0, moisture=0, threshold=35;
float temp=NAN,hum=NAN;
bool autoMode=false,pump=false,valid=false,alarm=false;
unsigned long pumpStart=0,lastStop=0,lastSample=0,lastLCD=0;
const unsigned long MAX_RUN=3000,COOLDOWN=20000;
bool stable[4]={HIGH,HIGH,HIGH,HIGH},observed[4]={HIGH,HIGH,HIGH,HIGH};
unsigned long changed[4]={0,0,0,0};
void stopPump(){if(pump){digitalWrite(RELAY,LOW);pump=false;lastStop=millis();}}
void startPump(){if(PUMP_CONNECTED&&valid&&!pump&&millis()-lastStop>=COOLDOWN){pumpStart=millis();digitalWrite(RELAY,HIGH);pump=true;}}
void readSensors(){
 long sum=0;for(int i=0;i<16;i++)sum+=analogRead(SOIL);raw=sum/16;
 valid=CALIBRATED&&abs(DRY-WET)>100&&raw>15&&raw<4080;
 if(valid)moisture=constrain((int)(100.0*(raw-DRY)/(WET-DRY)),0,100);
 temp=dht.readTemperature();hum=dht.readHumidity();
 if(!valid)stopPump();
 alarm=!valid||isnan(temp)||isnan(hum);
}
void action(int i){switch(i){case 0:autoMode=!autoMode;stopPump();break;case 1:autoMode=false;startPump();break;case 2:autoMode=false;stopPump();break;case 3:threshold=(threshold>=60?20:threshold+5);break;}}
void buttons(){for(int i=0;i<4;i++){bool v=digitalRead(BTNS[i]);if(v!=observed[i]){observed[i]=v;changed[i]=millis();}if(v!=stable[i]&&millis()-changed[i]>=40){stable[i]=v;if(!v)action(i);}}}
void page(){
 String s="<meta charset='utf-8'><meta name='viewport' content='width=device-width'><meta http-equiv='refresh' content='3'><h1>Smart Farm</h1><p>Soil raw: "+String(raw)+" / moisture: "+(valid?String(moisture)+"%":"UNCALIBRATED")+"</p><p>Temperature: "+String(temp)+" C / RH: "+String(hum)+"%</p><p>Pump: "+String(pump?"ON":"OFF")+" / Mode: "+String(autoMode?"AUTO":"MANUAL")+" / threshold: "+String(threshold)+"%</p>";
 s+="<form method='POST' action='/cmd'><button name='b' value='0'>Toggle AUTO</button><button name='b' value='1'>Water 3s</button><button name='b' value='2'>STOP</button><button name='b' value='3'>Threshold +5</button></form>";
 server.send(200,"text/html; charset=utf-8",s);
}
void setup(){
 digitalWrite(RELAY,LOW);pinMode(RELAY,OUTPUT);
 for(int p:LEDS){pinMode(p,OUTPUT);digitalWrite(p,LOW);}for(int p:BTNS)pinMode(p,INPUT_PULLUP);pinMode(BUZZ,OUTPUT);
 Serial.begin(115200);analogReadResolution(12);Wire.begin(SDA_PIN,SCL_PIN);lcd.init();lcd.backlight();dht.begin();
 WiFi.mode(WIFI_AP);WiFi.softAP("Farm-Team01","farmclass13");Serial.println(WiFi.softAPIP());
 server.on("/",HTTP_GET,page);server.on("/cmd",HTTP_POST,[]{String b=server.arg("b");if(b=="0"||b=="1"||b=="2"||b=="3")action(b.toInt());server.sendHeader("Location","/");server.send(303);});server.begin();
 lastStop=millis();readSensors();
}
void loop(){
 server.handleClient();buttons();unsigned long now=millis();
 if(now-lastSample>=2500){lastSample=now;readSensors();Serial.printf("raw=%d soil=%d valid=%d pump=%d\n",raw,moisture,valid,pump);}
 if(pump&&(now-pumpStart>=MAX_RUN||!valid||(autoMode&&moisture>=threshold+10)))stopPump();
 if(autoMode&&valid&&moisture<threshold)startPump();
 digitalWrite(LEDS[0],WiFi.softAPgetStationNum()>0);digitalWrite(LEDS[1],valid&&moisture<threshold);digitalWrite(LEDS[2],pump);digitalWrite(LEDS[3],autoMode);digitalWrite(LEDS[4],alarm);
 // brief intermittent alarm; passive piezo only
 if(alarm&&now%3000<120)tone(BUZZ,1200);else noTone(BUZZ);
 if(now-lastLCD>=1000){lastLCD=now; char line[17];lcd.setCursor(0,0);if(valid)snprintf(line,sizeof(line),"Soil:%3d%% %4s",moisture,pump?"PUMP":"OFF");else snprintf(line,sizeof(line),"CALIBRATE FIRST ");lcd.print("                ");lcd.setCursor(0,0);lcd.print(line);lcd.setCursor(0,1);lcd.print("                ");lcd.setCursor(0,1);if(isnan(temp)||isnan(hum))lcd.print("DHT error");else{snprintf(line,sizeof(line),"%4.1fC %4.1f%%",temp,hum);lcd.print(line);}}
}
