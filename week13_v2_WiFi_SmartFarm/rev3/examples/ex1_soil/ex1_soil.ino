// 예제 1 : 토양수분센서 (AO 아날로그 + DO 디지털) 읽기
// 보드 : ESP32 DevKitC V4 (Arduino IDE 보드 선택 : ESP32 Dev Module)

const int SOIL_AO = 34;   // 센서 모듈 AO -> GPIO34 (ADC1, 입력 전용 핀)
const int SOIL_DO = 35;   // 센서 모듈 DO -> GPIO35 (입력 전용 핀)

// 보정값 : 2단계에서 측정한 값으로 바꿉니다.
int DRY_RAW = 3500;       // 마른 흙(또는 공기 중)에서의 ADC 평균
int WET_RAW = 1500;       // 물에 적신 흙에서의 ADC 평균

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);      // ADC 결과를 0~4095 범위로 사용
  pinMode(SOIL_DO, INPUT);       // DO는 모듈에 풀업이 있으므로 INPUT
  Serial.println("Soil sensor test start");
}

int readSoilAverage() {
  long sum = 0;
  for (int i = 0; i < 10; i++) {   // 10번 읽어서 평균 -> 잡음 감소
    sum += analogRead(SOIL_AO);
    delay(5);
  }
  return sum / 10;
}

void loop() {
  int raw = readSoilAverage();
  int mv = analogReadMilliVolts(SOIL_AO);   // 보정된 전압(mV)
  int dout = digitalRead(SOIL_DO);          // HIGH=건조, LOW=습함

  int moisture = map(raw, DRY_RAW, WET_RAW, 0, 100);   // 상대 수분(%)
  moisture = constrain(moisture, 0, 100);

  Serial.printf("ADC=%4d  V=%4dmV  moisture=%3d%%  DO=%d(%s)\n",
                raw, mv, moisture, dout, dout ? "DRY" : "WET");
  delay(1000);
}
