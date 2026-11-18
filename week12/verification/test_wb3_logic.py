from pathlib import Path
import re, subprocess
s=Path('week12/firmware/04_iot_panel/04_iot_panel.ino').read_text()
logic=s[s.index('const char* AP_SSID'):s.index('String stateJson()')]
logic=logic.replace('WebServer server(80);','').replace('DHT dht(DHT_PIN, DHT11);','FakeDHT dht;')
pre='''#include <cstdint>
#include <cstddef>
#include <cmath>
#include <cassert>
using std::isnan;
#include <cstdio>
#define HIGH true
#define LOW false
unsigned long clockMs=0;
bool pins[40];
bool outputs[40];
unsigned long toneFreq=0;
unsigned long millis(){return clockMs;}
bool digitalRead(int p){return pins[p];}
void digitalWrite(int p,bool v){outputs[p]=v;}
unsigned long ledcWriteTone(int,unsigned long f){toneFreq=f;return f;}
struct FakeDHT {float readTemperature(){return 24;}float readHumidity(){return 52;}};
struct SerialStub {template<class... T> void printf(const char*,T...){}void println(const char*){}} Serial;
'''
test='''
void reset(){
 clockMs=0;allOff();buzzerReady=true;
 for(int i=0;i<40;i++)pins[i]=HIGH;
 for(int i=0;i<4;i++){rawButton[i]=HIGH;stableButton[i]=HIGH;changedAt[i]=0;pressedAt[i]=0;longHandled[i]=false;}
}
void tick(unsigned long t){clockMs=t;updateButtons();updateMusic();}
int main(){
 reset();pins[32]=LOW;tick(100);tick(120);assert(!ledState[0]);
 tick(131);pins[32]=HIGH;tick(200);tick(231);
 assert(ledState[0]&&outputs[16]);puts("PASS short press toggles on release");
 reset();pins[32]=LOW;tick(100);pins[32]=HIGH;tick(110);pins[32]=LOW;tick(120);tick(140);
 assert(stableButton[0]==HIGH);tick(151);assert(stableButton[0]==LOW);
 puts("PASS bounce rejected before 30ms stability");
 reset();pins[32]=LOW;tick(1000);tick(1031);tick(1831);
 assert(playingSong==1&&!ledState[0]);pins[32]=HIGH;tick(1840);tick(1871);
 assert(!ledState[0]);puts("PASS long press starts music without short press toggle");
 startMusic(2);assert(playingSong==2&&noteIndex==0&&toneFreq==330);
 stopMusic();assert(playingSong==0&&toneFreq==0);puts("PASS melody switch and stop");
 startMusic(1);size_t n=songLength;
 for(size_t i=0;i<n;i++){clockMs=noteStarted+noteDuration;updateMusic();}
 assert(playingSong==0&&toneFreq==0);puts("PASS melody finishes automatically");
 reset();startMusic(2);pins[33]=LOW;tick(20);tick(51);pins[33]=HIGH;tick(100);tick(131);
 assert(playingSong==2&&ledState[1]);puts("PASS button works while melody is playing");
 setLed(0,true);allOff();for(int i=0;i<4;i++)assert(!ledState[i]);assert(playingSong==0);
 puts("PASS all off clears LEDs and tone");
 clockMs=2500;updateSensor();assert(temperature==24&&humidity==52);puts("PASS sensor scheduler");
}
'''
Path('/tmp/wb3_logic.cpp').write_text(pre+logic+test)
subprocess.run(['g++','-std=c++17','-Wall','-Wextra','/tmp/wb3_logic.cpp','-o','/tmp/wb3_logic'],check=True)
r=subprocess.run(['/tmp/wb3_logic'],capture_output=True,text=True,check=True)
Path('week12/verification/wb3_logic_tests.txt').write_text(r.stdout)
js=re.search(r'<script>(.*?)</script>',s,re.S)[1];Path('/tmp/wb3_ui.js').write_text(js)
subprocess.run(['node','--check','/tmp/wb3_ui.js'],check=True)
print(r.stdout)
