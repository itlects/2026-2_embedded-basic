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

// ---- LED 상태 (버튼1~3이 정한다)
enum LedMode { STOP, FORWARD, REVERSE, BLINK };
LedMode ledMode = STOP;
bool paused = false;                    // 같은 버튼을 다시 누르면 일시정지/재개
const unsigned long intervalMs = 250;   // LED 단계 간격
unsigned long lastStepTime = 0;
int stepIndex = 0;

// ---- 멜로디 상태 (버튼4가 켜고 끈다)
bool melodyOn = false;
// 4음 이상, 각 음 200~300ms, 음 사이 30~50ms 무음, 끌 때까지 반복
const unsigned int melodyNotes[] = {262, 294, 330, 349, 392};   // 원하는 음으로 변경 가능
const int melodyLength = sizeof(melodyNotes) / sizeof(melodyNotes[0]);
const unsigned long noteMs = 250;
const unsigned long gapMs  = 40;
int noteIndex = 0;

// ---- 디바운싱 (버튼별)
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
  Serial.println("[STATE] led=STOP paused=0 melody=OFF");
}

void loop() {
  unsigned long now = millis();

  // TODO 1: 버튼 4개의 눌림(HIGH→LOW)을 디바운싱하여 한 번 누름당 한 번만 인식
  //         인식할 때마다 Serial에 "[EVENT] B1" 형식으로 한 줄 출력
  // TODO 2: 버튼1~3 → ledMode 선택(같은 버튼이면 paused 토글), 버튼4 → melodyOn 토글
  //         바뀔 때마다 "[STATE] led=... paused=... melody=..." 한 줄 출력
  // TODO 3: FORWARD, REVERSE, BLINK를 now와 lastStepTime으로 구현 (paused면 진행 멈춤)
  // TODO 4: melodyOn이면 음 재생 → 무음 → 다음 음, 마지막 음 다음은 처음 음(반복)
  //         버튼4로 끄면 noTone() + ledMode=STOP + 모든 LED 끄기
  // 주의: loop()에서 실행되는 코드에는 delay()를 사용하지 않는다.
}
