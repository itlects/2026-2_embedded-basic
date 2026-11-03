// Week09 Example 3: potentiometer ADC1 GPIO34 -> GPIO25 LED PWM
// analogRead(): 0~4095 정수(상대값) / analogReadMilliVolts(): 보정된 mV 값
const int POT_PIN = 34, LED_PIN = 25;
unsigned long lastRead = 0;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);                     // 0~4095 (기본값)
  analogSetPinAttenuation(POT_PIN, ADC_11db);   // 약 0.15~3.1V 측정 범위
}

void loop() {
  unsigned long now = millis();
  if (now - lastRead >= 100) {
    lastRead = now;
    int adc = analogRead(POT_PIN);
    uint32_t mv = analogReadMilliVolts(POT_PIN);
    int duty = map(adc, 0, 4095, 0, 255);
    duty = constrain(duty, 0, 255);
    analogWrite(LED_PIN, duty);                 // 8비트 PWM(LEDC)
    Serial.printf("ADC=%4d  mV=%4lu  PWM=%3d\n", adc, (unsigned long)mv, duty);
  }
}
