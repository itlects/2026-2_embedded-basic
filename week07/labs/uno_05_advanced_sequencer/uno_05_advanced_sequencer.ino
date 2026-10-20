#include <avr/interrupt.h>
// UNO: D2,D3,D4,D5 buttons to GND; D8..D12 -> resistors -> LEDs -> GND.
// Small passive piezo buzzer D6 -> buzzer (+), GND -> buzzer (-).
const byte buttons[4] = {2,3,4,5};
const byte leds[5] = {8,9,10,11,12};
const byte buzzer = 6;
const unsigned int notes[5] = {262,330,392,523,392};
const unsigned int tempos[3] = {450,250,120};
volatile byte events = 0;
volatile byte prevD = 0;
ISR(PCINT2_vect) {
  byte current = PIND;
  byte falling = prevD & (byte)~current;
  events |= (falling >> 2) & 15;
  prevD = current;
}
bool awaiting[4] = {false,false,false,false};
unsigned long lowSince[4] = {0,0,0,0};
unsigned long lastStep = 0, toneEnd = 0;
bool running = false, toneOn = false;
byte mode = 0, speed = 1, pos = 0;

void printStatus(const char* label) {
  Serial.print("[EVENT] "); Serial.println(label);
  Serial.print("[STATE] running="); Serial.print(running ? "ON" : "OFF");
  Serial.print(" mode="); Serial.print(mode);
  Serial.print(" period="); Serial.print(tempos[speed]);
  Serial.print(" position="); Serial.println(pos);
}
void draw() {
  for (byte i = 0; i < 5; i++) {
    bool active = (mode==0 && i==pos) ||
                  (mode==1 && i<=pos) ||
                  (mode==2 && pos%2==0);
    digitalWrite(leds[i], (running && active) ? HIGH : LOW);
  }
}
void applyButton(byte i, unsigned long now) {
  if (i==0) {
    running = !running;
    lastStep = now;
    if (!running) { noTone(buzzer); toneOn=false; }
    printStatus("A RUN/PAUSE");
  } else if (i==1) {
    mode = (mode+1)%3; printStatus("B MODE");
  } else if (i==2) {
    speed = (speed+1)%3; printStatus("C SPEED");
  } else {
    running=false; mode=0; speed=1; pos=0;
    noTone(buzzer); toneOn=false;
    printStatus("D RESET");
  }
  draw();
}
void updateButtons(unsigned long now) {
  byte snapshot;
  noInterrupts(); snapshot=events; events=0; interrupts();
  for (byte i=0; i<4; i++) {
    if (snapshot & (1<<i)) { awaiting[i]=true; lowSince[i]=0; }
    if (!awaiting[i]) continue;
    if (digitalRead(buttons[i]) == HIGH) {
      lowSince[i]=0;
      continue;
    }
    if (lowSince[i]==0) lowSince[i]=now;
    if (now-lowSince[i] < 35UL) continue;
    awaiting[i]=false; lowSince[i]=0;
    applyButton(i,now);
  }
}
void updatePattern(unsigned long now) {
  if (toneOn && (long)(now-toneEnd)>=0) {
    noTone(buzzer); toneOn=false;
  }
  if (!running || now-lastStep < tempos[speed]) return;
  lastStep += tempos[speed];
  pos=(pos+1)%5;
  draw();
  tone(buzzer,notes[pos]);
  toneEnd=now+80; toneOn=true;
  Serial.print("[STEP] LED="); Serial.print(pos+1);
  Serial.print(" note="); Serial.println(notes[pos]);
}
void setup() {
  Serial.begin(115200);
  for (byte i=0; i<4; i++) pinMode(buttons[i],INPUT_PULLUP);
  for (byte i=0; i<5; i++) {
    pinMode(leds[i],OUTPUT);
    digitalWrite(leds[i],LOW);
  }
  pinMode(buzzer,OUTPUT);
  noInterrupts();
  prevD=PIND;
  PCMSK2 |= _BV(PCINT18)|_BV(PCINT19)|_BV(PCINT20)|_BV(PCINT21);
  PCIFR |= _BV(PCIF2);
  PCICR |= _BV(PCIE2);
  interrupts();
  lastStep=millis();
  Serial.println("A RUN  B MODE  C SPEED  D RESET");
  printStatus("READY");
}
void loop() {
  unsigned long now=millis();
  updateButtons(now);
  updatePattern(now);
}
