// ESP32: GPIO18 -> push button -> GND, 115200bps
const int BUTTON=18;
volatile bool irqFlag=false;
void ARDUINO_ISR_ATTR buttonISR(){ irqFlag=true; }
void setup(){
 Serial.begin(115200);
 pinMode(BUTTON,INPUT_PULLUP);
 attachInterrupt(digitalPinToInterrupt(BUTTON),buttonISR,FALLING);
}
void loop(){
 bool pending;
 noInterrupts(); pending=irqFlag; irqFlag=false; interrupts();
 if(pending) Serial.println("Interrupt detected (bounce may repeat)");
}
