// Week09 Example 6E: I2C 슬레이브(UNO, 주소 0x08) — DHT11을 측정해 두었다가 마스터가 요청하면 4바이트로 보낸다
// 마스터(예제 6D)가 보내는 1바이트 명령(1=ON, 0=OFF)으로 내장 LED(D13)를 켠다.
// 결선: 마스터 A4(SDA)-슬레이브 A4, A5(SCL)-A5, GND-GND (같은 이름끼리)  / DHT11 DATA=D2, +=5V, -=GND
// 라이브러리: "DHT sensor library"(Adafruit) + "Adafruit Unified Sensor"
#include <Wire.h>
#include <DHT.h>

const uint8_t MY_ADDR = 0x08;
const int DHT_PIN = 2;
const int LED_PIN = 13;                   // A4/A5만 I2C로 쓰므로 D13은 자유
DHT dht(DHT_PIN, DHT11);

// 패킷: [0]=상태(0 정상, 1 실패) [1]=온도(°C, int8) [2]=습도(%) [3]=체크섬(0~2 합의 하위 8비트)
volatile uint8_t packet[4] = {1, 0, 0, 1};
volatile int8_t  ledCmd = -1;

void onRequestHandler() {                 // 마스터가 requestFrom() 할 때 (인터럽트 문맥) — 짧게
  uint8_t buf[4];
  for (int i = 0; i < 4; i++) buf[i] = packet[i];
  Wire.write(buf, 4);
}

void onReceiveHandler(int count) {        // 마스터가 beginTransmission~endTransmission으로 보낼 때
  while (Wire.available()) ledCmd = Wire.read();
  (void)count;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.begin();
  Wire.begin(MY_ADDR);                    // 주소를 주면 슬레이브로 시작
  Wire.onRequest(onRequestHandler);
  Wire.onReceive(onReceiveHandler);
  Serial.println(F("I2C SLAVE 0x08 (DHT11) ready"));
}

void loop() {
  static unsigned long last = 0;
  if (millis() - last >= 2000) {
    last = millis();
    float h = dht.readHumidity(), t = dht.readTemperature();
    uint8_t p[4];
    if (isnan(h) || isnan(t)) { p[0] = 1; p[1] = 0; p[2] = 0; }
    else { p[0] = 0; p[1] = (uint8_t)(int8_t)lround(t); p[2] = (uint8_t)lround(h); }
    p[3] = (uint8_t)(p[0] + p[1] + p[2]);
    noInterrupts();                       // 패킷을 바꾸는 도중 요청이 오지 않게
    for (int i = 0; i < 4; i++) packet[i] = p[i];
    interrupts();
    Serial.print(F("measure  T=")); Serial.print((int8_t)p[1]);
    Serial.print(F("C  H=")); Serial.print(p[2]); Serial.println(p[0] ? F("%  (DHT fail)") : F("%"));
  }
  if (ledCmd >= 0) {
    noInterrupts(); int8_t c = ledCmd; ledCmd = -1; interrupts();
    digitalWrite(LED_PIN, c ? HIGH : LOW);
    Serial.print(F("recv LED cmd = ")); Serial.println(c);
  }
}
