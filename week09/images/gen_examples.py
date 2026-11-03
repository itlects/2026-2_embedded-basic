# -*- coding: utf-8 -*-
"""9주차 예제별 회로도(schematic) + 브레드보드 배선도 생성. 사용: python3 gen_examples.py OUTDIR"""
import sys, os
from figlib import *

OUT = sys.argv[1] if len(sys.argv) > 1 else "."

# 실제 헤더 위치(Espressif 공식 J2/J3 표)
HDR = {"GPIO25": "J2-9", "GPIO26": "J2-10", "GPIO27": "J2-11", "GPIO34": "J2-5", "3V3": "J2-1",
       "GND": "J2-14", "GPIO18": "J3-9", "GPIO19": "J3-8", "GNDb": "J3-1"}

# ================================================================ 브레드보드
P = 18
NCOL = 40
BX, BY = 400, 210
ROWS_TOP, ROWS_BOT = "abcde", "fghij"


def col_x(c): return BX + 40 + (c - 1) * P


def row_y(r):
    if r == "T-": return BY + 26
    if r == "T+": return BY + 26 + P
    if r in ROWS_TOP: return BY + 26 + 2 * P + 22 + ROWS_TOP.index(r) * P
    if r in ROWS_BOT: return BY + 26 + 2 * P + 22 + 5 * P + 34 + ROWS_BOT.index(r) * P
    if r == "B-": return row_y("j") + 22 + P
    if r == "B+": return row_y("j") + 22 + 2 * P


def Hh(r, c): return (col_x(c), row_y(r))


def bb_w(): return 40 + (NCOL - 1) * P + 40


def bb_h(): return row_y("B+") - BY + 26


def breadboard_base(plus_label="+ 레일"):
    b = [rect(BX, BY, bb_w(), bb_h(), C["bb"], C["bbS"], 2, 10)]
    gy = (row_y("e") + row_y("f")) / 2
    b.append(f'<rect x="{BX+10}" y="{gy-10}" width="{bb_w()-20}" height="20" fill="{C["gap"]}"/>')
    b.append(T(BX + 16, gy + 6, "중앙 홈", 13, fill="#8a816a"))
    for r, col in (("T-", C["rail_neg"]), ("T+", C["rail_pos"]), ("B-", C["rail_neg"]), ("B+", C["rail_pos"])):
        y = row_y(r) + (-11 if r == "T-" else 11 if r == "T+" else -11 if r == "B-" else 11)
        b.append(line([(BX + 30, y), (BX + bb_w() - 30, y)], col, 2))
    for r in ("T-", "B-"):
        b.append(T(BX + 12, row_y(r) + 6, "−", 18, "bold", C["rail_neg"]))
    for r in ("T+", "B+"):
        b.append(T(BX + 12, row_y(r) + 6, "+", 18, "bold", C["rail_pos"]))
    for r in ["T-", "T+", "B-", "B+"]:
        for c in range(1, NCOL + 1):
            if (c - 1) % 6 == 5: continue
            x, y = Hh(r, c)
            b.append(f'<rect x="{x-3}" y="{y-3}" width="6" height="6" rx="1" fill="{C["hole"]}"/>')
    for r in ROWS_TOP + ROWS_BOT:
        b.append(T(BX + 22, row_y(r) + 4, r, 11, fill="#8a816a", anchor="middle"))
        for c in range(1, NCOL + 1):
            x, y = Hh(r, c)
            b.append(f'<rect x="{x-3}" y="{y-3}" width="6" height="6" rx="1" fill="{C["hole"]}"/>')
    for c in [1] + list(range(5, NCOL + 1, 5)):
        b.append(T(col_x(c), row_y("a") - 8, str(c), 11, fill="#8a816a", anchor="middle"))
        b.append(T(col_x(c), row_y("j") + 15, str(c), 11, fill="#8a816a", anchor="middle"))
    return b


def tact(c):
    x1, y1 = Hh("e", c); x2, y2 = Hh("f", c + 2)
    o = [f'<rect x="{x1-12}" y="{y1-12}" width="{x2-x1+24}" height="{y2-y1+24}" rx="5" fill="#3a3a3a" stroke="#111" stroke-width="2"/>',
         circle((x1 + x2) / 2, (y1 + y2) / 2, 11, "#6d6d6d", "#222", 2)]
    for (r, cc) in (("e", c), ("e", c + 2), ("f", c), ("f", c + 2)):
        x, y = Hh(r, cc); o.append(circle(x, y, 4, "#d9d9d9"))
    return o


def resistor(r, c1, c2, bands=("#c62828", "#c62828", "#6d4c2f", "#c9a227")):
    x1, y = Hh(r, c1); x2, _ = Hh(r, c2)
    o = [line([(x1, y), (x2, y)], "#9a9a9a", 3)]
    mx = (x1 + x2) / 2
    o.append(f'<rect x="{mx-24}" y="{y-8}" width="48" height="16" rx="7" fill="#e9cf9a" stroke="#8a6a2a" stroke-width="1.5"/>')
    for i, col in enumerate(bands):
        o.append(f'<rect x="{mx-17+i*9+(4 if i==3 else 0)}" y="{y-8}" width="4" height="16" fill="{col}"/>')
    o.append(dot(x1, y, "#9a9a9a", 3.5)); o.append(dot(x2, y, "#9a9a9a", 3.5))
    return o


def led(r, cp, cn, color):
    xp, y = Hh(r, cp); xn, _ = Hh(r, cn)
    mx = (xp + xn) / 2
    o = [line([(xp, y), (xp, y - 22)], "#9a9a9a", 3), line([(xn, y), (xn, y - 16)], "#9a9a9a", 3),
         f'<path d="M{mx-11} {y-20} L{mx-11} {y-34} A11 11 0 0 1 {mx+11} {y-34} L{mx+11} {y-20} Z" fill="{color}" fill-opacity="0.85" stroke="#333" stroke-width="1.5"/>',
         f'<rect x="{mx-13}" y="{y-22}" width="26" height="4" fill="{color}" stroke="#333" stroke-width="1"/>',
         dot(xp, y, "#9a9a9a", 3.5), dot(xn, y, "#9a9a9a", 3.5),
         T(xp - 4, y + 15, "+", 13, "bold", "#c62828", "middle")]
    return o


def pot(r, c):
    """가변저항 3핀: c, c+2(와이퍼), c+4. 행 r(아래 절반 g/h 권장)."""
    x1, y = Hh(r, c); x3, _ = Hh(r, c + 4); xm = (x1 + x3) / 2
    o = [f'<rect x="{x1-14}" y="{y-50}" width="{x3-x1+28}" height="38" rx="6" fill="#2f5d8a" stroke="#173552" stroke-width="2"/>',
         circle(xm, y - 31, 13, "#d8dde3", "#555", 2), line([(xm, y - 31), (xm + 8, y - 40)], "#333", 3)]
    for cc, lab in ((c, "1"), (c + 2, "2"), (c + 4, "3")):
        x, _ = Hh(r, cc)
        o.append(line([(x, y - 12), (x, y)], "#9a9a9a", 3))
        o.append(dot(x, y, "#9a9a9a", 3.5))
    return o


def buzzer(r, cp, cn):
    xp, y = Hh(r, cp); xn, _ = Hh(r, cn)
    mx = (xp + xn) / 2
    return [circle(mx, y, 27, "#2b2b2b", "#000", 2), circle(mx, y, 5, "#555"),
            T(mx - 15, y - 9, "+", 15, "bold", "#ff6b6b", "middle"),
            dot(xp, y, "#cfcfcf", 3.5), dot(xn, y, "#cfcfcf", 3.5)]


def jumper(r1, c1, r2, c2, color=C["gnd"]):
    (x1, y1), (x2, y2) = Hh(r1, c1), Hh(r2, c2)
    return [line([(x1, y1), (x2, y2)], color, 4), dot(x1, y1, color), dot(x2, y2, color)]


def board_block(pins, top_pins, bot_pins):
    """pins: [(label, key)] 보드 그림. 위쪽 묶음/아래쪽 묶음 각 y 계산."""
    pins_y = {}
    y = row_y("T-") + 4
    for name, k in top_pins:
        pins_y[k] = (name, y); y += 40
    y = max(y + 20, row_y("f") + 30)
    for name, k in bot_pins:
        pins_y[k] = (name, y); y += 40
    ys = [v[1] for v in pins_y.values()]
    x, w = 40, 280
    top, bot = min(ys) - 70, max(ys) + 62
    o = [rect(x, top, w, bot - top, C["board"], C["boardS"], 3, 16),
         T(x + w / 2, top + 34, "ESP32-DevKitC V4", 22, "bold", "white", "middle"),
         T(x + w / 2, top + 54, "(그림은 연결 관계만 표시)", 12, fill="#dce9f7", anchor="middle"),
         T(x + w / 2, bot - 16, "Micro-USB → PC  ·  3.3V 논리", 13, fill="#dce9f7", anchor="middle")]
    for k, (name, yy) in pins_y.items():
        o.append(f'<rect x="{x+w-24}" y="{yy-9}" width="18" height="18" rx="2" fill="#111"/>')
        hdr = HDR.get(name if not k.startswith("GB") else "GNDb", "")
        if name == "GND" and k.startswith("G2"):
            hdr = "J3-1"
        o.append(T(x + w - 34, yy + 6, name, 17, "bold", "white", "end"))
        if hdr:
            o.append(T(x + 16, yy + 5, hdr, 12, fill="#b9d3ee"))
    return o, pins_y, x + w - 15


def breadboard_figure(fname, ttl, sub, top_pins, bot_pins, top_targets, bot_targets, parts_fn, steps, ncol=40, note=None):
    global NCOL
    NCOL = ncol
    W = BX + bb_w() + 40
    body = [T(40, 48, ttl, 28, "bold", C["ink"]), T(40, 80, sub, 16, fill=C["sub"])]
    body += breadboard_base()
    blk, pins_y, px = board_block(None, top_pins, bot_pins)
    body += blk
    wires = []
    for n, (k, row, col, color) in enumerate(top_targets):
        y = pins_y[k][1]
        lane_x = px + 16 + n * 12
        over = BY - 30 - (len(top_targets) - 1 - n) * 14
        tx, ty = Hh(row, col)
        wires.append(line([(px, y), (lane_x, y), (lane_x, over), (tx, over), (tx, ty)], color, 4))
        wires.append(dot(tx, ty, color))
    nb = len(bot_targets)
    for n, (k, row, col, color) in enumerate(bot_targets):
        y = pins_y[k][1]
        lane_x = px + 16 + (nb - 1 - n) * 12
        under = BY + bb_h() + 18 + n * 14
        tx, ty = Hh(row, col)
        wires.append(line([(px, y), (lane_x, y), (lane_x, under), (tx, under), (tx, ty)], color, 4))
        wires.append(dot(tx, ty, color))
    j, parts = parts_fn()
    body += wires + j + parts
    last_under = BY + bb_h() + 18 + max(nb - 1, 0) * 14
    by = max(last_under, max(v[1] for v in pins_y.values()) + 62) + 36
    body.append(rect(40, by, W - 80, len(steps) * 30 + 64, "#f3f4f6", "#9aa3ad", 2, 10))
    body.append(T(62, by + 32, "배선 순서 (USB를 뺀 상태에서)", 17, "bold"))
    for i, (h, s) in enumerate(steps):
        body.append(T(62, by + 64 + i * 30, h, 15, "bold"))
        body.append(T(150, by + 64 + i * 30, s, 15))
    foot = note or "• 보드 핀의 실제 위치는 보드 실크인쇄(IO25 등)와 J2/J3 표기로 찾는다.  • 레일이 가운데에서 끊긴 브레드보드는 짧은 점퍼로 이어 준다."
    body.append(T(62, by + 64 + len(steps) * 30 - 4, foot, 13, fill="#555"))
    return fname, svg(W, by + len(steps) * 30 + 84, body)


# ---------------------------------------------------------------- 예제 1
LX1 = 12


def ex1_parts():
    j = jumper("a", LX1 + 5, "T-", LX1 + 5)
    p = resistor("c", LX1, LX1 + 4) + led("b", LX1 + 4, LX1 + 5, C["red"])
    x, _ = Hh("e", LX1 + 2); p += pill(x, row_y("e") + 4, "LED1 + 220Ω", C["red"])
    return j, p


def bb_ex1():
    return breadboard_figure(
        "ex1_led_breadboard_ko.svg", "예제 1 · GPIO25 LED 출력 — 브레드보드 배선도",
        "GPIO25 → a12 → 저항(c12~c16) → LED +(b16) / LED −(b17) → a17 → 위 − 레일 → 보드 GND",
        [("GND", "G1"), ("GPIO25", "L1")], [],
        [("G1", "T-", 30, C["gnd"]), ("L1", "a", LX1, C["red"])], [], ex1_parts,
        [("① GND", "보드 GND(J2-14) → 위쪽 − 레일 30열"),
         ("② 신호", "보드 GPIO25(J2-9) → a12"),
         ("③ 저항", "220Ω(빨강-빨강-갈색) 또는 330Ω(주황-주황-갈색)을 c12–c16에 꽂는다 (극성 없음)"),
         ("④ LED", "긴 다리(+) b16, 짧은 다리(−) b17  →  a17에서 위 − 레일로 짧은 점퍼"),
         ("⑤ 확인", "USB 연결 전: GPIO25–GND 사이에 저항·LED가 직렬로 하나의 길만 있는지 손가락으로 따라간다")],
        ncol=32)


# ---------------------------------------------------------------- 예제 2
BC2, LX2 = 4, 14


def ex2_parts():
    j = jumper("a", BC2, "T-", BC2) + jumper("a", LX2 + 5, "T-", LX2 + 5)
    p = tact(BC2)
    x, _ = Hh("h", BC2 + 1); p += pill(x, row_y("h") + 4, "버튼 RUN", "#b8322f")
    p += resistor("c", LX2, LX2 + 4) + led("b", LX2 + 4, LX2 + 5, C["red"])
    x, _ = Hh("e", LX2 + 2); p += pill(x, row_y("e") + 4, "LED1", C["red"])
    return j, p


def bb_ex2():
    return breadboard_figure(
        "ex2_button_breadboard_ko.svg", "예제 2 · 버튼(GPIO18) + LED(GPIO25) — 브레드보드 배선도",
        "버튼은 중앙 홈을 가로질러 꽂고 대각선 두 다리(a4 쪽, j6 쪽)를 사용한다 · INPUT_PULLUP이므로 외부 저항 불필요",
        [("GND", "G1"), ("GPIO25", "L1")], [("GPIO18", "B1")],
        [("G1", "T-", 30, C["gnd"]), ("L1", "a", LX2, C["red"])],
        [("B1", "j", BC2 + 2, "#b8322f")], ex2_parts,
        [("① GND", "보드 GND(J2-14) → 위쪽 − 레일 30열"),
         ("② 버튼", "택트 스위치를 e4·e6·f4·f6에 꽂는다 (중앙 홈을 가로지르게)"),
         ("", "GPIO18(J3-9) → j6  ·  a4 → 위 − 레일 (대각선 다리 사용: 방향이 바뀌어도 누를 때만 연결)"),
         ("③ LED", "GPIO25(J2-9) → a14 · 저항 c14–c18 · LED +b18 / −b19 · a19 → 위 − 레일"),
         ("④ 확인", "버튼을 누르지 않은 상태에서 GPIO18과 GND가 직접 이어져 있지 않은지 확인 (이어져 있으면 항상 LOW)")],
        ncol=32)


# ---------------------------------------------------------------- 예제 3
PC3, LX3 = 6, 18


def ex3_parts():
    j = jumper("j", PC3, "B+", PC3, C["v33"]) + jumper("j", PC3 + 4, "B-", PC3 + 4) + jumper("a", LX3 + 5, "T-", LX3 + 5)
    p = pot("h", PC3)
    x, _ = Hh("g", PC3 + 2); p += pill(x, (row_y("e")+row_y("f"))/2 + 5, "가변저항 10kΩ", "#2f5d8a", 12)
    p += resistor("c", LX3, LX3 + 4) + led("b", LX3 + 4, LX3 + 5, C["red"])
    x, _ = Hh("e", LX3 + 2); p += pill(x, row_y("e") + 4, "LED1 (PWM)", C["red"])
    return j, p


def bb_ex3():
    return breadboard_figure(
        "ex3_adc_pwm_breadboard_ko.svg", "예제 3 · 가변저항(GPIO34 ADC1) → LED PWM(GPIO25) — 브레드보드 배선도",
        "아래 + 레일 = 3V3 전용(5V 금지) · 가변저항 양 끝은 3V3/GND, 가운데(와이퍼)만 GPIO34",
        [("GND", "G1"), ("GPIO25", "L1")], [("GPIO34", "A1"), ("3V3", "V"), ("GND", "G2")],
        [("G1", "T-", 30, C["gnd"]), ("L1", "a", LX3, C["red"])],
        [("A1", "j", PC3 + 2, C["teal"]), ("V", "B+", 30, C["v33"]), ("G2", "B-", 31, C["gnd"])], ex3_parts,
        [("① 전원", "3V3(J2-1) → 아래 + 레일 30열  ·  GND(J3-1 또는 J2-14) → 아래 − 레일 31열, GND(J2-14) → 위 − 레일 30열"),
         ("② 가변", "가변저항 3핀을 h6·h8·h10에 꽂는다 → j6 → 아래 + 레일(3V3), j10 → 아래 − 레일(GND)"),
         ("", "가운데 다리 열 j8 ← GPIO34(J2-5)  (GPIO34는 입력 전용·내부 풀업 없음)"),
         ("③ LED", "GPIO25(J2-9) → a18 · 저항 c18–c22 · LED +b22 / −b23 · a23 → 위 − 레일"),
         ("④ 확인", "멀티미터로 j8–GND 전압이 손잡이 회전에 따라 0~3.3V로 바뀌는지 (USB 연결 후) 확인")],
        ncol=32)


# ---------------------------------------------------------------- 예제 4
BC4 = [3, 9]
LX4 = [16, 23]
PC4, BZ4 = 30, (38, 41)


def ex4_parts():
    j = []
    for c in BC4: j += jumper("a", c, "T-", c)
    for X in LX4: j += jumper("a", X + 5, "T-", X + 5)
    j += jumper("j", PC4, "B+", PC4, C["v33"]) + jumper("j", PC4 + 4, "B-", PC4 + 4) + jumper("j", BZ4[1], "B-", BZ4[1])
    p = []
    for i, c in enumerate(BC4):
        p += tact(c)
        x, _ = Hh("h", c + 1)
        p += pill(x, row_y("h") + 4, ["RUN", "MODE"][i], ["#b8322f", "#1b6e48"][i])
    for i, X in enumerate(LX4):
        p += resistor("c", X, X + 4) + led("b", X + 4, X + 5, [C["red"], C["green"]][i])
        x, _ = Hh("e", X + 2); p += pill(x, row_y("e") + 4, f"LED{i+1}", [C["red"], C["green"]][i])
    p += pot("h", PC4)
    x, _ = Hh("g", PC4 + 2); p += pill(x, (row_y("e")+row_y("f"))/2 + 5, "가변저항", "#2f5d8a", 12)
    p += buzzer("h", BZ4[0], BZ4[1])
    x, _ = Hh("h", BZ4[1]); p += pill(x + 70, row_y("h") + 5, "수동 부저", C["purple"])
    return j, p


def bb_ex4():
    return breadboard_figure(
        "ex4_integrated_breadboard_ko.svg", "예제 4 · 복합 제어(버튼2·LED2·가변저항·부저) — 브레드보드 배선도",
        "예제 1~3 회로를 한 브레드보드에 합친다 · 위 − 레일과 아래 − 레일은 모두 보드 GND · 아래 + 레일 = 3V3",
        [("GND", "G1"), ("GPIO26", "L2"), ("GPIO25", "L1")],
        [("GPIO18", "B1"), ("GPIO19", "B2"), ("GPIO34", "A1"), ("GPIO27", "Z"), ("3V3", "V"), ("GND", "G2")],
        [("G1", "T-", 46, C["gnd"]), ("L2", "a", LX4[1], C["green"]), ("L1", "a", LX4[0], C["red"])],
        [("B1", "j", BC4[0] + 2, "#b8322f"), ("B2", "j", BC4[1] + 2, "#1b6e48"), ("A1", "j", PC4 + 2, C["teal"]),
         ("Z", "j", BZ4[0], C["purple"]), ("V", "B+", 46, C["v33"]), ("G2", "B-", 47, C["gnd"])], ex4_parts,
        [("① 전원", "GND(J2-14) → 위 − 레일 46열 · GND(J3-1) → 아래 − 레일 47열 · 3V3(J2-1) → 아래 + 레일 46열"),
         ("② 버튼", f"RUN: e3·e5·f3·f5, GPIO18(J3-9) → j5, a3 → 위 − 레일  ·  MODE: e9·e11·f9·f11, GPIO19(J3-8) → j11, a9 → 위 − 레일"),
         ("③ LED", f"LED1: GPIO25 → a16, 저항 c16–c20, +b20/−b21  ·  LED2: GPIO26 → a23, 저항 c23–c27, +b27/−b28  ·  a21·a28 → 위 − 레일"),
         ("④ 가변", f"h30·h32·h34 · j30 → 아래 + 레일(3V3) · j34 → 아래 − 레일 · GPIO34(J2-5) → j32"),
         ("⑤ 부저", f"(+) h38, (−) h41 · GPIO27(J2-11) → j38 · j41 → 아래 − 레일  (3.3V 구동 가능한 소형 수동 부저만 직결)")],
        ncol=48)


# ================================================================ 회로도(개략)
def schematic(fname, ttl, sub, rows, foot, H=None):
    """rows: list of dict(kind, pin, color, label)"""
    W = 1240
    o = [T(40, 48, ttl, 28, "bold", C["ink"]), T(40, 80, sub, 16, fill=C["sub"])]
    bx, bw, top = 40, 240, 112
    px = bx + bw
    gnd_x = 1130
    y = 175
    taps = []
    elems = []
    pin_marks = []
    for r in rows:
        k = r["kind"]
        if k == "head":
            elems.append(T(320, y, r["label"], 17, "bold", "#333")); y += 46; continue
        if k == "led":
            pin_marks.append((r["pin"], y))
            elems.append(line([(px, y), (520, y)], r["color"], 4))
            elems += sym_resistor_h(520, 610, y, r.get("r", "220~330Ω"))
            elems.append(line([(610, y), (700, y)], r["color"], 4))
            elems += sym_led_h(700, y, r["color"])
            elems.append(T(790, y + 6, r["label"], 16, "bold", r["color"]))
            elems.append(line([(732, y), (760, y), (760, y + 24), (gnd_x, y + 24)], "#333", 3))
            taps.append(y + 24); y += 82
        elif k == "btn":
            pin_marks.append((r["pin"], y))
            elems.append(line([(px, y), (580, y)], r["color"], 4))
            elems += sym_switch_h(580, y, r["color"])
            elems.append(line([(640, y), (gnd_x, y)], "#333", 3))
            elems.append(T(680, y - 10, r["label"], 16, "bold", r["color"]))
            elems.append(T(420, y - 10, "내부 풀업 ≈45kΩ → 평소 HIGH", 13, fill="#666", anchor="middle"))
            taps.append(y); y += 62
        elif k == "pot":
            # 3V3 → 저항체 상단, 하단 → GND, 와이퍼 → GPIO34
            yv, yw = y, y + 50
            pin_marks.append(("3V3", yv)); pin_marks.append((r["pin"], yw))
            xr = 760
            elems.append(line([(px, yv), (xr, yv), (xr, yv + 6)], C["v33"], 4))
            elems.append(f'<rect x="{xr-12}" y="{yv+6}" width="24" height="90" rx="3" fill="#fff5da" stroke="#8a5818" stroke-width="2"/>')
            elems.append(line([(px, yw), (xr - 30, yw)], C["teal"], 4))
            elems += arrow(xr - 40, yw, xr - 13, yw, C["teal"], 4)
            elems.append(line([(xr, yv + 96), (xr, yv + 120), (gnd_x, yv + 120)], "#333", 3))
            elems.append(T(xr + 26, yv + 30, r["label"], 16, "bold", "#2f5d8a"))
            elems.append(T(xr + 26, yv + 54, "1번 → 3V3, 2번(가운데·와이퍼) → GPIO34, 3번 → GND", 14, fill="#444"))
            elems.append(T(xr + 26, yv + 76, "Vout = 3.3V × (아래쪽 저항 / 전체 저항)", 14, fill="#444"))
            elems.append(T(470, yv - 10, "3.3V (5V 금지)", 13, "bold", C["v33"], "middle"))
            elems.append(T(470, yw - 10, "ADC1_CH6 · 입력 전용", 13, "bold", C["teal"], "middle"))
            taps.append(yv + 120); y += 160
        elif k == "buz":
            pin_marks.append((r["pin"], y))
            elems.append(line([(px, y), (704, y)], C["purple"], 4))
            elems += sym_buzzer(730, y)
            elems.append(line([(756, y), (gnd_x, y)], "#333", 3))
            elems.append(T(790, y - 12, r["label"], 16, "bold", C["purple"]))
            elems.append(T(470, y - 10, "LEDC 톤(주파수) 출력", 13, fill="#666", anchor="middle"))
            taps.append(y); y += 72
    gy = y + 4
    pin_marks.append(("GND", gy))
    bot = gy + 50
    o.append(rect(bx, top, bw, bot - top, C["board"], C["boardS"], 3, 16))
    o.append(T(bx + bw / 2, top + 36, "ESP32-DevKitC V4", 20, "bold", "white", "middle"))
    for name, yy in pin_marks:
        o.append(f'<rect x="{px-20}" y="{yy-9}" width="18" height="18" rx="2" fill="#111"/>')
        o.append(T(px - 28, yy + 6, name, 17, "bold", "white", "end"))
        h = HDR.get(name, "")
        if h: o.append(T(bx + 14, yy + 5, h, 12, fill="#b9d3ee"))
    o += elems
    o.append(line([(gnd_x, min(taps)), (gnd_x, gy), (px, gy)], "#222", 5))
    for t in taps: o.append(circle(gnd_x, t, 5, "#222"))
    o.append(T(gnd_x - 10, gy + 30, "공통 GND", 16, "bold", "#222", "end"))
    o.append(T(40, bot + 34, foot, 14, fill="#555"))
    return fname, svg(W, bot + 52, o)


def sch_ex1():
    return schematic("ex1_led_schematic_ko.svg", "예제 1 · GPIO25 LED 출력 — 회로도",
                     "digitalWrite(25, HIGH) → 3.3V가 저항·LED를 거쳐 GND로 흐른다. 빨간 LED·220Ω이면 I ≈ (3.3−2.0)/220 ≈ 6mA",
                     [{"kind": "head", "label": "출력 — GPIO → 저항 → LED(+) … LED(−) → GND"},
                      {"kind": "led", "pin": "GPIO25", "color": C["red"], "label": "LED1 (긴 다리가 +)"}],
                     "저항은 LED의 앞(+쪽)이나 뒤(−쪽) 어디에 넣어도 전류 제한 효과는 같다. 단, 반드시 하나 이상 직렬로 들어가야 한다.")


def sch_ex2():
    return schematic("ex2_button_schematic_ko.svg", "예제 2 · 버튼 입력(GPIO18) + LED(GPIO25) — 회로도",
                     "pinMode(18, INPUT_PULLUP): 버튼을 놓으면 HIGH(1), 누르면 GND와 연결되어 LOW(0)",
                     [{"kind": "head", "label": "출력 — LED"},
                      {"kind": "led", "pin": "GPIO25", "color": C["red"], "label": "LED1 (토글 표시)"},
                      {"kind": "head", "label": "입력 — 버튼 (GPIO ↔ 버튼 ↔ GND)"},
                      {"kind": "btn", "pin": "GPIO18", "color": "#b8322f", "label": "버튼 RUN (누르면 LOW)"}],
                     "버튼 반대쪽을 3V3에 연결하지 않는다. INPUT_PULLUP 회로에서는 버튼이 GND로만 연결된다.")


def sch_ex3():
    return schematic("ex3_adc_pwm_schematic_ko.svg", "예제 3 · 가변저항 ADC 입력(GPIO34) → LED PWM(GPIO25) — 회로도",
                     "analogRead(34) 0~4095  →  map(…, 0, 255)  →  analogWrite(25, duty)",
                     [{"kind": "head", "label": "입력 — 가변저항 전압 분배 (ADC1)"},
                      {"kind": "pot", "pin": "GPIO34", "label": "가변저항 10kΩ (B형)"},
                      {"kind": "head", "label": "출력 — PWM으로 LED 밝기 조절"},
                      {"kind": "led", "pin": "GPIO25", "color": C["red"], "label": "LED1 (PWM 밝기)"}],
                     "가변저항 양 끝 중 어느 쪽을 3V3로 하느냐에 따라 회전 방향과 값 증가 방향만 바뀐다. 가운데 단자를 3V3/GND에 꽂으면 단락 위험.")


def sch_ex4():
    return schematic("ex4_integrated_schematic_ko.svg", "예제 4 · 복합 이벤트 컨트롤러 — 회로도",
                     "버튼 2 · LED 2 · 가변저항 1 · 수동 부저 1. 모든 GND는 하나로 연결된다.",
                     [{"kind": "head", "label": "출력 — LED"},
                      {"kind": "led", "pin": "GPIO25", "color": C["red"], "label": "LED1 · ADC 값 → PWM 밝기"},
                      {"kind": "led", "pin": "GPIO26", "color": C["green"], "label": "LED2 · RUN 중 400ms 점멸"},
                      {"kind": "head", "label": "입력 — 버튼 (INPUT_PULLUP)"},
                      {"kind": "btn", "pin": "GPIO18", "color": "#b8322f", "label": "버튼 RUN · 실행/정지"},
                      {"kind": "btn", "pin": "GPIO19", "color": "#1b6e48", "label": "버튼 MODE · 경보 ON/OFF"},
                      {"kind": "head", "label": "입력 — 가변저항 (ADC1)"},
                      {"kind": "pot", "pin": "GPIO34", "label": "가변저항 10kΩ"},
                      {"kind": "head", "label": "출력 — 수동 부저 (톤)"},
                      {"kind": "buz", "pin": "GPIO27", "label": "수동 부저 (+ 표시가 신호 쪽)"}],
                     "부저 정격이 3.3V·소전류가 아니면 GPIO27에 직결하지 말고 '부저 구동 회로(NPN)' 그림처럼 트랜지스터를 사용한다.")


# ================================================================ 부저 NPN 구동 회로
def buzzer_driver():
    W, Hh_ = 1240, 640
    o = []
    title(o, "선택 · 수동 부저 NPN 트랜지스터 구동 회로",
          "부저(특히 자석식)가 GPIO 허용 전류보다 많이 먹거나 5V 정격일 때 사용한다. GPIO는 트랜지스터의 베이스만 켠다.")
    # 왼쪽: 회로
    o.append(rect(40, 110, 700, 500))
    bx = 80
    o.append(rect(bx, 330, 150, 60, C["board"], C["boardS"], 2, 8))
    o.append(T(bx + 75, 366, "GPIO27", 18, "bold", "white", "middle"))
    o.append(line([(bx + 150, 360), (300, 360)], C["purple"], 4))
    o += sym_resistor_h(300, 400, 360, "1kΩ")
    o.append(line([(395, 360), (470, 360)], C["purple"], 4))
    # NPN
    cx, cy = 500, 360
    o.append(circle(cx + 10, cy, 40, "none", "#222", 2.5))
    o.append(line([(470, cy), (cx, cy)], "#222", 3))
    o.append(line([(cx, cy - 26), (cx, cy + 26)], "#222", 5))
    o.append(line([(cx, cy - 12), (cx + 30, cy - 34), (cx + 30, 250)], "#222", 3))
    o.append(line([(cx, cy + 12), (cx + 30, cy + 34), (cx + 30, 470)], "#222", 3))
    o += arrow(cx + 12, cy + 21, cx + 28, cy + 33, "#222", 3)
    o.append(T(cx - 6, cy - 34, "B", 14, "bold", "#333", "end"))
    o.append(T(cx + 40, cy - 40, "C", 14, "bold", "#333"))
    o.append(T(cx + 40, cy + 52, "E", 14, "bold", "#333"))
    o.append(T(cx - 20, cy + 70, "NPN (2N2222 · S8050 · BC547 등)", 14, fill="#444", anchor="end"))
    # 부저 + 다이오드
    o += sym_buzzer(cx + 30, 210, 24)
    o.append(line([(cx + 30, 186), (cx + 30, 150), (cx + 30, 140)], C["red"], 4))
    o.append(T(cx + 30, 132, "5V 핀(또는 3V3) — 부저 정격 전압", 14, "bold", C["red"], "middle"))
    o.append(line([(cx + 30, 234), (cx + 30, 250)], "#222", 3))
    # 플라이백 다이오드 (부저와 병렬)
    dx = cx + 110
    o.append(line([(cx + 30, 160), (dx, 160), (dx, 192)], "#555", 2.5))
    o.append(f'<path d="M{dx-12} 222 L{dx+12} 222 L{dx} 196 Z" fill="#555"/>')
    o.append(line([(dx - 12, 196), (dx + 12, 196)], "#555", 3))
    o.append(line([(dx, 222), (dx, 250), (cx + 30, 250)], "#555", 2.5))
    o.append(T(dx + 20, 205, "1N4148/1N4001", 13, fill="#444"))
    o.append(T(dx + 20, 224, "(자석식 부저일 때)", 13, fill="#444"))
    o += sym_gnd(cx + 30, 470)
    o.append(T(cx + 50, 490, "공통 GND", 14, "bold", "#222"))
    # 오른쪽 설명
    x = 770
    o.append(rect(x, 110, 430, 500, "#f3f4f6", "#9aa3ad"))
    lines = [("동작", ""), ("", "GPIO27 HIGH → 베이스 전류 → C–E 도통"), ("", "→ 부저에 전류가 흐른다 (트랜지스터가 스위치)"),
             ("부품", ""), ("", "베이스 저항 1kΩ: GPIO 전류 ≈ (3.3−0.7)/1k ≈ 2.6mA"),
             ("", "다이오드: 코일 역기전력 흡수 (캐소드 띠 → +쪽)"),
             ("언제", ""), ("", "부저 데이터시트 전류 > 약 10~20mA"), ("", "5V 정격 부저 · 자석식(전자기) 부저"),
             ("주의", ""), ("", "5V를 GPIO에 연결하지 않는다 (부저 쪽만 5V)"),
             ("", "NPN 핀 순서(E-B-C)는 품번마다 다르다 → 데이터시트")]
    yy = 150
    for h, s in lines:
        if h:
            o.append(T(x + 20, yy, h, 17, "bold", C["blue"])); yy += 28
        else:
            o.append(T(x + 30, yy, "• " + s, 15)); yy += 30
    return "ex4_buzzer_npn_driver_ko.svg", svg(W, Hh_, o)


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    for f in (bb_ex1, bb_ex2, bb_ex3, bb_ex4, sch_ex1, sch_ex2, sch_ex3, sch_ex4, buzzer_driver):
        n, s = f()
        open(os.path.join(OUT, n), "w", encoding="utf-8").write(s)
        print(n)
