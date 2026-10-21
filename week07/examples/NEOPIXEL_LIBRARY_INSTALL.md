# 예제 4 준비 — Adafruit NeoPixel 라이브러리 설치 (Arduino IDE)

NeoPixel(WS2812)은 Arduino 기본 함수만으로 제어할 수 없다. 라이브러리 없이 컴파일하면
`fatal error: Adafruit_NeoPixel.h: No such file or directory` 오류가 난다. 처음 한 번만 설치하면 된다.

## Arduino IDE 2.x
1. Arduino IDE를 실행하고 인터넷 연결을 확인한다.
2. 왼쪽 세로 도구 모음의 책 모양 아이콘(**Library Manager / 라이브러리 매니저**)을 누른다.
   메뉴: **스케치(Sketch) → 라이브러리 포함하기(Include Library) → 라이브러리 관리(Manage Libraries…)**, 단축키 `Ctrl + Shift + I`.
3. 검색창에 `Adafruit NeoPixel`을 입력한다.
4. 제작자가 **Adafruit**인 "Adafruit NeoPixel" 항목에서 **설치(INSTALL)** 를 누른다(최신 버전).
   이름이 비슷한 다른 라이브러리(Adafruit DMA neopixel, NeoPixelBus 등)는 설치하지 않는다.
5. 의존 라이브러리를 묻는 창이 나오면 **모두 설치(INSTALL ALL)** 를 누른다.
6. 항목에 **INSTALLED** 표시가 나타나면 완료.

## Arduino IDE 1.8.x
**툴 → 라이브러리 관리…** 에서 같은 방법으로 검색·설치한다.

## 인터넷이 없는 실습실
배포된 `Adafruit_NeoPixel` ZIP 파일을 **스케치 → 라이브러리 포함하기 → .ZIP 라이브러리 추가…** 로 설치한다.
수동으로 넣을 때는 `문서\Arduino\libraries\Adafruit_NeoPixel\Adafruit_NeoPixel.h` 경로가 되도록 한다.

## 설치 확인
1. **파일 → 예제 → Adafruit NeoPixel** 메뉴가 보이면 설치된 것이다.
2. `uno_04_neopixel_basic.ino`(또는 ESP32용)를 열고 **확인(✓)** 으로 컴파일 → "컴파일 완료" 후 업로드.
3. ESP32는 라이브러리와 별도로 **도구 → 보드 → 보드 매니저**에서 *esp32 by Espressif Systems*를 설치하고 보드 **ESP32 Dev Module**을 선택한다.

| 증상 | 원인과 조치 |
|---|---|
| `Adafruit_NeoPixel.h: No such file or directory` | 라이브러리 미설치 → 위 순서로 설치 후 IDE 재시작 |
| 검색 결과가 나오지 않음 | 인터넷·학내 방화벽 확인, ZIP 설치 사용 |
| 컴파일은 되는데 LED가 켜지지 않음 | 배선 문제 → DIN/DOUT, 공통 GND, 외부 5V, 핀 번호 확인 |
