// Week09 Example 7C: SPI 보드 간 통신 — 슬레이브 (Arduino UNO 전용, ATmega328P 레지스터 사용)
// 마스터(예제 7B)가 보낸 글자를 SPI 인터럽트로 받아 시리얼로 출력하고,
// 다음 전송 때 돌려줄 응답(받은 글자의 소문자)을 SPDR에 미리 넣어 둔다.
// 결선: D10(SS), D11(MOSI), D12(MISO), D13(SCK), GND — 마스터와 '같은 이름끼리' 연결
#include <SPI.h>
#if !defined(__AVR__)
#error "This slave example uses AVR registers (SPCR/SPDR). Upload it to an Arduino UNO."
#endif

volatile uint8_t rxByte = 0;
volatile bool    rxReady = false;

void setup() {
  Serial.begin(115200);
  pinMode(MISO, OUTPUT);        // 슬레이브는 MISO만 출력 (SCK·MOSI·SS는 입력)
  SPCR |= _BV(SPE);             // SPE=1: SPI 켜기. MSTR=0 이므로 슬레이브 모드
  SPDR = '-';                   // 첫 전송 때 마스터가 받을 응답
  SPI.attachInterrupt();        // SPIE=1: 1바이트 수신 완료 시 ISR(SPI_STC_vect) 실행
  Serial.println(F("SPI SLAVE ready (UNO)"));
}

// 8비트가 모두 들어오면(Serial Transfer Complete) 호출 — 짧게!
ISR(SPI_STC_vect) {
  uint8_t b = SPDR;                                     // 받은 바이트
  rxByte = b;
  rxReady = true;
  SPDR = (b >= 'A' && b <= 'Z') ? (b + ('a' - 'A')) : '?';   // 다음 전송에 보낼 응답 준비
}

void loop() {
  if (rxReady) {                // ISR에서는 Serial을 쓰지 않고, 플래그로 loop에 알린다
    noInterrupts();
    uint8_t b = rxByte;
    rxReady = false;
    interrupts();
    Serial.print(F("Receive : ")); Serial.print((char)b);
    Serial.print(F("   (next reply : "));
    Serial.print((char)((b >= 'A' && b <= 'Z') ? b + ('a' - 'A') : '?'));
    Serial.println(')');
  }
}
