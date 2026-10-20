// ESP32: GPIO18 -> push button -> GND, 115200bps
// ISR과 loop()가 공유하는 플래그는 portMUX 임계구역으로 보호한다(듀얼코어 대비).
const int BUTTON = 18;
volatile bool irqFlag = false;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void ARDUINO_ISR_ATTR buttonISR() {
  portENTER_CRITICAL_ISR(&mux);
  irqFlag = true;
  portEXIT_CRITICAL_ISR(&mux);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON), buttonISR, FALLING);
  Serial.println("Press GPIO18 button");
}

void loop() {
  bool pending;
  portENTER_CRITICAL(&mux);
  pending = irqFlag;
  irqFlag = false;
  portEXIT_CRITICAL(&mux);
  if (pending) Serial.println("Interrupt detected (bounce may repeat)");
}
