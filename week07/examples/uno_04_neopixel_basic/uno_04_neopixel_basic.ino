// Week 07 - 예제 4: NeoPixel(WS2812) 기본 출력 (Arduino UNO)
// 부품: WS2812 일자형 8LED(DIN=D6), 원형 12LED(DIN=D7), 외부 5V 2A 전원, 330Ω 2개, 1000µF 콘덴서
// 라이브러리: Adafruit NeoPixel — Arduino IDE 라이브러리 매니저(Ctrl+Shift+I)에서 "Adafruit NeoPixel"(제작자 Adafruit) 설치
//            자세한 방법: examples/NEOPIXEL_LIBRARY_INSTALL.md
//
// 동작: 8LED 바는 0→8칸까지 초록색으로 차오른 뒤 비워지고(게이지),
//       12LED 링은 파란 점 하나가 시계 방향으로 회전한다.
//       delay() 없이 millis()로 두 애니메이션을 동시에 진행한다.
// 주의: setPixelColor()는 버퍼만 바꾸고, show()를 호출해야 LED에 전송된다.
#include <Adafruit_NeoPixel.h>

const uint8_t BAR_PIN = 6;
const uint8_t RING_PIN = 7;
const uint8_t BAR_COUNT = 8;
const uint8_t RING_COUNT = 12;
const uint8_t BRIGHTNESS = 40;          // 0~255, 처음에는 낮게(전류 제한)

Adafruit_NeoPixel bar(BAR_COUNT, BAR_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring(RING_COUNT, RING_PIN, NEO_GRB + NEO_KHZ800);

uint32_t lastBar = 0, lastRing = 0;
uint8_t barLevel = 0;       // 0~8
uint8_t ringPos = 0;        // 0~11

void drawBar() {
  bar.clear();
  for (uint8_t i = 0; i < barLevel; i++) {
    bar.setPixelColor(i, bar.Color(0, 60, 0));          // 초록
  }
  bar.show();
}

void drawRing() {
  ring.clear();
  ring.setPixelColor(ringPos, ring.Color(0, 0, 80));             // 머리: 파랑
  ring.setPixelColor((ringPos + RING_COUNT - 1) % RING_COUNT,
                     ring.Color(0, 0, 15));                       // 꼬리: 흐린 파랑
  ring.show();
}

void setup() {
  Serial.begin(115200);
  bar.begin();
  ring.begin();
  bar.setBrightness(BRIGHTNESS);
  ring.setBrightness(BRIGHTNESS);
  drawBar();
  drawRing();
  Serial.println("UNO NeoPixel basic: bar gauge (D6) + ring spinner (D7)");
}

void loop() {
  uint32_t now = millis();

  if ((uint32_t)(now - lastBar) >= 300UL) {      // 0.3초마다 바 한 칸
    lastBar = now;
    barLevel = (barLevel + 1) % (BAR_COUNT + 1);  // 0..8 반복
    drawBar();
    Serial.print("[BAR] level=");
    Serial.println(barLevel);
  }

  if ((uint32_t)(now - lastRing) >= 100UL) {     // 0.1초마다 링 한 칸 회전
    lastRing = now;
    ringPos = (ringPos + 1) % RING_COUNT;
    drawRing();
  }
}
