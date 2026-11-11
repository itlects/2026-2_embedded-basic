# -*- coding: utf-8 -*-
"""10주차 그림 공통 부품: 실제 헤더 순서의 ESP32-DevKitC V4, DHT11(모듈/단품), 부저, LED, DS18B20, LCD."""
from figlib import *

MONO = "Consolas,'DejaVu Sans Mono',monospace"
CD, CW1, CI2 = "#c78a00", "#7b4bb3", "#208657"   # DHT 단일선, 1-Wire, I2C 색


def mono(x, y, s, size=14, fill="#1d2433", anchor="start", w="normal"):
    return (f'<text x="{x}" y="{y}" font-size="{size}" font-weight="{w}" font-family="{MONO}" '
            f'fill="{fill}" text-anchor="{anchor}" xml:space="preserve">{esc(s)}</text>')


# Espressif 공식 J2/J3 표 (USB가 아래, 안테나가 위일 때 J2=왼쪽, J3=오른쪽, 1번=위)
J2 = ["3V3", "EN", "VP·36", "VN·39", "IO34", "IO35", "IO32", "IO33", "IO25", "IO26",
      "IO27", "IO14", "IO12", "GND", "IO13", "D2", "D3", "CMD", "5V"]
J3 = ["GND", "IO23", "IO22", "TX·1", "RX·3", "IO21", "GND", "IO19", "IO18", "IO5",
      "IO17", "IO16", "IO4", "IO0", "IO2", "IO15", "D1", "D0", "CLK"]
FLASH = {"D2", "D3", "CMD", "D1", "D0", "CLK"}
STRAP = {"IO0", "IO2", "IO12", "IO15", "IO5"}
INONLY = {"VP·36", "VN·39", "IO34", "IO35"}


def devkit(o, x, y, used, pitch=30, W=330, note=True):
    """실제 핀 순서의 DevKitC V4. used = {핀라벨: (색, 설명)} (J2/J3 중복 GND는 'GND@J2', 'GND@J3a', 'GND@J3b').
    반환: {'J2-1': (x,y), ..., 'IO4': (x,y) ...} 헤더 바깥쪽 연결점."""
    top_pin = y + 175
    H = 175 + 19 * pitch + 95
    o.append(rect(x, y, W, H, C["board"], C["boardS"], 3, 16))
    # WROOM 모듈 + 안테나
    o.append(rect(x + 55, y + 14, W - 110, 140, "#c9ccd1", "#80858d", 2, 6))
    o.append(rect(x + 55, y + 14, W - 110, 36, "#e6e8eb", "#80858d", 2, 6))
    zz = " ".join(f"L{x+75+i*18} {y+(22 if i%2 else 42)}" for i in range(11))
    o.append(f'<path d="M{x+75} {y+42} {zz}" stroke="#b08a2a" stroke-width="3" fill="none"/>')
    o.append(T(x + W / 2, y + 88, "ESP32-WROOM-32", 16, "bold", "#333", "middle"))
    o.append(T(x + W / 2, y + 112, "안테나는 위쪽", 12, fill="#555", anchor="middle"))
    o.append(T(x + W / 2, y + 140, "ESP32-DevKitC V4", 17, "bold", "#1d3f66", "middle"))
    pos = {}
    for side, names, hx in (("J2", J2, x + 14), ("J3", J3, x + W - 14)):
        gcount = 0
        for i, nm in enumerate(names):
            py = top_pin + i * pitch
            key = nm
            if nm == "GND":
                key = "GND@J2" if side == "J2" else ("GND@J3a" if gcount == 0 else "GND@J3b")
                if side == "J3": gcount += 1
            u = used.get(key) or (used.get(nm) if nm != "GND" else None)
            fill = "#9aa3ad" if nm in FLASH else ("#111" if not u else u[0])
            o.append(f'<rect x="{hx-9}" y="{py-9}" width="18" height="18" rx="2" fill="{fill}" stroke="#000" stroke-width="1"/>')
            o.append(circle(hx, py, 3.5, "#d4af37"))
            tcol = "#8fa6bf" if nm in FLASH else ("white" if not u else "#fff")
            tw = "bold" if u else "normal"
            if side == "J2":
                if u: o.append(f'<rect x="{hx+14}" y="{py-12}" width="{len(nm)*10+16}" height="24" rx="12" fill="{u[0]}"/>')
                o.append(T(hx + 22, py + 5, nm, 14, tw, tcol))
                if not u: o.append(T(x - 8, py + 4, f"{i+1}", 11, fill="#8a8f98", anchor="end"))
            else:
                if u: o.append(f'<rect x="{hx-14-len(nm)*10-16}" y="{py-12}" width="{len(nm)*10+16}" height="24" rx="12" fill="{u[0]}"/>')
                o.append(T(hx - 22, py + 5, nm, 14, tw, tcol, "end"))
                if not u: o.append(T(x + W + 8, py + 4, f"{i+1}", 11, fill="#8a8f98"))
            pt = (x if side == "J2" else x + W, py)
            pos[f"{side}-{i+1}"] = pt
            pos[key] = pt
    o.append(T(x + 6, top_pin - 26, "J2", 14, "bold", "#dce9f7"))
    o.append(T(x + W - 6, top_pin - 26, "J3", 14, "bold", "#dce9f7", "end"))
    yb = top_pin + 19 * pitch
    o.append(rect(x + W / 2 - 34, y + H - 40, 68, 40, "#b8bcc2", "#6b7078", 2, 4))
    o.append(T(x + W / 2, y + H - 46, "Micro-USB (전원·업로드·시리얼)", 12, fill="#dce9f7", anchor="middle"))
    o.append(rect(x + 30, yb - 6, 40, 22, "#444", "#222", 1, 3)); o.append(T(x + 50, yb + 30, "EN", 11, fill="#dce9f7", anchor="middle"))
    o.append(rect(x + W - 70, yb - 6, 40, 22, "#444", "#222", 1, 3)); o.append(T(x + W - 50, yb + 30, "BOOT", 11, fill="#dce9f7", anchor="middle"))
    pos["_H"] = H
    return pos


def dht11_module(o, x, y, label="DHT11 모듈", order=("S", "+", "−"), pins_down=True):
    """3핀 모듈. (x,y)=좌상단, 폭 130, 높이 170. 반환 핀 끝 좌표(아래)."""
    o.append(rect(x, y, 130, 170, "#2a7fc4", "#14507e", 2, 8))
    o.append(rect(x + 18, y + 12, 94, 86, "#3d9be0", "#1f6aa8", 2, 6))
    for r in range(4):
        for c in range(4):
            o.append(f'<rect x="{x+26+c*21}" y="{y+19+r*19}" width="14" height="13" rx="2" fill="#d8eefc"/>')
    o.append(T(x + 65, y + 116, label, 13, "bold", "white", "middle"))
    o.append(f'<rect x="{x+20}" y="{y+124}" width="30" height="12" rx="2" fill="#222"/>')
    o.append(T(x + 56, y + 135, "10kΩ 풀업", 10, fill="#dce9f7"))
    pos = {}
    for k, lab in enumerate(order):
        px = x + 35 + k * 30
        o.append(T(px, y + 160, lab, 14, "bold", "white", "middle"))
        o.append(line([(px, y + 170), (px, y + 196)], "#a9a9a9", 4)); o.append(circle(px, y + 199, 5, "#d4af37"))
        pos[lab] = (px, y + 199)
    return pos


def dht11_bare(o, x, y, label="DHT11 단품(4핀)"):
    """4핀 단품. 폭 110, 높이 140. 핀 1~4 왼→오."""
    o.append(rect(x, y, 110, 140, "#3d9be0", "#1f6aa8", 2, 6))
    for r in range(5):
        for c in range(4):
            o.append(f'<rect x="{x+12+c*23}" y="{y+10+r*20}" width="16" height="14" rx="2" fill="#d8eefc"/>')
    o.append(T(x + 55, y + 128, label, 12, "bold", "white", "middle"))
    pos = {}
    for k, lab in enumerate(["VCC", "DATA", "NC", "GND"]):
        px = x + 19 + k * 24
        o.append(line([(px, y + 140), (px, y + 176)], "#a9a9a9", 4)); o.append(circle(px, y + 179, 5, "#d4af37"))
        o.append(T(px, y + 198, str(k + 1), 12, "bold", "#333", "middle"))
        pos[lab] = (px, y + 179)
    return pos


def buzzer_part(o, cx, cy, label="수동 부저"):
    o.append(circle(cx, cy, 40, "#2b2b2b", "#000", 2)); o.append(circle(cx, cy, 7, "#555"))
    o.append(T(cx - 22, cy - 14, "+", 18, "bold", "#ff6b6b", "middle"))
    o.append(T(cx, cy - 52, label, 14, "bold", C["purple"], "middle"))
    return {"+": (cx - 14, cy + 40), "−": (cx + 14, cy + 40)}


def led_part(o, cx, cy, color=C["red"], label="LED"):
    o.append(f'<path d="M{cx-16} {cy+10} L{cx-16} {cy-14} A16 16 0 0 1 {cx+16} {cy-14} L{cx+16} {cy+10} Z" fill="{color}" fill-opacity="0.85" stroke="#333" stroke-width="1.5"/>')
    o.append(f'<rect x="{cx-19}" y="{cy+10}" width="38" height="6" fill="{color}" stroke="#333"/>')
    o.append(line([(cx - 8, cy + 16), (cx - 8, cy + 62)], "#a9a9a9", 3))
    o.append(line([(cx + 8, cy + 16), (cx + 8, cy + 48)], "#a9a9a9", 3))
    o.append(T(cx - 22, cy + 40, "+", 15, "bold", "#c62828", "middle"))
    o.append(T(cx + 22, cy + 40, "−", 15, "bold", "#333", "middle"))
    o.append(T(cx, cy - 40, label, 14, "bold", color, "middle"))
    return {"A": (cx - 8, cy + 62), "K": (cx + 8, cy + 48)}


def inline_res(o, a, b, label, vertical=False):
    """두 점 사이 직선 위 저항(색띠)."""
    (x1, y1), (x2, y2) = a, b
    mx, my = (x1 + x2) / 2, (y1 + y2) / 2
    if vertical:
        o.append(f'<rect x="{mx-9}" y="{my-26}" width="18" height="52" rx="7" fill="#e9cf9a" stroke="#8a6a2a" stroke-width="1.5"/>')
        for i, c in enumerate(("#6d4c2f", "#111", "#e07b16", "#c9a227")):
            o.append(f'<rect x="{mx-9}" y="{my-18+i*10}" width="18" height="4" fill="{c}"/>')
        o.append(T(mx + 16, my + 5, label, 13, "bold", "#8a5818"))
    else:
        o.append(f'<rect x="{mx-26}" y="{my-9}" width="52" height="18" rx="7" fill="#e9cf9a" stroke="#8a6a2a" stroke-width="1.5"/>')
        for i, c in enumerate(("#e07b16", "#e07b16", "#6d4c2f", "#c9a227")):
            o.append(f'<rect x="{mx-18+i*10}" y="{my-9}" width="4" height="18" fill="{c}"/>')
        o.append(T(mx, my - 14, label, 13, "bold", "#8a5818", "middle"))


def ds18b20(o, cx, y, label="DS18B20"):
    o.append(f'<path d="M{cx-34} {y+70} L{cx-34} {y+34} A34 34 0 0 1 {cx+34} {y+34} L{cx+34} {y+70} Z" fill="#222"/>')
    o.append(T(cx, y + 50, label, 11, "bold", "white", "middle"))
    pos = {}
    for lab, dx in (("GND", -20), ("DQ", 0), ("VDD", 20)):
        o.append(line([(cx + dx, y + 70), (cx + dx, y + 104)], "#a9a9a9", 3)); o.append(circle(cx + dx, y + 107, 4.5, "#d4af37"))
        o.append(T(cx + dx, y + 124, lab, 10, "bold", "#333", "middle"))
        pos[lab] = (cx + dx, y + 107)
    return pos


def lcd1602(o, x, y):
    o.append(rect(x, y, 330, 120, "#1f5f2a", "#0f3a16", 2, 8))
    o.append(rect(x + 20, y + 22, 290, 76, "#9ed36a", "#4a7a2a", 2, 4))
    o.append(mono(x + 34, y + 54, "A22.5 B28.0 C", 19, "#123", w="bold"))
    o.append(mono(x + 34, y + 84, "DHT24.0C  45%", 19, "#123", w="bold"))
    o.append(T(x + 165, y + 140, "LCD1602 + I²C 백팩(PCF8574, 0x27)", 13, "bold", "#1f5f2a", "middle"))
    pos = {}
    for k, lab in enumerate(["GND", "VCC", "SDA", "SCL"]):
        px = x + 60 + k * 70
        o.append(circle(px, y, 6, "#d4af37", "#7a5d00", 1.5)); o.append(T(px, y + 18, lab, 11, "bold", "white", "middle"))
        pos[lab] = (px, y)
    return pos


def W(o, pts, color, w=4):
    o.append(line(pts, color, w)); o.append(dot(*pts[-1], color, 4.5)); o.append(dot(*pts[0], color, 4.5))


def junction(o, x, y, color):
    o.append(circle(x, y, 6, color, "white", 1.5))


def table(o, x, y, head, rows, widths, title_s=None, size=14):
    tw = sum(widths)
    hh = 34
    if title_s:
        o.append(T(x, y - 14, title_s, 17, "bold", C["ink"]))
    o.append(rect(x, y, tw, hh * (len(rows) + 1), "white", "#9aa3ad", 1.5, 8))
    o.append(f'<rect x="{x}" y="{y}" width="{tw}" height="{hh}" rx="8" fill="#e8edf3"/>')
    xs = [x]
    for wv in widths[:-1]: xs.append(xs[-1] + wv)
    for j, s in enumerate(head): o.append(T(xs[j] + 12, y + 23, s, size, "bold", C["ink"]))
    for i, r in enumerate(rows):
        yy = y + hh * (i + 1)
        o.append(line([(x, yy), (x + tw, yy)], "#d5dae0", 1))
        for j, s in enumerate(r):
            col = "#222"
            if isinstance(s, tuple): s, col = s
            o.append(T(xs[j] + 12, yy + 23, s, size, "bold" if j == 0 else "normal", col))
    return y + hh * (len(rows) + 1)


def digital(x0, y, bits, w, h, color, lw=3):
    pts = []
    for i, b in enumerate(bits):
        yy = y - h / 2 if b else y + h / 2
        xa = x0 + i * w
        if pts and pts[-1][1] != yy: pts.append((xa, yy))
        elif not pts: pts.append((xa, yy))
        pts.append((xa + w, yy))
    return [line(pts, color, lw, cap="butt")]
