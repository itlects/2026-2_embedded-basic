# -*- coding: utf-8 -*-
"""10주차 예제 실체 배선도(실제 J2/J3 핀 위치) + 회로도. 사용: python3 gen_wiring.py OUTDIR"""
import sys, os
from parts10 import *

OUT = sys.argv[1] if len(sys.argv) > 1 else "."
RED, BLK, YEL, PUR, GRN = C["v33"], "#222", "#d9a400", C["purple"], C["green"]


def wiring(fname, ttl, sub, buzzer=False, led=False, steps=(), extra_rows=()):
    Wd = 1400
    o = [T(40, 48, ttl, 28, "bold", C["ink"]), T(40, 80, sub, 16, fill=C["sub"])]
    bx, by = 470, 150
    used = {"3V3": (RED, ""), "GND@J3a": (BLK, ""), "IO4": (YEL, "")}
    if buzzer: used.update({"IO25": (PUR, ""), "GND@J2": (BLK, "")})
    if led: used.update({"IO26": (GRN, ""), "GND@J2": (BLK, "")})
    p = devkit(o, bx, by, used)
    H_board = p["_H"]
    # ---- DHT11 모듈 (오른쪽)
    m = dht11_module(o, 1040, 360)
    io4, g3, v33 = p["IO4"], p["GND@J3a"], p["3V3"]
    W(o, [io4, (m["S"][0], io4[1]), m["S"]], YEL)
    W(o, [g3, (1320, g3[1]), (1320, 640), (m["−"][0], 640), m["−"]], BLK)
    W(o, [v33, (440, v33[1]), (440, 124), (1290, 124), (1290, 610), (m["+"][0], 610), m["+"]], RED)
    o.append(T(m["S"][0] - 10, io4[1] - 10, "DATA → IO4 (J3-13)", 14, "bold", "#9a7400", "end"))
    o.append(T(1278, 145, "3V3 (J2-1) → VCC(+)", 14, "bold", RED, "end"))
    o.append(T(1278, g3[1] - 10, "GND (J3-1) → GND(−)", 14, "bold", BLK, "end"))
    o.append(T(1105, 352, "핀 순서는 모듈마다 다름 → 실크인쇄 확인", 13, "bold", "#c62828", "middle"))
    # ---- 왼쪽: 부저·LED 또는 4핀 단품 안내
    if buzzer:
        bz = buzzer_part(o, 200, 400)
        io25 = p["IO25"]; gl = p["GND@J2"]
        W(o, [io25, (bz["+"][0], io25[1]), bz["+"]], PUR)
        W(o, [gl, (300, gl[1]), (300, 470), (bz["−"][0], 470), bz["−"]], BLK)
        o.append(T(380, io25[1] - 8, "IO25 (J2-9)", 13, "bold", PUR, "middle"))
    if led:
        lp = led_part(o, 120, 610, C["red"], "경보 LED")
        io26 = p["IO26"]; gl = p["GND@J2"]
        a = lp["A"]
        o.append(line([io26, (300, io26[1])], GRN, 4)); o.append(dot(*io26, GRN, 4.5))
        o.append(line([(300, io26[1]), (300, 700), (a[0], 700), a], GRN, 4)); o.append(dot(*a, GRN, 4.5))
        inline_res(o, (300, io26[1]), (300, 700), "330Ω", vertical=True)
        W(o, [lp["K"], (lp["K"][0], 730), (340, 730), (340, gl[1]), gl], BLK)
        o.append(T(380, io26[1] + 22, "IO26 (J2-10)", 13, "bold", GRN, "middle"))
        o.append(T(380, gl[1] + 22, "GND (J2-14)", 13, "bold", BLK, "middle"))
    if not buzzer and not led:
        # 4핀 단품 사용 시 인셋
        o.append(rect(40, 300, 380, 470, "#fff8e8", "#e0b44a", 2, 12))
        o.append(T(60, 334, "4핀 단품 DHT11을 쓸 때", 17, "bold", "#8a5818"))
        bp = dht11_bare(o, 80, 360)
        vx, vy = bp["VCC"]; dx, dy = bp["DATA"]
        o.append(line([(vx, vy), (vx, 640)], RED, 4)); o.append(line([(dx, dy), (dx, 600), (260, 600)], YEL, 4))
        o.append(line([(bp["GND"][0], bp["GND"][1]), (bp["GND"][0], 690)], BLK, 4))
        o.append(line([(vx, 640), (300, 640)], RED, 4))
        o.append(line([(260, 600), (300, 600)], YEL, 4))
        inline_res(o, (280, 600), (280, 640), "", vertical=True)
        o.append(T(310, 605, "→ IO4", 14, "bold", "#9a7400")); o.append(T(310, 645, "→ 3V3", 14, "bold", RED))
        o.append(T(bp["GND"][0] + 10, 705, "→ GND", 14, "bold", BLK))
        o.append(T(250, 580, "10kΩ 풀업", 13, "bold", "#8a5818", "end"))
        o.append(T(60, 735, "1 VCC · 2 DATA · 3 NC(비움) · 4 GND", 13, fill="#444"))
        o.append(T(60, 756, "DATA–VCC 사이 4.7k~10kΩ 필수 (모듈은 내장)", 13, "bold", "#c62828"))
    # ---- 아래: 순서
    y0 = by + H_board + 40
    o.append(rect(40, y0, Wd - 80, len(steps) * 30 + 70, "#f3f4f6", "#9aa3ad", 2, 10))
    o.append(T(62, y0 + 32, "배선 순서 (USB를 뺀 상태에서) — 핀 번호는 J2/J3 위에서부터 1번", 17, "bold"))
    for i, (h, s) in enumerate(steps):
        o.append(T(62, y0 + 64 + i * 30, h, 15, "bold")); o.append(T(170, y0 + 64 + i * 30, s, 15))
    o.append(T(62, y0 + 64 + len(steps) * 30 - 2, "• 회색 핀(D0~D3·CMD·CLK)은 내부 플래시 연결 → 사용 금지  • 모든 GND는 보드 안에서 연결되어 있다", 13, fill="#555"))
    return fname, svg(Wd, y0 + len(steps) * 30 + 90, o)


def ex1():
    return wiring("ex1_dht11_wiring_ko.svg", "실습 1 · DHT11 → ESP32-DevKitC V4 실체 배선도",
                  "실제 헤더 위치로 그림: DATA는 오른쪽 J3-13(IO4), 전원은 왼쪽 J2-1(3V3), 접지는 오른쪽 J3-1(GND)",
                  steps=[("① 전원", "USB 분리 → DHT11 VCC(+) ← 3V3(J2-1)   ※ 5V 핀(J2-19) 아님"),
                         ("② 접지", "DHT11 GND(−) ← GND(J3-1)"),
                         ("③ 신호", "DHT11 DATA(S/OUT) ← IO4(J3-13)"),
                         ("④ 풀업", "3핀 모듈: 내장 확인 / 4핀 단품: DATA–3V3 사이 10kΩ 추가"),
                         ("⑤ 확인", "VCC–GND 단락 없음 확인 후 USB 연결 → 시리얼 115200")])


def ex2():
    return wiring("ex2_dht11_buzzer_wiring_ko.svg", "실습 2 · DHT11 + 수동 부저 실체 배선도",
                  "실습 1 배선 유지 + 부저(+) → IO25(J2-9), 부저(−) → GND(J2-14). 3.3V 저전류 피에조만 직결",
                  buzzer=True,
                  steps=[("① DHT11", "실습 1과 동일: 3V3(J2-1)·GND(J3-1)·IO4(J3-13)"),
                         ("② 부저 +", "부저의 + 표시(긴 다리) ← IO25(J2-9)"),
                         ("③ 부저 −", "부저 − ← GND(J2-14)"),
                         ("④ 확인", "능동/수동 확인: 3V 직접 연결 시 혼자 '삐-' 소리 = 능동형(이번 실습은 수동형)")])


def ex3():
    return wiring("ex3_dht11_buzzer_led_wiring_ko.svg", "실습 3·4 · 완성 회로 — DHT11 + 부저 + LED 실체 배선도",
                  "IO26(J2-10) → 330Ω → LED 애노드(+, 긴 다리) / LED 캐소드(−) → GND(J2-14). 센서·부저 배선은 유지",
                  buzzer=True, led=True,
                  steps=[("① DHT11", "3V3(J2-1) → VCC · GND(J3-1) → GND · IO4(J3-13) → DATA"),
                         ("② 부저", "IO25(J2-9) → 부저 + · 부저 − → GND(J2-14)"),
                         ("③ LED", "IO26(J2-10) → 330Ω → LED +(긴 다리) · LED −(짧은 다리, 평평한 면) → GND"),
                         ("④ 전류", "LED 전류 ≈ (3.3 − 2.0) V ÷ 330 Ω ≈ 4 mA  (GPIO 권장 범위 이내)"),
                         ("⑤ 확인", "USB 연결 전 3V3·GND 단락 여부, LED 방향, 부저 극성 재확인")])


# ================================================================ 회로도
def schematic_full(fname, ttl, sub, buzzer=False, led=False):
    Wd = 1300
    o = [T(40, 48, ttl, 28, "bold", C["ink"]), T(40, 80, sub, 16, fill=C["sub"])]
    bx, by, bw = 60, 130, 230
    rows = [("3V3", 190, RED), ("IO4", 330, YEL)]
    if buzzer: rows.append(("IO25", 470, PUR))
    if led: rows.append(("IO26", 570, GRN))
    gy = (rows[-1][1] + 110)
    o.append(rect(bx, by, bw, gy - by + 50, C["board"], C["boardS"], 3, 14))
    o.append(T(bx + bw / 2, by + 34, "ESP32-DevKitC V4", 18, "bold", "white", "middle"))
    hdr = {"3V3": "J2-1", "IO4": "J3-13", "IO25": "J2-9", "IO26": "J2-10", "GND": "J3-1/J2-14"}
    px = bx + bw
    for nm, yy, col in rows + [("GND", gy, BLK)]:
        o.append(f'<rect x="{px-20}" y="{yy-9}" width="18" height="18" rx="2" fill="#111"/>')
        o.append(T(px - 28, yy + 6, nm, 17, "bold", "white", "end"))
        o.append(T(bx + 14, yy + 5, hdr[nm], 12, fill="#b9d3ee"))
    # 3V3 레일
    o.append(line([(px, 190), (1000, 190)], RED, 4))
    o.append(T(330, 180, "3.3 V", 14, "bold", RED))
    # GND 레일
    o.append(line([(px, gy), (1000, gy)], BLK, 5))
    o.append(T(330, gy - 10, "GND (공통)", 14, "bold", BLK))
    # DHT11 블록
    dx = 820
    o.append(rect(dx, 260, 170, 150, "#e8f3fc", "#2a7fc4", 2.5, 10))
    o.append(T(dx + 85, 290, "DHT11", 18, "bold", "#1f6aa8", "middle"))
    o.append(T(dx + 85, 312, "온습도 센서", 13, fill="#333", anchor="middle"))
    for lab, yy in (("VCC", 340), ("DATA", 330 + 0), ("GND", 390)):
        pass
    # 핀: VCC(위), DATA(왼쪽), GND(아래)
    o.append(line([(dx + 60, 190), (dx + 60, 260)], RED, 4)); o.append(T(dx + 66, 252, "VCC", 12, "bold", RED))
    o.append(line([(dx + 60, 410), (dx + 60, gy)], BLK, 3.5)); o.append(T(dx + 66, 428, "GND", 12, "bold", BLK)); junction(o, dx + 60, gy, BLK)
    o.append(line([(px, 330), (dx, 330)], YEL, 4)); o.append(T(dx - 8, 322, "DATA", 12, "bold", "#9a7400", "end"))
    # 풀업
    rx = 620
    o.append(line([(rx, 190), (rx, 222)], RED, 3)); o.append(line([(rx, 298), (rx, 330)], YEL, 3))
    o += [f'<rect x="{rx-11}" y="222" width="22" height="76" rx="3" fill="#fff5da" stroke="#8a5818" stroke-width="2"/>']
    junction(o, rx, 190, RED); junction(o, rx, 330, YEL)
    o.append(T(rx + 18, 252, "R1 10kΩ", 14, "bold", "#8a5818")); o.append(T(rx + 18, 272, "풀업", 13, fill="#8a5818"))
    o.append(T(rx + 18, 292, "(모듈은 내장)", 12, fill="#666"))
    # 바이패스 콘덴서
    cx = 1080
    o.append(line([(1000, 190), (cx, 190), (cx, 280)], RED, 3)); o.append(line([(cx - 22, 280), (cx + 22, 280)], "#333", 3))
    o.append(line([(cx - 22, 292), (cx + 22, 292)], "#333", 3)); o.append(line([(cx, 292), (cx, gy), (1000, gy)], BLK, 3))
    o.append(T(cx + 30, 284, "C1 100nF", 13, "bold", "#333")); o.append(T(cx + 30, 302, "(권장, 센서 가까이)", 12, fill="#666"))
    o.append(T(380, 352, "단일 DATA선: MCU와 센서가 번갈아 LOW로 당김", 12, fill="#666"))
    if buzzer:
        y = 470
        o.append(line([(px, y), (704, y)], PUR, 4)); o += sym_buzzer(730, y)
        o.append(line([(756, y), (790, y), (790, gy)], BLK, 3)); junction(o, 790, gy, BLK)
        o.append(T(480, y - 30, "BZ1 수동 피에조 (3.3V 저전류형)", 14, "bold", PUR))
        o.append(T(330, y - 10, "tone(25, 2000) → 2kHz 사각파", 12, fill="#666"))
    if led:
        y = 570
        o.append(line([(px, y), (480, y)], GRN, 4)); o += sym_resistor_h(480, 580, y, "R2 330Ω")
        o.append(line([(580, y), (650, y)], GRN, 4)); o += sym_led_h(650, y, C["red"])
        o.append(line([(682, y), (720, y), (720, gy)], BLK, 3)); junction(o, 720, gy, BLK)
        o.append(T(650, y + 46, "D1 경보 LED", 14, "bold", C["red"]))
        o.append(T(380, y - 14, "HIGH = 점등", 12, fill="#666"))
    return fname, svg(Wd, gy + 80, o)


def sch1():
    return schematic_full("ex1_dht11_schematic_ko.svg", "실습 1 · DHT11 회로도",
                          "VCC=3.3V, DATA=IO4, DATA–VCC 10kΩ 풀업(평소 HIGH 유지), 100nF 바이패스는 전원 잡음 억제용")


def sch3():
    return schematic_full("ex3_dht11_buzzer_led_schematic_ko.svg", "실습 2~4 · DHT11 + 부저 + LED 회로도",
                          "입력 1(IO4) · 출력 2(IO25 부저, IO26 LED) · 모든 소자 GND 공통. 실습 2는 LED 가지를 빼고 연결",
                          buzzer=True, led=True)


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    for f in (ex1, ex2, ex3, sch1, sch3):
        n, s = f()
        open(os.path.join(OUT, n), "w", encoding="utf-8").write(s)
        print(n)
