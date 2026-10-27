// 2026-2 임베디드프로세서기초 중간고사 1차 수행평가 시작 코드 (정답 아님)
// 제출 파일명: midterm_학번_이름.ino
// UNO와 ESP32 DevKitC V4(esp32 3.x 코어) 핀은 아래 조건부 컴파일로 구분한다.

#if defined(ARDUINO_ARCH_ESP32)
const int ledPins[4]    = {16, 17, 18, 19};
const int buttonPins[4] = {21, 22, 23, 25};
const int buzzerPin     = 26;
#else
const int ledPins[4]    = {3, 4, 5, 6};
const int buttonPins[4] = {7, 8, 9, 10};
const int buzzerPin     = 11;
#endif

enum Mode { STOP, FORWARD, REVERSE, BLINK, MELODY };
Mode mode = STOP;
bool paused = false;              // 같은 버튼 재입력 시 일시정지/재개

const unsigned long intervalMs = 250;   // LED 단계 간격
unsigned long lastStepTime = 0;
int stepIndex = 0;

// 멜로디: 4음 이상, 각 음 200~300ms, 음 사이 30~50ms 무음
const unsigned int melodyNotes[] = {262, 294, 330, 349, 392};   // 원하는 음으로 변경 가능
const int melodyLength = sizeof(melodyNotes) / sizeof(melodyNotes[0]);
const unsigned long noteMs = 250;
const unsigned long gapMs  = 40;
int noteIndex = 0;

// 디바운싱용 변수 (버튼별)
const unsigned long debounceMs = 30;

void allLedsOff() {
  for (int i = 0; i < 4; i++) digitalWrite(ledPins[i], LOW);
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(buzzerPin, OUTPUT);
  allLedsOff();
  Serial.println("[STATE] mode=STOP paused=0");
}

void loop() {
  unsigned long now = millis();

  // TODO 1: 버튼 4개의 눌림(HIGH→LOW)을 디바운싱하여 한 번 누름당 한 번만 인식
  //         인식할 때마다 Serial에 "[EVENT] B1" 형식으로 한 줄 출력
  // TODO 2: 상태표에 따라 mode / paused 변경, 변경 시 "[STATE] mode=... paused=..." 출력
  //         모드가 바뀌면 LED 소등·noTone() 후 새 모드 시작
  // TODO 3: FORWARD, REVERSE, BLINK 모드를 now와 lastStepTime으로 구현 (paused면 진행 멈춤)
  // TODO 4: MELODY 모드: 음 재생 → 무음 구간 → 다음 음, 마지막 음 후 STOP
  // 주의: loop()에서 실행되는 코드에는 delay()를 사용하지 않는다.
}
