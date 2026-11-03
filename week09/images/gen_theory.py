# -*- coding: utf-8 -*-
"""9주차 이론 그림: 핀맵, GPIO, 디바운스, ADC, PWM, 부품, 전원 안전. 사용: python3 gen_theory.py OUTDIR"""
import sys, os, math
from figlib import *

OUT = sys.argv[1] if len(sys.argv) > 1 else "."

# ================================================================ 1. 핀맵
# (이름, GPIO번호 or None, 기능, 분류)  분류: pwr, gnd, en, in(입력전용), flash, strap, uart, io
J2 = [("3V3", None, "3.3V 전원", "pwr"), ("EN", None, "리셋(CHIP_PU)", "en"),
      ("VP", 36, "ADC1_CH0 · 입력 전용", "in"), ("VN", 39, "ADC1_CH3 · 입력 전용", "in"),
      ("IO34", 34, "ADC1_CH6 · 입력 전용", "in"), ("IO35", 35, "ADC1_CH7 · 입력 전용", "in"),
      ("IO32", 32, "ADC1_CH4 · TOUCH9", "io"), ("IO33", 33, "ADC1_CH5 · TOUCH8", "io"),
      ("IO25", 25, "ADC2_CH8 · DAC1", "io"), ("IO26", 26, "ADC2_CH9 · DAC2", "io"),
      ("IO27", 27, "ADC2_CH7 · TOUCH7", "io"), ("IO14", 14, "ADC2_CH6 · TOUCH6", "io"),
      ("IO12", 12, "ADC2_CH5 · 부팅 전압 스트랩", "strap"), ("GND", None, "접지", "gnd"),
      ("IO13", 13, "ADC2_CH4 · TOUCH4", "io"), ("D2", 9, "SPI 플래시", "flash"),
      ("D3", 10, "SPI 플래시", "flash"), ("CMD", 11, "SPI 플래시", "flash"), ("5V", None, "5V 전원(USB)", "pwr5")]
J3 = [("GND", None, "접지", "gnd"), ("IO23", 23, "VSPI MOSI", "io"), ("IO22", 22, "I2C SCL(관례)", "io"),
      ("TX", 1, "UART0 TX · 업로드/시리얼", "uart"), ("RX", 3, "UART0 RX · 업로드/시리얼", "uart"),
      ("IO21", 21, "I2C SDA(관례)", "io"), ("GND", None, "접지", "gnd"), ("IO19", 19, "VSPI MISO", "io"),
      ("IO18", 18, "VSPI SCK", "io"), ("IO5", 5, "부팅 스트랩", "strap"), ("IO17", 17, "WROVER는 내부 사용", "warn"),
      ("IO16", 16, "WROVER는 내부 사용", "warn"), ("IO4", 4, "ADC2_CH0 · TOUCH0", "io"),
      ("IO0", 0, "BOOT 버튼 · 부팅 스트랩", "strap"), ("IO2", 2, "부팅 스트랩", "strap"),
      ("IO15", 15, "부팅 스트랩", "strap"), ("D1", 8, "SPI 플래시", "flash"), ("D0", 7, "SPI 플래시", "flash"),
      ("CLK", 6, "SPI 플래시", "flash")]
USE = {"3V3": ("가변저항 1번", C["v33"]), 34: ("가변저항 가운데", C["teal"]), 25: ("LED1 (PWM)", C["red"]),
       26: ("LED2", C["green"]), 27: ("수동 부저", C["purple"]), 18: ("버튼 RUN", "#b8322f"),
       19: ("버튼 MODE", "#1b6e48"), "GND": ("공통 GND", "#222")}
CAT = {"pwr": ("#fde3e3", "#c63a3a"), "pwr5": ("#ffd6d6", "#a40000"), "gnd": ("#e2e2e2", "#222"),
       "en": ("#e8e8ff", "#4b4bb3"), "in": ("#fff3c4", "#a07800"), "flash": ("#d9d9d9", "#777"),
       "strap": ("#ffe2c4", "#c06000"), "uart": ("#efe1ff", "#7b4bb3"), "io": ("#ffffff", "#8899aa"),
       "warn": ("#f4f4f4", "#999")}


def pinout():
    W, Hh = 1460, 1160
    o = []
    title(o, "ESP32-DevKitC V4 핀맵 (J2 · J3 헤더) — 9주차 사용 핀",
          "윗면에서 본 그림 · 안테나가 위, Micro-USB가 아래 · 코드에는 IO 뒤의 숫자(GPIO 번호)를 쓴다 — 헤더 순번(1~19)과 다르다")
    bx, bw, by, bh = 560, 340, 130, 960
    o.append(rect(bx, by, bw, bh, "#1f2a36", "#0e151d", 3, 18))
    o.append(rect(bx + 50, by + 18, bw - 100, 270, "#c9ccd1", "#888", 2, 6))
    o.append(rect(bx + 50, by + 18, bw - 100, 60, "#9aa0a8", "#888", 2, 6))
    o.append(T(bx + bw / 2, by + 56, "PCB 안테나", 16, "bold", "#333", "middle"))
    o.append(T(bx + bw / 2, by + 170, "ESP32-WROOM-32", 18, "bold", "#222", "middle"))
    o.append(T(bx + bw / 2, by + 196, "(모듈 실크 확인)", 14, fill="#333", anchor="middle"))
    o.append(T(bx + bw / 2, by + 230, "ESP32 칩 + 4MB 플래시", 14, fill="#333", anchor="middle"))
    o.append(rect(bx + 110, by + 560, 120, 70, "#333", "#555", 2, 6))
    o.append(T(bx + bw / 2, by + 590, "USB-UART", 14, "bold", "white", "middle"))
    o.append(T(bx + bw / 2, by + 612, "브리지", 14, "bold", "white", "middle"))
    o.append(rect(bx + 120, by + 430, 100, 60, "#333", "#555", 2, 6))
    o.append(T(bx + bw / 2, by + 466, "3.3V LDO", 14, "bold", "white", "middle"))
    for lx, lab in ((bx + 40, "EN"), (bx + bw - 100, "BOOT")):
        o.append(rect(lx, by + 830, 60, 46, "#ddd", "#555", 2, 6)); o.append(circle(lx + 30, by + 853, 13, "#555"))
        o.append(T(lx + 30, by + 900, lab, 15, "bold", "white", "middle"))
    o.append(rect(bx + 120, by + bh - 40, 100, 52, "#b8bcc2", "#666", 2, 4))
    o.append(T(bx + bw / 2, by + bh + 36, "Micro-USB (전원 + 업로드 + 시리얼)", 16, "bold", C["ink"], "middle"))
    o.append(T(bx + 40, by + 320, "J2", 22, "bold", "white")); o.append(T(bx + bw - 40, by + 320, "J3", 22, "bold", "white", "end"))
    y0, dy = by + 40, 48
    for side, pins in (("L", J2), ("R", J3)):
        for i, (name, g, func, cat) in enumerate(pins):
            y = y0 + i * dy
            fill, stroke = CAT[cat]
            px = bx + 8 if side == "L" else bx + bw - 26
            o.append(rect(px, y - 9, 18, 18, "#d4af37", "#7a5d00", 1.5, 3))
            key = g if g is not None else name
            used = USE.get(key)
            if side == "L":
                x0 = bx - 330
                o.append(rect(x0, y - 18, 300, 36, fill, stroke, 2 if not used else 3.5, 6))
                o.append(T(x0 + 10, y + 6, f"{i+1:>2}", 13, fill="#888"))
                o.append(T(x0 + 36, y + 6, name, 16, "bold", "#111"))
                o.append(T(x0 + 100, y + 6, func, 13, fill="#333"))
                o.append(line([(x0 + 300, y), (px, y)], stroke, 2))
                if used:
                    o += pill(x0 - 90, y + 5, used[0], used[1], 13, "#fff")
                    o.append(rect(x0, y - 18, 300, 36, "none", used[1], 4, 6))
            else:
                x0 = bx + bw + 30
                o.append(rect(x0, y - 18, 300, 36, fill, stroke, 2, 6))
                o.append(T(x0 + 290, y + 6, f"{i+1}", 13, fill="#888", anchor="end"))
                o.append(T(x0 + 10, y + 6, name, 16, "bold", "#111"))
                o.append(T(x0 + 70, y + 6, func, 13, fill="#333"))
                o.append(line([(px + 18, y), (x0, y)], stroke, 2))
                if used:
                    o += pill(x0 + 390, y + 5, used[0], used[1], 13, "#fff")
                    o.append(rect(x0, y - 18, 300, 36, "none", used[1], 4, 6))
            if cat == "flash":
                xx = bx - 330 if side == "L" else bx + bw + 30
                o.append(line([(xx + 8, y), (xx + 292, y)], "#777", 2))
    # 범례
    ly = by + bh + 70
    leg = [("io", "일반 GPIO"), ("in", "입력 전용(34~39, 풀업 없음)"), ("strap", "부팅 스트랩 — 피함"),
           ("uart", "UART0 — 업로드용"), ("flash", "SPI 플래시 — 사용 금지"), ("pwr", "전원"), ("gnd", "GND")]
    x = 40
    for k, s in leg:
        f, st = CAT[k]
        o.append(rect(x, ly - 16, 26, 22, f, st, 2, 4)); o.append(T(x + 34, ly + 1, s, 14)); x += 34 + len(s) * 13 + 30
    o.append(T(40, ly + 34, "굵은 테두리 = 9주차 사용 핀.  GPIO16/17은 WROOM 모듈에서만 사용 가능(WROVER는 PSRAM용).  핀 기능 출처: Espressif ESP32-DevKitC V4 User Guide J2/J3 표.", 14, fill="#555"))
    return "t1_devkitc_v4_pinout_ko.svg", svg(W, ly + 56, o)


# ================================================================ 2. GPIO 원리
def gpio():
    W, Hh = 1240, 760
    o = []
    title(o, "GPIO 원리 — 디지털 출력(LED)과 디지털 입력(버튼 · 풀업)",
          "GPIO 하나는 코드(pinMode)로 출력 또는 입력이 된다. 출력은 핀 전압을 3.3V/0V로 만들고, 입력은 핀 전압이 HIGH인지 LOW인지 읽는다.")
    # (a) 출력
    card(o, 40, 110, 560, 610, "① 출력: digitalWrite(25, HIGH)", C["red"])
    o.append(rect(70, 180, 120, 70, C["board"], C["boardS"], 2, 8)); o.append(T(130, 222, "GPIO25", 17, "bold", "white", "middle"))
    o.append(T(130, 270, "HIGH = 3.3V", 14, "bold", C["red"], "middle"))
    o.append(line([(190, 215), (260, 215)], C["red"], 4))
    o += sym_resistor_h(260, 350, 215, "220Ω")
    o.append(line([(350, 215), (400, 215)], C["red"], 4))
    o += sym_led_h(400, 215, C["red"])
    o.append(line([(432, 215), (500, 215), (500, 250)], "#333", 3)); o += sym_gnd(500, 250)
    o += arrow(250, 180, 420, 180, "#c62828", 3)
    o.append(T(335, 170, "전류 I", 14, "bold", "#c62828", "middle"))
    rows = [("LED 색", "Vf(대략)", "220Ω 전류", "330Ω 전류"),
            ("빨강·노랑", "1.8~2.1V", "≈ 6mA", "≈ 4mA"),
            ("초록(일반)", "2.0~2.2V", "≈ 5mA", "≈ 3.5mA"),
            ("파랑·흰색", "2.8~3.2V", "≈ 0.5~2mA", "매우 어두움")]
    yy = 330
    o.append(T(70, yy - 14, "I = (3.3V − Vf) ÷ R", 18, "bold", C["ink"]))
    for i, r in enumerate(rows):
        y = yy + 14 + i * 34
        if i == 0: o.append(rect(66, y - 22, 512, 32, "#eef2f7", "none", 0, 4))
        for j, s in enumerate(r):
            o.append(T(80 + j * 128, y, s, 14, "bold" if i == 0 else "normal"))
    o.append(T(70, 510, "• 3.3V 보드에서는 빨강·노랑·초록 LED를 권장한다.", 15))
    o.append(T(70, 538, "  파랑/흰 LED는 Vf가 3V 근처라 거의 켜지지 않을 수 있다.", 15))
    o.append(T(70, 572, "• 저항이 없으면 전류가 GPIO 한계(수십 mA)까지 흘러", 15))
    o.append(T(70, 600, "  LED·GPIO가 손상될 수 있다. LED 1개당 저항 1개.", 15))
    o.append(T(70, 640, "• digitalWrite(25, LOW) → 0V → 전류 0 → LED 꺼짐", 15, "bold", "#333"))
    o.append(T(70, 674, "• 권장: 핀당 10mA 이하로 설계 (구동 세기 기본값 기준)", 15))
    # (b) 입력
    card(o, 630, 110, 570, 610, "② 입력: pinMode(18, INPUT_PULLUP)", C["blue"])
    def pullup_panel(x, y, pressed):
        nonlocal o
        o.append(T(x, y, "버튼 누름 → LOW (0)" if pressed else "버튼 놓음 → HIGH (1)", 16, "bold", "#c62828" if pressed else C["green"]))
        o.append(T(x + 10, y + 40, "3.3V", 14, "bold", C["red"]))
        o.append(line([(x + 30, y + 46), (x + 30, y + 60)], C["red"], 3))
        o += sym_resistor_v(x + 30, y + 60, y + 150, "내부 풀업 ≈45kΩ")
        o.append(line([(x + 30, y + 150), (x + 30, y + 170)], "#333", 3))
        o.append(circle(x + 30, y + 170, 5, "#333"))
        o.append(line([(x + 30, y + 170), (x + 150, y + 170)], "#333", 3))
        o.append(T(x + 150, y + 160, "GPIO18 읽기", 14, "bold", C["blue"]))
        o.append(T(x + 150, y + 188, "= LOW" if pressed else "= HIGH", 16, "bold", "#c62828" if pressed else C["green"]))
        o.append(line([(x + 30, y + 170), (x + 30, y + 195)], "#333", 3))
        # 세로 스위치
        if pressed:
            o.append(line([(x + 30, y + 195), (x + 30, y + 245)], "#333", 4))
        else:
            o.append(line([(x + 30, y + 195), (x + 52, y + 235)], "#333", 4))
        o.append(circle(x + 30, y + 195, 5, "#333")); o.append(circle(x + 30, y + 245, 5, "#333"))
        o.append(T(x + 62, y + 225, "버튼", 14, fill="#555"))
        o.append(line([(x + 30, y + 245), (x + 30, y + 260)], "#333", 3)); o += sym_gnd(x + 30, y + 260)
        if pressed:
            o += arrow(x - 10, y + 70, x - 10, y + 260, "#c62828", 2.5)
            o.append(T(x - 18, y + 300, "≈73µA만 흐름", 13, "bold", "#c62828"))
    pullup_panel(660, 180, False)
    pullup_panel(930, 180, True)
    o.append(T(660, 530, "• 풀업 저항이 핀을 3.3V 쪽으로 '당겨' 두므로 평소 HIGH.", 15))
    o.append(T(660, 560, "• 버튼이 GND와 연결되면 LOW → 코드: digitalRead(18) == LOW", 15))
    o.append(T(660, 594, "• 풀업이 없으면(INPUT) 핀이 떠서(floating) 손만 대도", 15, "bold", "#c06000"))
    o.append(T(660, 622, "  HIGH/LOW가 무작위로 바뀐다.", 15, "bold", "#c06000"))
    o.append(T(660, 656, "• GPIO34~39는 내부 풀업이 없다 → 버튼에 쓰려면 외부 10kΩ.", 15))
    o.append(T(660, 690, "  (9주차 버튼은 GPIO18·19 사용)", 15, fill="#555"))
    return "t2_gpio_principle_ko.svg", svg(W, Hh, o)


# ================================================================ 3. 디바운스
def debounce():
    W, Hh = 1240, 600
    o = []
    title(o, "버튼 바운싱(채터링)과 millis() 디바운스",
          "기계식 접점은 누르는 순간 수 ms 동안 여러 번 붙었다 떨어진다. 코드는 '같은 값이 25ms 이상 유지될 때'만 진짜 변화로 인정한다.")
    x0, x1 = 160, 1180
    def wave(y, pts, color, lab):
        nonlocal o
        o.append(T(40, y + 6, lab, 16, "bold", color))
        o.append(line([(x0, y - 40), (x1, y - 40)], "#ddd", 1)); o.append(line([(x0, y + 40), (x1, y + 40)], "#ddd", 1))
        o.append(T(x0 - 10, y - 35, "HIGH", 12, fill="#888", anchor="end")); o.append(T(x0 - 10, y + 45, "LOW", 12, fill="#888", anchor="end"))
        p = []
        for (t, v) in pts:
            yy = y - 40 if v else y + 40
            if p: p.append((t, p[-1][1]))
            p.append((t, yy))
        p.append((x1, p[-1][1]))
        o.append(line(p, color, 3.5, cap="butt"))
    raw = [(x0, 1), (300, 0), (312, 1), (322, 0), (340, 1), (348, 0), (372, 1), (380, 0), (700, 1), (712, 0), (722, 1), (734, 0), (742, 1)]
    wave(170, raw, "#c06000", "원 신호 raw")
    deb = [(x0, 1), (380 + 25 * 4, 0), (742 + 100, 1)]
    wave(330, deb, C["blue"], "판정 stable")
    for a, b in ((300, 380), (700, 742)):
        o.append(rect(a, 115, b - a, 110, "#ffefe0", "none", 0, 0, ' fill-opacity="0.6"'))
    o.append(T(340, 112, "바운싱 수 ms", 13, "bold", "#c06000", "middle"))
    o.append(T(721, 112, "놓을 때도 바운싱", 13, "bold", "#c06000", "middle"))
    for a, txt in ((380, "마지막 변화 후 25ms"), (742, "마지막 변화 후 25ms")):
        o += arrow(a, 260, a + 100, 260, C["blue"], 2.5)
        o.append(T(a + 50, 252, txt, 12, "bold", C["blue"], "middle"))
        o.append(line([(a, 120), (a, 380)], C["blue"], 1.5, "4 4")); o.append(line([(a + 100, 120), (a + 100, 380)], C["blue"], 1.5, "4 4"))
    o.append(T(480, 400, "▲ 눌림 1회 확정 → LED 토글", 14, "bold", "#c62828", "middle"))
    o.append(T(842, 400, "▲ 놓임 확정 (동작 없음)", 14, "bold", "#555", "middle"))
    o.append(rect(40, 430, 1160, 140, "#f3f4f6", "#9aa3ad"))
    code = ["if (raw != previousRaw) changedAt = now;           // 값이 바뀔 때마다 타이머 다시 시작",
            "if (now - changedAt >= 25 && raw != stable) {     // 25ms 동안 그대로면 확정",
            "  stable = raw;  if (stable == LOW) { /* 눌림 1회 처리 */ }",
            "}   previousRaw = raw;"]
    for i, s in enumerate(code):
        o.append(f'<text x="64" y="{466+i*28}" font-size="15" xml:space="preserve" font-family="Consolas,\'DejaVu Sans Mono\',monospace" fill="#1d2433">{esc(s)}</text>')
    return "t3_button_debounce_ko.svg", svg(W, Hh, o)


# ================================================================ 4. ADC
def adc():
    W, Hh = 1240, 800
    o = []
    title(o, "ADC 원리 — 가변저항 전압을 0~4095 정수로",
          "ADC(Analog-to-Digital Converter)는 핀 전압을 일정 간격으로 잘라(양자화) 숫자로 바꾼다. ESP32 기본 12비트 → 4096단계.")
    # (a) 전압 분배
    card(o, 40, 110, 360, 650, "① 가변저항 = 전압 분배기", "#2f5d8a")
    x = 130
    o.append(T(x - 20, 180, "3V3", 16, "bold", C["red"]))
    o.append(line([(x, 188), (x, 210)], C["red"], 3))
    o.append(f'<rect x="{x-14}" y="210" width="28" height="230" rx="4" fill="#fff5da" stroke="#8a5818" stroke-width="2"/>')
    o.append(T(x - 24, 300, "R1", 15, "bold", "#8a5818", "end")); o.append(T(x - 24, 390, "R2", 15, "bold", "#8a5818", "end"))
    o += arrow(x + 90, 330, x + 16, 330, C["teal"], 3.5)
    o.append(T(x + 96, 326, "와이퍼", 14, "bold", C["teal"])); o.append(T(x + 96, 348, "→ GPIO34", 14, "bold", C["teal"]))
    o.append(line([(x, 440), (x, 470)], "#333", 3)); o += sym_gnd(x, 470)
    o.append(T(70, 545, "Vout = 3.3V × R2 / (R1 + R2)", 16, "bold", C["ink"]))
    o.append(T(70, 580, "• 손잡이를 돌리면 R1·R2 비율이 바뀐다", 14))
    o.append(T(70, 606, "• 전체 10kΩ: 3.3V/10kΩ = 0.33mA", 14))
    o.append(T(70, 632, "• 가운데 단자를 3V3·GND에 직접", 14, "bold", "#c06000"))
    o.append(T(70, 656, "  꽂지 않는다(끝까지 돌리면 단락)", 14, "bold", "#c06000"))
    o.append(T(70, 690, "• 측정은 ADC1(GPIO32~39) 사용:", 14))
    o.append(T(70, 714, "  ADC2는 Wi-Fi 사용 중 충돌", 14))
    # (b) 양자화
    card(o, 420, 110, 380, 650, "② 양자화 (예: 3비트)", C["blue"])
    gx, gy, gw, gh = 480, 170, 290, 300
    o.append(line([(gx, gy), (gx, gy + gh), (gx + gw, gy + gh)], "#333", 2))
    steps = 8
    pts = []
    for k in range(steps):
        xa = gx + k * gw / steps; xb = gx + (k + 1) * gw / steps
        yk = gy + gh - k * gh / (steps - 1) * 0.95
        pts += [(xa, yk), (xb, yk)]
    o.append(line(pts, C["blue"], 3, cap="butt"))
    o.append(line([(gx, gy + gh), (gx + gw, gy + gh * 0.05)], "#c62828", 2, "6 4"))
    o.append(T(gx + gw / 2, gy + gh + 30, "입력 전압 →", 14, fill="#444", anchor="middle"))
    o.append(T(gx - 10, gy + 10, "코드", 13, fill="#444", anchor="end"))
    o.append(T(gx - 10, gy + gh, "0", 13, fill="#444", anchor="end")); o.append(T(gx - 10, gy + 24, "7", 13, fill="#444", anchor="end"))
    o.append(T(450, 540, "n비트 → 2ⁿ 단계, 최댓값 2ⁿ − 1", 16, "bold", C["ink"]))
    for i, s in enumerate(["• UNO: 10비트 → 0~1023", "• ESP32: 12비트 → 0~4095",
                           "• 1단계 ≈ 3.3V / 4095 ≈ 0.8mV (이상적)", "• analogReadResolution(12) 기본값",
                           "• 빨간 점선 = 이상적인 직선, 파란 계단 = ADC"]):
        o.append(T(450, 576 + i * 28, s, 14))
    # (c) ESP32 실제 응답
    card(o, 820, 110, 380, 650, "③ ESP32 실제 응답 (11dB)", "#c06000")
    gx, gy, gw, gh = 880, 170, 290, 300
    o.append(line([(gx, gy), (gx, gy + gh), (gx + gw, gy + gh)], "#333", 2))
    V = lambda v: gx + v / 3.3 * gw
    D = lambda d: gy + gh - d / 4095 * gh
    o.append(line([(V(0), D(0)), (V(3.3), D(4095))], "#c62828", 2, "6 4"))
    curve = [(V(0), D(0)), (V(0.15), D(0)), (V(0.5), D(420)), (V(1.5), D(1660)), (V(2.5), D(2950)), (V(2.9), D(3650)), (V(3.1), D(4095)), (V(3.3), D(4095))]
    o.append(line(curve, "#c06000", 3.5))
    o.append(rect(V(0), gy, V(0.15) - V(0), gh, "#ffe2c4", "none", 0, 0, ' fill-opacity="0.7"'))
    o.append(rect(V(3.1), gy, V(3.3) - V(3.1), gh, "#ffe2c4", "none", 0, 0, ' fill-opacity="0.7"'))
    o.append(T(V(0) + 4, gy + gh + 22, "0.15V", 12, fill="#c06000")); o.append(T(V(3.1) - 18, gy + gh + 22, "3.1V", 12, fill="#c06000"))
    o.append(T(gx + gw / 2, gy + gh + 46, "입력 전압(V) →", 14, fill="#444", anchor="middle"))
    o.append(T(gx - 8, gy + 10, "4095", 12, fill="#444", anchor="end"))
    o.append(T(gx + 10, gy + 30, "개념도(실측 아님)", 12, fill="#888", italic=True))
    for i, s in enumerate(["• 11dB 감쇠: 측정 범위 약 0.15~3.1V", "  양 끝은 0 또는 4095로 '붙는다'",
                           "• 중간 구간도 완전한 직선이 아니다", "• 전압이 필요하면 보정값 사용:",
                           "  analogReadMilliVolts(34) → mV", "• analogRead 값은 '상대적 크기'로 쓴다"]):
        o.append(T(850, 560 + i * 28, s, 14, "bold" if "Milli" in s else "normal"))
    return "t4_adc_principle_ko.svg", svg(W, Hh, o)


# ================================================================ 5. PWM
def pwm():
    W, Hh = 1240, 900
    o = []
    title(o, "PWM 원리 — 켜져 있는 시간의 비율(듀티비)로 밝기 조절",
          "PWM은 전압을 낮추는 것이 아니라 0V/3.3V를 빠르게 반복한다. 사람 눈은 평균값을 밝기로 느낀다.  평균 전압 ≈ 3.3V × 듀티비")
    x0, period, n = 300, 180, 4
    duties = [(0, 0), (25, 64), (50, 128), (75, 191), (100, 255)]
    for i, (d, v) in enumerate(duties):
        y = 150 + i * 92
        o.append(T(40, y + 8, f"듀티 {d}%", 18, "bold", C["ink"]))
        o.append(T(150, y + 8, f"값 {v}/255", 14, fill="#555"))
        o.append(line([(x0, y - 28), (x0 + period * n, y - 28)], "#eee", 1)); o.append(line([(x0, y + 28), (x0 + period * n, y + 28)], "#eee", 1))
        p = []
        for k in range(n):
            xs = x0 + k * period; xh = xs + period * d / 100
            if d == 0:
                p += [(xs, y + 28), (xs + period, y + 28)]
            elif d == 100:
                p += [(xs, y - 28), (xs + period, y - 28)]
            else:
                p += [(xs, y + 28), (xs, y - 28), (xh, y - 28), (xh, y + 28), (xs + period, y + 28)]
        o.append(line(p, C["blue"], 3, cap="butt"))
        avg = y + 28 - 56 * d / 100
        o.append(line([(x0, avg), (x0 + period * n, avg)], "#c62828", 2, "6 5"))
        o.append(T(x0 + period * n + 20, avg + 5, f"평균 ≈ {3.3*d/100:.2f}V", 14, "bold", "#c62828"))
        o.append(circle(1150, y, 20, C["red"], "#333", 1.5))
        o.append(f'<circle cx="1150" cy="{y}" r="20" fill="#ffffff" fill-opacity="{1-d/100*0.9:.2f}"/>')
    o += arrow(x0, 125, x0 + period, 125, "#555", 2); o += arrow(x0 + period, 125, x0, 125, "#555", 2)
    o.append(T(x0 + period / 2, 118, "주기 T = 1/주파수", 13, "bold", "#555", "middle"))
    y = 620
    o.append(rect(40, y, 560, 250, "#f3f4f6", "#9aa3ad"))
    o.append(T(60, y + 34, "ESP32 LEDC: 주파수 × 2^해상도 ≤ 80MHz", 17, "bold", C["ink"]))
    rows = [("해상도", "단계", "최대 주파수(대략)"), ("8비트", "0~255", "312kHz"), ("10비트", "0~1023", "78kHz"),
            ("12비트", "0~4095", "19.5kHz"), ("13비트", "0~8191", "9.7kHz")]
    for i, r in enumerate(rows):
        for j, s in enumerate(r):
            o.append(T(70 + j * 160, y + 74 + i * 32, s, 15, "bold" if i == 0 else "normal"))
    o.append(T(60, y + 238, "LED: 1~5kHz면 깜빡임이 보이지 않는다. analogWrite 기본 = 8비트.", 13, fill="#555"))
    o.append(rect(630, y, 570, 250, "#f3f4f6", "#9aa3ad"))
    o.append(T(650, y + 34, "두 가지 코드 방법 (Arduino-ESP32 3.x)", 17, "bold", C["ink"]))
    code = ["analogWrite(25, 128);          // 간단: 8비트, 약 50%",
            "", "ledcAttach(25, 5000, 8);       // 직접: 5kHz, 8비트",
            "ledcWrite(25, 192);            // 약 75%",
            "", "ledcAttach(27, 2000, 8);       // 부저",
            "ledcWriteTone(27, 1000);       // 1kHz 50% 톤"]
    for i, s in enumerate(code):
        o.append(f'<text x="650" y="{y+70+i*24}" font-size="14" xml:space="preserve" font-family="Consolas,\'DejaVu Sans Mono\',monospace" fill="#1d2433">{esc(s)}</text>')
    return "t5_pwm_principle_ko.svg", svg(W, Hh, o)


# ================================================================ 6. ADC → PWM 흐름
def flow():
    W, Hh = 1240, 330
    o = []
    title(o, "예제 3의 신호 흐름 — 입력(ADC)과 출력(PWM)은 서로 다른 장치",
          "loop()가 100ms마다 한 바퀴: 측정 → 변환 → 출력 → 기록")
    boxes = [("가변저항 회전", "0~3.3V", "#2f5d8a"), ("GPIO34 · ADC1", "analogRead → 0~4095", C["teal"]),
             ("map()", "0~4095 → 0~255", "#555"), ("GPIO25 · LEDC", "analogWrite → 듀티", C["blue"]),
             ("LED 밝기", "평균 전류 변화", C["red"])]
    x = 40
    for i, (a, b, col) in enumerate(boxes):
        o.append(rect(x, 140, 200, 110, "#fff", col, 3, 12))
        o.append(T(x + 100, 186, a, 17, "bold", col, "middle")); o.append(T(x + 100, 216, b, 14, fill="#333", anchor="middle"))
        if i < 4: o += arrow(x + 204, 195, x + 236, 195, "#333", 3)
        x += 240
    o.append(T(40, 300, "duty = map(adc, 0, 4095, 0, 255);  duty = constrain(duty, 0, 255);   — 비례식 duty = adc × 255 / 4095", 15, fill="#333"))
    return "t6_adc_to_pwm_flow_ko.svg", svg(W, Hh, o)


# ================================================================ 7. 부품
def parts():
    W, Hh = 1240, 980
    o = []
    title(o, "9주차 부품 확인 — 모양 · 극성 · 동작원리",
          "부품을 꽂기 전에 다리 방향과 내부 연결을 확인한다. (그림은 일반적인 형태이며 제품마다 모양이 조금 다를 수 있다)")
    cw, ch = 280, 380
    cards = [(40, 110, "LED (발광 다이오드)"), (340, 110, "저항 220Ω / 330Ω"), (640, 110, "4핀 택트 스위치"), (940, 110, "가변저항 10kΩ"),
             (40, 520, "수동 부저 vs 능동 부저"), (340, 520, "브레드보드"), (640, 520, "점퍼선 · USB 케이블"), (940, 520, "NPN 트랜지스터(선택)")]
    for x, y, t in cards:
        o.append(rect(x, y, cw, ch)); o.append(T(x + cw / 2, y + 34, t, 18, "bold", anchor="middle"))
    # LED
    x, y = 180, 180
    o.append(f'<path d="M{x-22} {y+40} L{x-22} {y} A22 22 0 0 1 {x+22} {y} L{x+22} {y+40} Z" fill="{C["red"]}" fill-opacity="0.85" stroke="#333" stroke-width="2"/>')
    o.append(f'<rect x="{x-26}" y="{y+40}" width="52" height="7" fill="{C["red"]}" stroke="#333"/>')
    o.append(line([(x - 10, y + 47), (x - 10, y + 150)], "#999", 4)); o.append(line([(x + 10, y + 47), (x + 10, y + 125)], "#999", 4))
    o.append(T(x - 18, y + 170, "긴 다리 = +(애노드)", 14, "bold", "#c62828", "end"))
    o.append(T(x + 18, y + 140, "짧은 다리 = −", 14, "bold", "#333"))
    for i, s in enumerate(["+→− 방향으로만 전류가 흐르며 빛남", "평평한 테두리 쪽 = −(캐소드)", "반드시 직렬 저항과 함께"]):
        o.append(T(60, 390 + i * 26, "• " + s, 13))
    # 저항
    x, y = 480, 220
    o.append(line([(x - 110, y), (x + 110, y)], "#999", 4))
    o.append(f'<rect x="{x-55}" y="{y-20}" width="110" height="40" rx="16" fill="#e9cf9a" stroke="#8a6a2a" stroke-width="2"/>')
    for i, col in enumerate(["#c62828", "#c62828", "#6d4c2f"]):
        o.append(f'<rect x="{x-38+i*18}" y="{y-20}" width="9" height="40" fill="{col}"/>')
    o.append(f'<rect x="{x+28}" y="{y-20}" width="9" height="40" fill="#c9a227"/>')
    for i, s in enumerate(["220Ω: 빨강-빨강-갈색-금", "330Ω: 주황-주황-갈색-금", "", "• 전류를 제한한다(옴의 법칙 V=IR)", "• 극성 없음 · LED마다 1개", "• 5색 띠 저항: 빨-빨-검-검-갈 = 220Ω"]):
        o.append(T(360, 290 + i * 26, s, 13, "bold" if i < 2 else "normal"))
    # 택트
    x, y = 780, 230
    o.append(rect(x - 55, y - 55, 110, 110, "#3a3a3a", "#111", 2, 8)); o.append(circle(x, y, 24, "#6d6d6d", "#222", 2))
    for dx, dy, n in ((-55, -40, "1"), (55, -40, "2"), (-55, 40, "3"), (55, 40, "4")):
        o.append(f'<rect x="{x+dx-(14 if dx<0 else 0)}" y="{y+dy-4}" width="14" height="8" fill="#bbb"/>')
        o.append(T(x + dx + (-22 if dx < 0 else 22), y + dy + 6, n, 14, "bold", anchor="middle"))
    o.append(line([(x - 42, y - 40), (x + 42, y - 40)], "#ffb347", 3, "6 4")); o.append(line([(x - 42, y + 40), (x + 42, y + 40)], "#ffb347", 3, "6 4"))
    for i, s in enumerate(["1–2, 3–4는 항상 연결(점선)", "누르면 위·아래가 연결", "→ 대각선 1번·4번 다리 사용", "• 멀티미터 도통 모드로 확인"]):
        o.append(T(660, 330 + i * 26, s, 13, "bold" if i == 2 else "normal", "#c62828" if i == 2 else "#222"))
    # 가변저항
    x, y = 1080, 200
    o.append(rect(x - 60, y - 30, 120, 70, "#2f5d8a", "#173552", 2, 8)); o.append(circle(x, y + 5, 26, "#d8dde3", "#555", 2))
    o.append(line([(x, y + 5), (x + 14, y - 14)], "#333", 4))
    for i, dx in enumerate((-40, 0, 40)):
        o.append(line([(x + dx, y + 40), (x + dx, y + 90)], "#999", 4))
        o.append(T(x + dx, y + 112, ["1", "2", "3"][i], 15, "bold", C["teal"] if i == 1 else "#333", "middle"))
    for i, s in enumerate(["1 → 3V3, 3 → GND", "2(가운데, 와이퍼) → GPIO34", "• 양 끝 사이 저항은 항상 10kΩ", "• 돌리면 와이퍼 위치가 이동", "• B형(선형) 권장"]):
        o.append(T(960, 340 + i * 26, s, 13, "bold" if i < 2 else "normal", C["teal"] if i == 1 else "#222"))
    # 부저
    for k, (lab, sub, bx_) in enumerate((("수동(Passive)", "밑면 기판 보임", 110), ("능동(Active)", "밑면 밀봉·스티커", 250))):
        o.append(circle(bx_, 620, 36, "#2b2b2b", "#000", 2)); o.append(circle(bx_, 620, 6, "#555"))
        o.append(T(bx_ - 18, 606, "+", 16, "bold", "#ff6b6b", "middle"))
        o.append(T(bx_, 680, lab, 14, "bold", C["purple"] if k == 0 else "#555", "middle"))
        o.append(T(bx_, 700, sub, 12, fill="#555", anchor="middle"))
    for i, s in enumerate(["• 수동: 내부 발진기 없음 → PWM 주파수", "  를 넣어야 소리(음높이 조절 가능)", "• 능동: HIGH만 주면 고정음",
                           "• 9주차는 수동 부저 사용", "• + 다리 → GPIO27, − → GND", "• 소비전류 큰 부저 → NPN 구동"]):
        o.append(T(56, 740 + i * 24, s, 13, "bold" if i == 3 else "normal"))
    # 브레드보드
    bx0, by0 = 370, 580
    o.append(rect(bx0, by0, 220, 150, C["bb"], C["bbS"], 2, 6))
    o.append(line([(bx0 + 10, by0 + 16), (bx0 + 210, by0 + 16)], C["rail_neg"], 3))
    for r in range(5):
        for i in range(10):
            o.append(f'<rect x="{bx0+14+i*20}" y="{by0+32+r*14}" width="6" height="6" fill="{C["hole"]}"/>')
    o.append(rect(bx0 + 52, by0 + 27, 16, 72, "none", "#2e9d57", 3, 6))
    o.append(rect(bx0 + 4, by0 + 104, 212, 16, C["gap"], "none", 0, 0))
    o.append(T(bx0 + 110, by0 + 138, "(아래쪽 f~j도 같은 구조)", 12, fill="#555", anchor="middle"))
    for i, s in enumerate(["• 레일(−/+): 가로로 연결", "• a~e, f~j: 세로 5칸씩 연결", "• 중앙 홈 위·아래는 끊겨 있음",
                           "• 긴 레일은 가운데가 끊긴 제품도 있음"]):
        o.append(T(356, 760 + i * 26, s, 13))
    # 점퍼/USB
    o.append(line([(680, 600), (760, 600)], "#2664bf", 6)); o.append(rect(660, 594, 20, 12, "#222", "none", 0, 2)); o.append(rect(760, 594, 20, 12, "#222", "none", 0, 2))
    o.append(T(800, 606, "M-M", 13, "bold"))
    o.append(line([(680, 640), (760, 640)], "#d43b3b", 6)); o.append(rect(660, 634, 20, 12, "#222", "none", 0, 2)); o.append(rect(760, 632, 20, 16, "#555", "#222", 1, 2))
    o.append(T(800, 646, "M-F (보드 핀 연결)", 13, "bold"))
    o.append(rect(680, 670, 60, 30, "#777", "#333", 2, 4)); o.append(line([(740, 685), (860, 685)], "#333", 6))
    o.append(T(680, 720, "Micro-USB '데이터' 케이블", 13, "bold"))
    for i, s in enumerate(["• 충전 전용 케이블은 포트가 안 보임", "• 색 규칙: 빨강=3V3, 검정=GND,", "  그 외 색 = 신호선",
                           "• 장치 관리자에서 COM 포트 확인", "  (CP210x/CH340 드라이버)"]):
        o.append(T(656, 750 + i * 24, s, 13))
    # NPN
    cx, cy = 1060, 620
    o.append(f'<path d="M{cx-30} {cy+20} L{cx-30} {cy-10} A30 30 0 0 1 {cx+30} {cy-10} L{cx+30} {cy+20} Z" fill="#222"/>')
    for i, dx in enumerate((-18, 0, 18)):
        o.append(line([(cx + dx, cy + 20), (cx + dx, cy + 70)], "#999", 4))
        o.append(T(cx + dx, cy + 90, ["E", "B", "C"][i], 14, "bold", "#333", "middle"))
    o.append(T(cx + 50, cy + 10, "평면이 앞", 12, fill="#555"))
    for i, s in enumerate(["• 작은 전류(B)로 큰 전류(C→E) 스위칭", "• 부저·릴레이·모터 구동 시 사용", "• 핀 순서는 품번마다 다름(예시 E-B-C)",
                           "• 9주차 필수 아님 → 확장 과제"]):
        o.append(T(956, 740 + i * 26, s, 13))
    return "t7_parts_guide_ko.svg", svg(W, Hh, o)


# ================================================================ 8. 전원 · 안전
def power():
    W, Hh = 1240, 900
    o = []
    title(o, "전원 연결과 배선 안전 — 전원을 켜기 전에",
          "공식 문서: 전원은 ① Micro-USB(기본) ② 5V+GND 핀 ③ 3V3+GND 핀 중 '오직 하나'만 사용한다. 동시에 쓰면 보드·전원이 손상될 수 있다.")
    # 전원 경로
    o.append(rect(40, 110, 1160, 230))
    o.append(T(60, 144, "보드 내부 전원 경로", 18, "bold"))
    xs = [(70, "PC USB 5V", "#a40000"), (330, "보드 5V 라인", "#a40000"), (590, "3.3V LDO 레귤레이터", "#555"), (850, "3V3 라인", C["red"])]
    for x, s, col in xs:
        o.append(rect(x, 180, 200, 70, "#fff", col, 3, 10)); o.append(T(x + 100, 222, s, 16, "bold", col, "middle"))
    for x in (270, 530, 790):
        o += arrow(x + 4, 215, x + 56, 215, "#333", 3)
    o.append(rect(1080, 180, 100, 70, C["board"], C["boardS"], 2, 10)); o.append(T(1130, 222, "ESP32", 16, "bold", "white", "middle"))
    o += arrow(1050, 215, 1076, 215, "#333", 3)
    o.append(T(430, 280, "▼ 5V 핀 (J2-19)", 14, "bold", "#a40000", "middle"))
    o.append(T(950, 280, "▼ 3V3 핀 (J2-1) → 가변저항·센서", 14, "bold", C["red"], "middle"))
    o.append(T(60, 318, "GPIO는 3V3 라인 기준으로 동작한다 → GPIO 입력에는 0~3.3V만. 5V 핀은 전원 핀이며 GPIO 신호가 아니다.", 15, fill="#333"))
    # 하지 말 것
    o.append(rect(40, 360, 570, 500, "#fff6f6", "#e0a0a0"))
    o.append(T(60, 396, "✗ 하지 말 것", 20, "bold", "#c62828"))
    bad = ["5V(UNO 출력·5V 센서·5V 핀)를 GPIO에 연결", "3V3 또는 5V를 GND와 직접 연결(단락)",
           "USB를 꽂은 채 5V/3V3 핀에 외부 전원 추가", "저항 없이 LED를 GPIO–GND에 연결",
           "GPIO34~39에 LED·부저 연결(입력 전용)", "GPIO6~11(플래시)·GPIO1/3(UART0) 사용",
           "부팅 스트랩 GPIO0/2/5/12/15에 부품 연결", "USB 연결 상태에서 배선 변경",
           "모터·릴레이·큰 부저를 GPIO로 직접 구동"]
    for i, s in enumerate(bad):
        o.append(T(70, 436 + i * 46, f"{i+1}. {s}", 16))
    # 할 것
    o.append(rect(630, 360, 570, 500, "#f3fbf5", "#9fd0ae"))
    o.append(T(650, 396, "✓ 확인 순서", 20, "bold", C["green"]))
    good = ["USB 분리 상태에서 배선", "보드 GND ↔ 브레드보드 − 레일 (공통 GND)",
            "+ 레일에는 3V3만 연결 (레일에 '3V3' 표시)", "LED 극성·저항 직렬 확인",
            "버튼 대각선 다리 · 가변저항 가운데 = GPIO34", "손가락으로 각 GPIO에서 GND까지 경로 따라가기",
            "USB 연결 → 보드 전원 LED 확인", "포트 선택·업로드 → 시리얼 115200",
            "이상(발열·냄새·LED 꺼짐) 시 즉시 USB 분리"]
    for i, s in enumerate(good):
        o.append(T(660, 436 + i * 46, f"{i+1}. {s}", 16))
    o.append(T(40, 888, "※ DevKitC는 폭이 넓어 브레드보드 하나에 꽂으면 한쪽 핀 옆 구멍이 거의 남지 않는다 → 보드는 밖에 두고 M-F 점퍼선으로 연결하거나 브레드보드 2개를 나란히 쓴다.", 14, "bold", "#555"))
    return "t8_power_safety_ko.svg", svg(W, Hh + 10, o)


# ================================================================ 9. UNO vs ESP32
def uno_vs():
    W, Hh = 1240, 700
    o = []
    title(o, "Arduino UNO R3 → ESP32-DevKitC V4: 바뀌는 것",
          "코드 문법(setup/loop, digitalWrite…)은 거의 같다. 바뀌는 것은 전압 · 핀 번호 · ADC 범위 · PWM 방식이다.")
    rows = [("항목", "Arduino UNO R3", "ESP32-DevKitC V4", "실습에서 주의"),
            ("MCU / 클럭", "ATmega328P 8비트 16MHz", "ESP32 32비트 듀얼코어 최대 240MHz", "millis 기반 코드는 그대로"),
            ("메모리", "플래시 32KB · SRAM 2KB", "플래시 4MB(모듈) · SRAM 520KB", "큰 배열·문자열 여유"),
            ("논리 전압", "5V", "3.3V (5V 인가 금지)", "UNO 회로를 그대로 옮기지 않는다"),
            ("핀 이름", "D2, A0 … 보드 숫자", "IO18 → 코드 18 (GPIO 번호)", "J2/J3 순번과 다름"),
            ("ADC", "10비트 0~1023 · A0~A5", "12비트 0~4095 · ADC1/ADC2", "map 범위 4095로 수정"),
            ("PWM", "D3,5,6,9,10,11 고정", "LEDC: 출력 가능한 거의 모든 GPIO", "analogWrite / ledcAttach"),
            ("입력 전용 핀", "없음", "GPIO34~39 (풀업 없음)", "출력·버튼(풀업) 불가"),
            ("무선 통신", "없음(외장 모듈)", "Wi-Fi · Bluetooth 내장", "Wi-Fi 사용 시 ADC2 불가"),
            ("업로드", "USB-시리얼 → 자동", "USB-UART → 자동(안 되면 BOOT 누름)", "보드 'ESP32 Dev Module'")]
    xs = [40, 220, 520, 860]
    for i, r in enumerate(rows):
        y = 130 + i * 54
        o.append(rect(30, y - 30, 1180, 50, "#eef2f7" if i == 0 else ("#ffffff" if i % 2 else "#f8f9fb"), "#dde2ea", 1, 4))
        for j, s in enumerate(r):
            col = "#a40000" if (j == 2 and i in (3, 7)) else ("#1d2433")
            o.append(T(xs[j] + 6, y + 2, s, 15, "bold" if (i == 0 or j == 0) else "normal", col))
    return "t9_uno_vs_esp32_ko.svg", svg(W, Hh, o)


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    for f in (pinout, gpio, debounce, adc, pwm, flow, parts, power, uno_vs):
        n, s = f()
        open(os.path.join(OUT, n), "w", encoding="utf-8").write(s)
        print(n)
