# -*- coding: utf-8 -*-
"""중간고사 시험지용 그림 생성 (SVG). 사용: python3 gen_figs.py OUTDIR"""
import sys, os

OUT = sys.argv[1]
FONT = "'Noto Sans KR','Noto Sans CJK KR','Malgun Gothic',sans-serif"
C = dict(red="#d43b3b", green="#208657", blue="#2664bf", orange="#e07b16",
         purple="#7b4bb3", black="#222", gnd="#222", board="#2f6aa8",
         boardS="#1d3f66", bb="#f4f1e8", bbS="#cfc6ad", hole="#6b6658",
         gap="#e3dccb", yellow="#f2c230", rail_neg="#2a4fa0", rail_pos="#c63a3a")

def svg(w, h, body):
    return (f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" '
            f'viewBox="0 0 {w} {h}" font-family="{FONT}"><rect width="{w}" height="{h}" fill="white"/>'
            + "".join(body) + "</svg>")

def T(x, y, s, size=18, w="normal", fill="#222", anchor="start"):
    s = s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
    return f'<text x="{x}" y="{y}" font-size="{size}" font-weight="{w}" fill="{fill}" text-anchor="{anchor}">{s}</text>'

def line(pts, color, w=4, dash=None):
    d = "M" + " L".join(f"{x} {y}" for x, y in pts)
    da = f' stroke-dasharray="{dash}"' if dash else ""
    return f'<path d="{d}" stroke="{color}" stroke-width="{w}" fill="none" stroke-linejoin="round" stroke-linecap="round"{da}/>'

# ------------------------------------------------------------------ 핀 정의
BOARDS = {
    "uno": dict(name="Arduino UNO R3", file="uno", vlogic="5V 논리",
                led=["D3", "D4", "D5", "D6"], btn=["D7", "D8", "D9", "D10"], buz="D11",
                gnd=["GND", "GND"]),
    "esp32": dict(name="ESP32 DevKitC V4", file="esp32", vlogic="3.3V 논리 · 5V 인가 금지",
                  led=["GPIO16", "GPIO17", "GPIO18", "GPIO19"], btn=["GPIO21", "GPIO22", "GPIO23", "GPIO25"],
                  buz="GPIO26", gnd=["GND", "GND"]),
}
LEDC = [C["red"], C["green"], C["blue"], C["orange"]]
BTNC = ["#b8322f", "#1b6e48", "#1f529c", "#a85a0c"]

# ------------------------------------------------------------------ 브레드보드 배치
P = 18            # 구멍 간격(px)
NCOL = 63
BX, BY = 380, 200  # 브레드보드 좌상단
ROWS_TOP = "abcde"; ROWS_BOT = "fghij"

def col_x(c): return BX + 40 + (c - 1) * P
def row_y(r):
    # 위 레일 2줄(−, +) → a~e → 중앙 홈 → f~j → 아래 레일 2줄(−, +)
    order = {"T-": 0, "T+": 1}
    if r in order: return BY + 26 + order[r] * P
    if r in ROWS_TOP: return BY + 26 + 2 * P + 22 + ROWS_TOP.index(r) * P
    if r in ROWS_BOT: return BY + 26 + 2 * P + 22 + 5 * P + 34 + ROWS_BOT.index(r) * P
    if r == "B-": return row_y("j") + 22 + P
    if r == "B+": return row_y("j") + 22 + 2 * P
def H(r, c): return (col_x(c), row_y(r))

BB_W = 40 + (NCOL - 1) * P + 40
BB_H = row_y("B+") - BY + 26

# 레이아웃 (열 번호)
BTN_COL = [4, 10, 16, 22]          # 스위치 왼쪽 다리 열, 오른쪽 다리 = +2
LED_X = [33, 40, 47, 54]           # 신호선 열, 저항 c[X]~c[X+4], LED +: X+4, -: X+5
BUZ = (40, 43)                     # 부저 + 열, - 열 (아래쪽 절반, h줄)

def breadboard_base():
    b = []
    b.append(f'<rect x="{BX}" y="{BY}" width="{BB_W}" height="{BB_H}" rx="10" fill="{C["bb"]}" stroke="{C["bbS"]}" stroke-width="2"/>')
    # 중앙 홈
    gy = (row_y("e") + row_y("f")) / 2
    b.append(f'<rect x="{BX+10}" y="{gy-10}" width="{BB_W-20}" height="20" fill="{C["gap"]}"/>')
    b.append(T(BX + 16, gy + 6, "중앙 홈", 13, fill="#8a816a"))
    # 레일 선
    for r, col in (("T-", C["rail_neg"]), ("T+", C["rail_pos"]), ("B-", C["rail_neg"]), ("B+", C["rail_pos"])):
        y = row_y(r) + (-11 if r.endswith("-") and r[0] == "T" else 11 if r[0] == "T" else -11 if r == "B-" else 11)
        b.append(line([(BX + 30, y), (BX + BB_W - 30, y)], col, 2))
    b.append(T(BX + 12, row_y("T-") + 6, "−", 18, "bold", C["rail_neg"]))
    b.append(T(BX + 12, row_y("T+") + 6, "+", 18, "bold", C["rail_pos"]))
    b.append(T(BX + 12, row_y("B-") + 6, "−", 18, "bold", C["rail_neg"]))
    b.append(T(BX + 12, row_y("B+") + 6, "+", 18, "bold", C["rail_pos"]))
    # 구멍
    for r in ["T-", "T+", "B-", "B+"]:
        for c in range(1, NCOL + 1):
            if (c - 1) % 6 == 5: continue  # 레일 5개 묶음 간격
            x, y = H(r, c)
            b.append(f'<rect x="{x-3}" y="{y-3}" width="6" height="6" rx="1" fill="{C["hole"]}"/>')
    for r in ROWS_TOP + ROWS_BOT:
        b.append(T(BX + 22, row_y(r) + 4, r, 11, fill="#8a816a", anchor="middle"))
        for c in range(1, NCOL + 1):
            x, y = H(r, c)
            b.append(f'<rect x="{x-3}" y="{y-3}" width="6" height="6" rx="1" fill="{C["hole"]}"/>')
    for c in (1, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60):
        b.append(T(col_x(c), row_y("a") - 8, str(c), 11, fill="#8a816a", anchor="middle"))
        b.append(T(col_x(c), row_y("j") + 15, str(c), 11, fill="#8a816a", anchor="middle"))
    return b

def dot(x, y, color, r=5):
    return f'<circle cx="{x}" cy="{y}" r="{r}" fill="{color}" stroke="white" stroke-width="1.5"/>'

def tact(c):
    """중앙 홈을 가로지르는 4핀 택트 스위치: 다리 e[c], e[c+2], f[c], f[c+2]"""
    x1, y1 = H("e", c); x2, y2 = H("f", c + 2)
    o = [f'<rect x="{x1-12}" y="{y1-12}" width="{x2-x1+24}" height="{y2-y1+24}" rx="5" fill="#3a3a3a" stroke="#111" stroke-width="2"/>',
         f'<circle cx="{(x1+x2)/2}" cy="{(y1+y2)/2}" r="11" fill="#6d6d6d" stroke="#222" stroke-width="2"/>']
    for (r, cc) in (("e", c), ("e", c + 2), ("f", c), ("f", c + 2)):
        x, y = H(r, cc); o.append(f'<circle cx="{x}" cy="{y}" r="4" fill="#d9d9d9"/>')
    return o

def resistor(r, c1, c2):
    x1, y = H(r, c1); x2, _ = H(r, c2)
    o = [line([(x1, y), (x2, y)], "#9a9a9a", 3)]
    mx = (x1 + x2) / 2
    o.append(f'<rect x="{mx-24}" y="{y-8}" width="48" height="16" rx="7" fill="#e9cf9a" stroke="#8a6a2a" stroke-width="1.5"/>')
    for i, col in enumerate(["#c62828", "#c62828", "#6d4c2f", "#c9a227"]):
        o.append(f'<rect x="{mx-17+i*9 + (4 if i==3 else 0)}" y="{y-8}" width="4" height="16" fill="{col}"/>')
    o.append(dot(x1, y, "#9a9a9a", 3.5)); o.append(dot(x2, y, "#9a9a9a", 3.5))
    return o

def led(r, cp, cn, color):
    xp, y = H(r, cp); xn, _ = H(r, cn)
    mx = (xp + xn) / 2
    o = [line([(xp, y), (xp, y - 22)], "#9a9a9a", 3), line([(xn, y), (xn, y - 16)], "#9a9a9a", 3)]
    o.append(f'<path d="M{mx-11} {y-20} L{mx-11} {y-34} A11 11 0 0 1 {mx+11} {y-34} L{mx+11} {y-20} Z" fill="{color}" fill-opacity="0.85" stroke="#333" stroke-width="1.5"/>')
    o.append(f'<rect x="{mx-13}" y="{y-22}" width="26" height="4" fill="{color}" stroke="#333" stroke-width="1"/>')
    o.append(dot(xp, y, "#9a9a9a", 3.5)); o.append(dot(xn, y, "#9a9a9a", 3.5))
    o.append(T(xp - 3, y + 15, "+", 13, "bold", "#c62828", "middle"))
    return o

def buzzer(r, cp, cn):
    xp, y = H(r, cp); xn, _ = H(r, cn)
    mx = (xp + xn) / 2
    o = [f'<circle cx="{mx}" cy="{y}" r="27" fill="#2b2b2b" stroke="#000" stroke-width="2"/>',
         f'<circle cx="{mx}" cy="{y}" r="5" fill="#555"/>',
         T(mx - 15, y - 9, "+", 15, "bold", "#ff6b6b", "middle")]
    o.append(dot(xp, y, "#cfcfcf", 3.5)); o.append(dot(xn, y, "#cfcfcf", 3.5))
    return o

def board_block(bd, pins_y):
    """보드 블록: pins_y = {label: y}"""
    x, w = 40, 250
    top = min(pins_y.values()) - 64; bot = max(pins_y.values()) + 58
    o = [f'<rect x="{x}" y="{top}" width="{w}" height="{bot-top}" rx="16" fill="{C["board"]}" stroke="{C["boardS"]}" stroke-width="3"/>',
         T(x + w / 2, top + 36, bd["name"], 22, "bold", "white", "middle"),
         T(x + w / 2, bot - 18, "USB → PC  ·  " + bd["vlogic"], 13, fill="#dce9f7", anchor="middle")]
    for lab, y in pins_y.items():
        name = lab.split("#")[0]
        o.append(f'<rect x="{x+w-24}" y="{y-9}" width="18" height="18" rx="2" fill="#111"/>')
        o.append(T(x + w - 34, y + 6, name, 17, "bold", "white", "end"))
    return o, x + w - 15

def pill(x, y, s, color, size=14):
    w = len(s) * size * 0.62 + 14
    return [f'<rect x="{x-w/2}" y="{y-size+1}" width="{w}" height="{size+8}" rx="{(size+8)/2}" fill="white" stroke="{color}" stroke-width="1.5"/>',
            T(x, y + 2, s, size, "bold", color, "middle")]

def breadboard_figure(key):
    bd = BOARDS[key]
    W = BX + BB_W + 40
    body = [T(40, 48, f"{bd['name']} — 중간고사 브레드보드 배선도", 30, "bold"),
            T(40, 80, "구멍 좌표(예: a33)를 그대로 따라 꽂는다. 보드는 브레드보드 밖에 두고 점퍼선으로 연결한다.", 17, fill="#444")]
    body += breadboard_base()

    # 보드 핀 (위→아래): GND, LED4..LED1, 버튼1..4, 부저, GND
    top_pins = [("GND", "G1")] + [(bd["led"][i], f"L{i}") for i in (3, 2, 1, 0)]
    bot_pins = [(bd["btn"][i], f"B{i}") for i in range(4)] + [(bd["buz"], "Z"), ("GND", "G2")]
    pins_y = {}
    y = row_y("T-") + 4
    for name, k in top_pins:
        pins_y[f"{name}#{k}"] = y; y += 40
    y = row_y("f") + 34
    for name, k in bot_pins:
        pins_y[f"{name}#{k}"] = y; y += 40
    blk, px = board_block(bd, pins_y)
    body += blk

    wires = []
    # ---- 위쪽 묶음: 위 핀일수록 안쪽 세로선, 가장 높은 가로선, 가장 먼 열
    top_targets = [("G1", "T-", 62, C["gnd"])] + [(f"L{i}", "a", LED_X[i], LEDC[i]) for i in (3, 2, 1, 0)]
    for n, (k, row, col, color) in enumerate(top_targets):
        name = [p for p in pins_y if p.endswith("#" + k)][0]
        y = pins_y[name]
        lane_x = px + 14 + n * 11
        over = BY - 72 + n * 13
        tx, ty = H(row, col)
        wires.append(line([(px, y), (lane_x, y), (lane_x, over), (tx, over), (tx, ty)], color, 4))
        wires.append(dot(tx, ty, color))
    # ---- 아래쪽 묶음: 위 핀일수록 바깥 세로선, 가장 얕은 가로선, 가장 가까운 열
    bot_targets = [(f"B{i}", "j", BTN_COL[i] + 2, BTNC[i]) for i in range(4)] + \
                  [("Z", "j", BUZ[0], C["purple"]), ("G2", "B-", 62, C["gnd"])]
    nb = len(bot_targets)
    for n, (k, row, col, color) in enumerate(bot_targets):
        name = [p for p in pins_y if p.endswith("#" + k)][0]
        y = pins_y[name]
        lane_x = px + 14 + (nb - 1 - n) * 11
        under = BY + BB_H + 18 + n * 13
        tx, ty = H(row, col)
        wires.append(line([(px, y), (lane_x, y), (lane_x, under), (tx, under), (tx, ty)], color, 4))
        wires.append(dot(tx, ty, color))
    # ---- 브레드보드 위 짧은 점퍼 (GND)
    def jumper(r1, c1, r2, c2):
        (x1, y1), (x2, y2) = H(r1, c1), H(r2, c2)
        return [line([(x1, y1), (x2, y2)], C["gnd"], 4), dot(x1, y1, C["gnd"]), dot(x2, y2, C["gnd"])]
    for c in BTN_COL:
        wires += jumper("a", c, "T-", c)          # 스위치 왼쪽 위 다리 열 → 위 − 레일
    for X in LED_X:
        wires += jumper("a", X + 5, "T-", X + 5)  # LED − 열 → 위 − 레일
    wires += jumper("j", BUZ[1], "B-", BUZ[1])     # 부저 − → 아래 − 레일

    parts = []
    for i, c in enumerate(BTN_COL):
        parts += tact(c)
        x, _ = H("h", c + 1)
        parts += pill(x, row_y("h") + 4, f"버튼{i+1}", BTNC[i])
    for i, X in enumerate(LED_X):
        parts += resistor("c", X, X + 4)
        parts += led("b", X + 4, X + 5, LEDC[i])
        x, _ = H("e", X + 2)
        parts += pill(x, row_y("e") + 4, f"LED{i+1}", LEDC[i])
    parts += buzzer("h", BUZ[0], BUZ[1])
    x, _ = H("h", BUZ[1])
    parts += pill(x + 80, row_y("h") + 5, "수동 부저", C["purple"], 15)
    body += wires + parts

    # ---- 설명 상자
    last_under = BY + BB_H + 18 + (nb - 1) * 13
    by = max(last_under, max(pins_y.values()) + 60) + 34
    b = bd["btn"]; l = bd["led"]
    lines = [
        ("① 버튼", f"스위치를 중앙 홈에 걸쳐 e{BTN_COL[0]}·e{BTN_COL[0]+2}·f{BTN_COL[0]}·f{BTN_COL[0]+2}에 꽂는다(버튼2~4는 {BTN_COL[1]}·{BTN_COL[2]}·{BTN_COL[3]}열에서 시작)."),
        ("", f"입력선 {b[0]}→j{BTN_COL[0]+2}, {b[1]}→j{BTN_COL[1]+2}, {b[2]}→j{BTN_COL[2]+2}, {b[3]}→j{BTN_COL[3]+2}  ·  GND 점퍼 a{BTN_COL[0]}, a{BTN_COL[1]}, a{BTN_COL[2]}, a{BTN_COL[3]} → 위 − 레일 (대각선 다리 사용)"),
        ("② LED", f"신호선 {l[0]}→a{LED_X[0]}, {l[1]}→a{LED_X[1]}, {l[2]}→a{LED_X[2]}, {l[3]}→a{LED_X[3]}  ·  저항 c{LED_X[0]}–c{LED_X[0]+4} (LED2~4도 같은 모양으로 7칸씩 오른쪽)"),
        ("", f"LED 긴 다리(+) b{LED_X[0]+4}, 짧은 다리(−) b{LED_X[0]+5}  ·  GND 점퍼 a{LED_X[0]+5}, a{LED_X[1]+5}, a{LED_X[2]+5}, a{LED_X[3]+5} → 위 − 레일"),
        ("③ 부저", f"(+) h{BUZ[0]}, (−) h{BUZ[1]}  ·  신호선 {bd['buz']}→j{BUZ[0]}  ·  GND 점퍼 j{BUZ[1]} → 아래 − 레일"),
        ("④ GND", "보드 GND 한 가닥 → 위 − 레일 62열, 다른 한 가닥 → 아래 − 레일 62열  (+ 레일은 사용하지 않는다)"),
    ]
    body.append(f'<rect x="40" y="{by}" width="{W-80}" height="{len(lines)*30+56}" rx="10" fill="#f3f4f6" stroke="#9aa3ad" stroke-width="2"/>')
    for i, (h, s) in enumerate(lines):
        body.append(T(62, by + 34 + i * 30, h, 16, "bold"))
        body.append(T(140, by + 34 + i * 30, s, 16))
    body.append(T(62, by + 40 + len(lines) * 30, "• 보드 그림은 연결 관계만 보여 준다. 실제 핀 위치·순서는 보드에 인쇄된 이름으로 찾는다.   • 레일이 가운데에서 끊긴 브레드보드는 좌우 레일을 짧은 점퍼로 이어 준다.", 14, fill="#555"))
    Hh = by + len(lines) * 30 + 76
    return svg(W, Hh, body)

# ------------------------------------------------------------------ 회로 연결도(개략)
def schematic_figure(key):
    bd = BOARDS[key]
    W, Hh = 1240, 900
    o = [T(40, 48, f"{bd['name']} — 중간고사 회로 연결도", 30, "bold"),
         T(40, 80, "LED·버튼·부저가 각각 어느 핀에 어떻게 연결되는지 보여 주는 개략도이다. 모든 GND는 하나로 연결된다.", 17, fill="#444")]
    bx, bw, top, bot = 40, 230, 110, 848
    o.append(f'<rect x="{bx}" y="{top}" width="{bw}" height="{bot-top}" rx="16" fill="{C["board"]}" stroke="{C["boardS"]}" stroke-width="3"/>')
    o.append(T(bx + bw / 2, top + 38, bd["name"], 21, "bold", "white", "middle"))
    px = bx + bw
    gnd_x = 1120
    # LED
    o.append(T(330, 150, "출력 — LED (GPIO → 저항 → LED + … LED − → GND)", 18, "bold", "#333"))
    for i in range(4):
        y = 195 + i * 70
        o.append(f'<rect x="{px-20}" y="{y-9}" width="18" height="18" rx="2" fill="#111"/>')
        o.append(T(px - 28, y + 6, bd["led"][i], 17, "bold", "white", "end"))
        o.append(line([(px, y), (520, y)], LEDC[i], 4))
        o.append(f'<rect x="520" y="{y-12}" width="90" height="24" rx="4" fill="#fff5da" stroke="#8a5818" stroke-width="2"/>')
        o.append(T(565, y + 6, "220~330Ω", 14, anchor="middle"))
        o.append(line([(610, y), (700, y)], LEDC[i], 4))
        # LED 기호
        o.append(f'<path d="M700 {y-16} L700 {y+16} L732 {y} Z" fill="{LEDC[i]}" stroke="#222" stroke-width="2"/>')
        o.append(line([(732, y - 16), (732, y + 16)], "#222", 3))
        o.append(line([(712, y - 22), (724, y - 34)], "#222", 2)); o.append(line([(722, y - 20), (734, y - 32)], "#222", 2))
        o.append(T(690, y - 20, "+", 15, "bold", "#c62828", "end"))
        o.append(T(745, y - 18, "−", 16, "bold", "#333"))
        o.append(T(780, y + 6, f"LED{i+1}  (긴 다리가 +)", 16, "bold", LEDC[i]))
        o.append(line([(732, y), (760, y), (760, y + 22), (gnd_x, y + 22)], "#333", 3))
    # 버튼
    o.append(T(330, 500, "입력 — 버튼 (GPIO ↔ 버튼 ↔ GND, 내부 풀업 INPUT_PULLUP)", 18, "bold", "#333"))
    for i in range(4):
        y = 545 + i * 52
        o.append(f'<rect x="{px-20}" y="{y-9}" width="18" height="18" rx="2" fill="#111"/>')
        o.append(T(px - 28, y + 6, bd["btn"][i], 17, "bold", "white", "end"))
        o.append(line([(px, y), (560, y)], BTNC[i], 4))
        o.append(f'<circle cx="560" cy="{y}" r="5" fill="{BTNC[i]}"/>')
        o.append(line([(560, y), (600, y - 18)], "#222", 3))
        o.append(f'<circle cx="620" cy="{y}" r="5" fill="#222"/>')
        o.append(line([(620, y), (gnd_x, y)], "#333", 3))
        o.append(T(650, y - 8, f"버튼{i+1}  (누르면 LOW)", 16, "bold", BTNC[i]))
    # 부저
    yb = 132 + 0
    o.append(f'<rect x="880" y="470" width="300" height="0" fill="none"/>')
    ybz = 760
    o.append(f'<rect x="{px-20}" y="{ybz-9}" width="18" height="18" rx="2" fill="#111"/>')
    o.append(T(px - 28, ybz + 6, bd["buz"], 17, "bold", "white", "end"))
    o.append(line([(px, ybz), (700, ybz)], C["purple"], 4))
    o.append(f'<circle cx="730" cy="{ybz}" r="26" fill="#2b2b2b" stroke="#000" stroke-width="2"/>')
    o.append(T(712, ybz - 2, "+", 15, "bold", "#ff6b6b", "middle"))
    o.append(line([(756, ybz), (gnd_x, ybz)], "#333", 3))
    o.append(T(780, ybz - 10, "수동 부저 (+ 표시가 신호 쪽)", 16, "bold", C["purple"]))
    # GND
    o.append(line([(gnd_x, 217), (gnd_x, 820), (px - 10, 820)], "#222", 5))
    for yy in [217 + i * 70 for i in range(4)] + [545 + i * 52 for i in range(4)] + [ybz]:
        o.append(f'<circle cx="{gnd_x}" cy="{yy}" r="5" fill="#222"/>')
    o.append(f'<rect x="{px-20}" y="811" width="18" height="18" rx="2" fill="#111"/>')
    o.append(T(px - 28, 826, "GND", 17, "bold", "white", "end"))
    o.append(T(gnd_x - 10, 852, "공통 GND", 17, "bold", "#222", "end"))
    o.append(T(40, 885, "버튼은 외부 저항 없이 내부 풀업을 사용한다(평소 HIGH, 누르면 LOW).  LED마다 저항을 하나씩 반드시 직렬로 연결한다.", 15, fill="#555"))
    return svg(W, Hh, o)

# ------------------------------------------------------------------ 부품 안내
def parts_figure():
    W, Hh = 1240, 700
    o = [T(40, 48, "부품 확인 — 극성과 내부 연결", 30, "bold"),
         T(40, 80, "배선 전에 각 부품의 방향을 확인한다. 극성을 반대로 꽂으면 LED가 켜지지 않거나 부저가 울리지 않는다.", 17, fill="#444")]
    # 카드 틀
    cards = [(40, 110, "LED"), (340, 110, "저항 220~330Ω"), (640, 110, "4핀 택트 스위치"), (940, 110, "수동 부저")]
    for x, y, t in cards:
        o.append(f'<rect x="{x}" y="{y}" width="270" height="300" rx="12" fill="#fafafa" stroke="#c9ced6" stroke-width="2"/>')
        o.append(T(x + 135, y + 34, t, 20, "bold", anchor="middle"))
    # LED
    x, y = 175, 200
    o.append(f'<path d="M{x-22} {y+40} L{x-22} {y} A22 22 0 0 1 {x+22} {y} L{x+22} {y+40} Z" fill="{C["red"]}" fill-opacity="0.85" stroke="#333" stroke-width="2"/>')
    o.append(f'<rect x="{x-26}" y="{y+40}" width="52" height="7" fill="{C["red"]}" stroke="#333"/>')
    o.append(line([(x - 10, y + 47), (x - 10, y + 165)], "#999", 4))
    o.append(line([(x + 10, y + 47), (x + 10, y + 135)], "#999", 4))
    o.append(T(x - 18, y + 190, "긴 다리 = +", 16, "bold", "#c62828", "end"))
    o.append(T(x + 18, y + 160, "짧은 다리 = −", 16, "bold", "#333"))
    o.append(T(x + 30, y + 30, "평평한 테두리 쪽 = −", 13, fill="#555"))
    o.append(T(x, y + 220 - 10, "", 1))
    # 저항
    x, y = 475, 240
    o.append(line([(x - 110, y), (x + 110, y)], "#999", 4))
    o.append(f'<rect x="{x-55}" y="{y-20}" width="110" height="40" rx="16" fill="#e9cf9a" stroke="#8a6a2a" stroke-width="2"/>')
    for i, col in enumerate(["#c62828", "#c62828", "#6d4c2f"]):
        o.append(f'<rect x="{x-38+i*18}" y="{y-20}" width="9" height="40" fill="{col}"/>')
    o.append(f'<rect x="{x+28}" y="{y-20}" width="9" height="40" fill="#c9a227"/>')
    o.append(T(x, y + 60, "220Ω: 빨강-빨강-갈색-금", 15, anchor="middle"))
    o.append(T(x, y + 86, "330Ω: 주황-주황-갈색-금", 15, anchor="middle"))
    o.append(T(x, y + 120, "극성 없음 · LED마다 1개씩", 15, "bold", anchor="middle"))
    # 택트 스위치
    x, y = 775, 250
    o.append(f'<rect x="{x-60}" y="{y-60}" width="120" height="120" rx="8" fill="#3a3a3a" stroke="#111" stroke-width="2"/>')
    o.append(f'<circle cx="{x}" cy="{y}" r="26" fill="#6d6d6d" stroke="#222" stroke-width="2"/>')
    for dx, dy, n in ((-60, -45, "1"), (60, -45, "2"), (-60, 45, "3"), (60, 45, "4")):
        o.append(f'<rect x="{x+dx-(14 if dx<0 else 0)}" y="{y+dy-4}" width="14" height="8" fill="#bbb"/>')
        o.append(T(x + dx + (-22 if dx < 0 else 22), y + dy + 6, n, 15, "bold", anchor="middle"))
    o.append(line([(x - 46, y - 45), (x + 46, y - 45)], "#ffb347", 3, "6 4"))
    o.append(line([(x - 46, y + 45), (x + 46, y + 45)], "#ffb347", 3, "6 4"))
    o.append(T(x, y + 90, "1–2, 3–4는 항상 연결(점선)", 14, anchor="middle"))
    o.append(T(x, y + 112, "누르면 위·아래가 연결된다", 14, anchor="middle"))
    o.append(T(x, y + 136, "→ 대각선 1번·4번 다리 사용", 15, "bold", "#c62828", anchor="middle"))
    # 부저
    x, y = 1075, 230
    o.append(f'<circle cx="{x}" cy="{y}" r="48" fill="#2b2b2b" stroke="#000" stroke-width="2"/>')
    o.append(f'<circle cx="{x}" cy="{y}" r="8" fill="#555"/>')
    o.append(T(x - 26, y - 14, "+", 22, "bold", "#ff6b6b", "middle"))
    o.append(line([(x - 14, y + 48), (x - 14, y + 100)], "#999", 4)); o.append(line([(x + 14, y + 48), (x + 14, y + 100)], "#999", 4))
    o.append(T(x - 20, y + 125, "+", 18, "bold", "#c62828", "end")); o.append(T(x + 20, y + 125, "−", 18, "bold", "#333"))
    o.append(T(x, y + 150, "밑면에 기판이 보이면 수동 부저", 13, anchor="middle"))
    o.append(T(x, y + 170, "(스티커 붙은 밀봉형은 능동)", 13, fill="#555", anchor="middle"))
    # 브레드보드 내부 연결
    y0 = 450
    o.append(f'<rect x="40" y="{y0}" width="1170" height="220" rx="12" fill="#fafafa" stroke="#c9ced6" stroke-width="2"/>')
    o.append(T(60, y0 + 34, "브레드보드 내부 연결", 20, "bold"))
    bx0, by0 = 110, y0 + 60
    o.append(f'<rect x="{bx0}" y="{by0}" width="560" height="140" rx="8" fill="{C["bb"]}" stroke="{C["bbS"]}" stroke-width="2"/>')
    for i in range(24):
        o.append(f'<rect x="{bx0+30+i*21}" y="{by0+12}" width="6" height="6" fill="{C["hole"]}"/>')
    o.append(line([(bx0 + 25, by0 + 15), (bx0 + 540, by0 + 15)], C["rail_neg"], 3))
    for r in range(5):
        for i in range(24):
            o.append(f'<rect x="{bx0+30+i*21}" y="{by0+36+r*18}" width="6" height="6" fill="{C["hole"]}"/>')
    for i in (3, 9):
        o.append(f'<rect x="{bx0+26+i*21}" y="{by0+31}" width="14" height="88" rx="6" fill="none" stroke="#2e9d57" stroke-width="3"/>')
    o.append(T(bx0 - 50, by0 + 20, "레일", 14, fill="#555"))
    o.append(T(bx0 - 50, by0 + 80, "a~e", 14, fill="#555"))
    o.append(T(720, by0 + 20, "● 위·아래 레일(− / +)은 가로로 길게 연결된다.", 16))
    o.append(T(720, by0 + 52, "● 가운데 구멍은 세로 5칸(a~e 또는 f~j)씩 연결된다.", 16))
    o.append(T(720, by0 + 84, "● 중앙 홈 위(a~e)와 아래(f~j)는 서로 연결되지 않는다.", 16))
    o.append(T(720, by0 + 116, "● 같은 세로줄에 두 부품 다리를 꽂으면 서로 연결된다.", 16))
    return svg(W, Hh, o)

# ------------------------------------------------------------------ 동작 타이밍
def timing_figure():
    W, Hh = 1240, 920
    o = [T(40, 48, "동작 요구사항 — 시간 흐름으로 보기", 30, "bold"),
         T(40, 80, "한 칸 = 250ms. 버튼을 누르는 순간(0칸)에 첫 상태가 바로 나타나고, 이후 250ms마다 다음 칸으로 바뀐다.", 17, fill="#444")]
    cell, x0 = 62, 250
    def grid(y, title, rows, color, note):
        g = [T(40, y - 14, title, 19, "bold", color)]
        for k in range(12):
            g.append(T(x0 + k * cell + cell / 2, y + 2, f"{k*250}", 11, fill="#888", anchor="middle"))
        for r, (lab, seq) in enumerate(rows):
            yy = y + 12 + r * 30
            g.append(T(x0 - 12, yy + 19, lab, 15, anchor="end"))
            for k in range(12):
                on = seq[k % len(seq)]
                g.append(f'<rect x="{x0+k*cell+2}" y="{yy}" width="{cell-4}" height="24" rx="4" fill="{color if on else "#eceef1"}"/>')
        g.append(T(x0 + 12 * cell + 16, y + 60, note, 14, fill="#555"))
        return g
    def leds_seq(order):
        return [(f"LED{n}", [1 if order[k] == n else 0 for k in range(4)]) for n in (1, 2, 3, 4)]
    o += grid(140, "버튼1 · 정방향(FORWARD)", leds_seq([1, 2, 3, 4]), C["red"], "LED1→2→3→4→1… 반복")
    o += grid(320, "버튼2 · 역방향(REVERSE)", leds_seq([4, 3, 2, 1]), C["green"], "LED4→3→2→1→4… 반복")
    o += grid(500, "버튼3 · 동시 점멸(BLINK)", [(f"LED{n}", [1, 0]) for n in (1, 2, 3, 4)], C["blue"], "4개 ON/OFF 반복")
    # 멜로디: 버튼4 ON/OFF, 재생 중 LED 패턴 동시 동작
    y = 700
    o.append(T(40, y - 14, "버튼4 · 멜로디 ON/OFF — 켜면 반복 재생, 끄면 소리와 LED 모두 꺼짐", 19, "bold", C["purple"]))
    k_scale = cell / 250.0                       # px per ms (위 표와 같은 축)
    t_on, t_off = 0, 2500                        # 예시: 0ms에 ON, 2500ms에 OFF
    for k in range(12):
        o.append(T(x0 + k * cell + cell / 2, y + 2, f"{k*250}", 11, fill="#888", anchor="middle"))
    # 부저 줄
    yy = y + 12
    o.append(T(x0 - 12, yy + 23, "부저", 15, anchor="end"))
    o.append(f'<rect x="{x0}" y="{yy}" width="{12*cell}" height="34" rx="4" fill="#eceef1"/>')
    names = ["도", "레", "미", "파", "솔"]
    t = t_on; i = 0
    while t < t_off:
        w = min(250, t_off - t) * k_scale
        o.append(f'<rect x="{x0+t*k_scale+1}" y="{yy}" width="{w-2}" height="34" rx="4" fill="{C["purple"]}"/>')
        if w > 30: o.append(T(x0 + t * k_scale + w / 2, yy + 23, names[i % 5], 14, "bold", "white", "middle"))
        t += 290; i += 1
    # LED 줄 (예: 정방향이 실행 중)
    yy2 = yy + 44
    o.append(T(x0 - 12, yy2 + 19, "LED(정방향)", 14, anchor="end"))
    for k in range(12):
        on = (k * 250) < t_off
        lab = f"{k % 4 + 1}" if on else ""
        o.append(f'<rect x="{x0+k*cell+2}" y="{yy2}" width="{cell-4}" height="26" rx="4" fill="{C["red"] if on else "#eceef1"}"/>')
        if lab: o.append(T(x0 + k * cell + cell / 2, yy2 + 18, "LED" + lab, 12, "bold", "white", "middle"))
    # ON / OFF 표시
    xo = x0 + t_off * k_scale
    o.append(line([(x0, yy - 8), (x0, yy2 + 34)], "#333", 3, "5 4"))
    o.append(T(x0 + 4, yy2 + 52, "▲ 버튼4 ON", 14, "bold", C["purple"]))
    o.append(line([(xo, yy - 8), (xo, yy2 + 34)], "#c62828", 3, "5 4"))
    o.append(T(xo + 4, yy2 + 52, "▲ 버튼4 OFF → 소리 정지 + LED 모두 꺼짐", 14, "bold", "#c62828"))
    o.append(T(x0 + 12 * cell + 16, yy + 23, "도→레→미→파→솔→도… 반복", 14, fill="#555"))
    o.append(T(x0 + 12 * cell + 16, yy2 + 19, "멜로디와 함께 계속", 14, fill="#555"))
    o.append(T(x0, y + 132, "음 길이 200~300ms(그림은 250ms) · 음 사이 무음 30~50ms(그림은 40ms) · 4음 이상이면 곡은 자유", 14, fill="#555"))
    o.append(T(40, y + 168, "※ 버튼1~3: LED 방식 선택. 같은 버튼을 다시 누르면 일시정지(불빛 유지) → 한 번 더 누르면 이어서 진행. 다른 버튼은 즉시 전환.", 15, "bold", "#c62828"))
    o.append(T(40, y + 196, "※ 버튼4: 멜로디 ON/OFF. 멜로디가 나오는 동안에도 버튼1~3으로 LED 방식을 바꿀 수 있다.", 15, "bold", "#c62828"))
    return svg(W, Hh, o)

os.makedirs(OUT, exist_ok=True)
files = {
    "midterm_parts_guide_ko.svg": parts_figure(),
    "midterm_uno_schematic_ko.svg": schematic_figure("uno"),
    "midterm_esp32_schematic_ko.svg": schematic_figure("esp32"),
    "midterm_uno_breadboard_ko.svg": breadboard_figure("uno"),
    "midterm_esp32_breadboard_ko.svg": breadboard_figure("esp32"),
    "midterm_timing_ko.svg": timing_figure(),
}
for n, s in files.items():
    open(os.path.join(OUT, n), "w", encoding="utf-8").write(s)
    print(n)
