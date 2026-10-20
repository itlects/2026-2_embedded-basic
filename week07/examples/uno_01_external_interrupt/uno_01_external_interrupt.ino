// UNO: D2(INT0) -> push button -> GND, 115200bps
const byte BUTTON=2;
volatile bool irqFlag=false;
void buttonISR(){ irqFlag=true; }
void setup(){
  Serial.begin(115200);
  pinMode(BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON), buttonISR, FALLING);
  Serial.println("Press D2 button");
}
void loop(){
  bool pending;
  noInterrupts(); pending=irqFlag; irqFlag=false; interrupts();
  if(pending){ Serial.println("Interrupt occurred (contact bounce may repeat)"); }
}
