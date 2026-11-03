// Week09 Example 3: potentiometer ADC1 GPIO34 -> GPIO25 LED PWM
const int POT_PIN = 34, LED_PIN = 25;
unsigned long lastRead = 0;
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(POT_PIN, ADC_11db);
}
void loop() {
  unsigned long now = millis();
  if (now - lastRead >= 100) {
    lastRead = now;
    int adc = analogRead(POT_PIN);
    int duty = map(adc, 0, 4095, 0, 255);
    duty = constrain(duty, 0, 255);
    analogWrite(LED_PIN, duty);
    Serial.printf("ADC=%d PWM=%d\n", adc, duty);
  }
}
