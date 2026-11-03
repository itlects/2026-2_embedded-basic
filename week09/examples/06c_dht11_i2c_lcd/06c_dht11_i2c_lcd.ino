// Week09 Example 6C: DHT11 온습도 → I2C LCD1602(PCF8574 백팩)에 표시
// BH1750이 없을 때의 I2C 대체 실습. ※ DHT11은 I2C가 아니다(고유 단일선). I2C 장치는 LCD 쪽이다.
// 필요 라이브러리: "DHT sensor library"(Adafruit) + "Adafruit Unified Sensor", "LiquidCrystal I2C"(Frank de Brabander)
// UNO : DHT11 DATA=D2,  LCD SDA=A4, SCL=A5, VCC=5V
// ESP32: DHT11 DATA=GPIO4, LCD SDA=GPIO21, SCL=GPIO22  (5V LCD는 레벨 시프터 권장 — 워크북 참고)
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#if defined(ESP32)
const int DHT_PIN = 4;
#else
const int DHT_PIN = 2;
#endif
const uint8_t LCD_ADDR = 0x27;            // 0x27 또는 0x3F — 예제 6B 스캐너로 확인

DHT dht(DHT_PIN, DHT11);
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

void setup() {
  Serial.begin(115200);
#if defined(ESP32)
  Wire.begin(21, 22);                     // SDA, SCL
#else
  Wire.begin();                           // A4, A5
#endif
  lcd.init();                             // I2C로 LCD 초기화 명령 전송
  lcd.backlight();
  lcd.print("DHT11 + I2C LCD");
  dht.begin();
  Serial.println(F("tempC,humidity"));
}

void loop() {
  static unsigned long last = 0;
  if (millis() - last < 2000) return;     // DHT11은 1초에 1번 이하로 읽는다
  last = millis();

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println(F("DHT11 read fail"));
    lcd.setCursor(0, 1); lcd.print("DHT read fail   ");
    return;
  }
  Serial.print(t, 0); Serial.print(','); Serial.println(h, 0);

  lcd.setCursor(0, 0);                    // I2C로 커서 이동 명령
  lcd.print("Temp: "); lcd.print(t, 0); lcd.print((char)223); lcd.print("C    ");
  lcd.setCursor(0, 1);
  lcd.print("Humi: "); lcd.print(h, 0); lcd.print(" %     ");
}
