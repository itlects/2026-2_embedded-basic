/*
 * 예제 6-3  LEDC 타이머로 소리 만들기  —  ESP32 DevKitC V4
 * ------------------------------------------------------------------
 * 회로 : GPIO23 → 수동부저(+) ,  수동부저(-) → GND
 *        GPIO13 ↔ 푸시버튼 ↔ GND                        (옥타브 전환)
 *
 * 원리 : 소리의 높이 = 신호의 주파수. 듀티는 50%로 고정하고 주파수만 바꾼다.
 *        ledcWriteTone(pin, f) 는 LEDC 타이머의 분주비를 계산해 f Hz, 듀티 50% 를 만든다.
 *        반환값은 "실제로 설정된 주파수"이다 → 목표값과 비교해 본다.
 *
 * 동작 : 도~높은 도를 400 ms 울림 + 100 ms 쉼으로 반복 (delay() 없이 millis 상태 머신)
 *        버튼을 누르면 한 옥타브 위(주파수 × 2)로 전환
 *
 * 코어 : Arduino-ESP32 3.x (ledcAttach / ledcWriteTone)
 */

const int BUZZER_PIN = 23;
const int BTN_PIN    = 13;

const uint32_t NOTES[] = {262, 294, 330, 349, 392, 440, 494, 523};
const char* NAMES[]    = {"Do", "Re", "Mi", "Fa", "Sol", "La", "Si", "Do'"};
const int NOTE_COUNT = sizeof(NOTES) / sizeof(NOTES[0]);

const unsigned long ON_MS  = 400;
const unsigned long OFF_MS = 100;

int  noteNo  = 0;
int  octave  = 1;
bool soundOn = false;
unsigned long stateMs = 0;

const unsigned long DEBOUNCE_MS = 30;
int lastReading = HIGH, stableState = HIGH;
unsigned long lastChangeMs = 0;

bool buttonPressed() {
  int reading = digitalRead(BTN_PIN);
  if (reading != lastReading) { lastChangeMs = millis(); lastReading = reading; }
  if (millis() - lastChangeMs >= DEBOUNCE_MS && reading != stableState) {
    stableState = reading;
    return stableState == LOW;
  }
  return false;
}

void playNote() {
  uint32_t target = NOTES[noteNo] * octave;
  uint32_t real   = ledcWriteTone(BUZZER_PIN, target);   // 실제 설정 주파수
  Serial.printf("%-4s target=%luHz  real=%luHz\n", NAMES[noteNo],
                (unsigned long)target, (unsigned long)real);
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);
  ledcAttach(BUZZER_PIN, 1000, 10);     // 초기 1 kHz, 10-bit (주파수는 곧 바뀜)
  Serial.println("Ex6-3 LEDC tone (ESP32)");
  playNote();
  soundOn = true;
  stateMs = millis();
}

void loop() {
  if (buttonPressed()) {
    octave = (octave == 1) ? 2 : 1;
    Serial.printf("Octave x%d\n", octave);
  }

  unsigned long now = millis();
  if (soundOn && now - stateMs >= ON_MS) {
    ledcWriteTone(BUZZER_PIN, 0);       // 0 Hz → 출력 정지
    soundOn = false;
    stateMs = now;
  } else if (!soundOn && now - stateMs >= OFF_MS) {
    noteNo = (noteNo + 1) % NOTE_COUNT;
    playNote();
    soundOn = true;
    stateMs = now;
  }
}
