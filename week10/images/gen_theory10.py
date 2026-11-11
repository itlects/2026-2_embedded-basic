# -*- coding: utf-8 -*-
"""10주차 이론·Wokwi 그림. 사용: python3 gen_theory10.py OUTDIR"""
import sys, os
from parts10 import *

OUT = sys.argv[1] if len(sys.argv) > 1 else "."
RED, BLK, YEL, PUR, GRN, BLU, ORG = C["v33"], "#222", "#d9a400", C["purple"], C["green"], C["blue"], C["orange"]
MCU, SEN = "#2664bf", "#d06a00"     # 파형: MCU가 구동 / 센서가 구동


def seg(o, pts, col, w=3.5):
    o.append(line(pts, col, w, cap="butt"))


def dim(o, x1, x2, y, s, col="#333", size=12):
    o += arrow(x1 + (x2 - x1) / 2, y, x1, y, col, 1.5)
    o += arrow(x1 + (x2 - x1) / 2, y, x2, y, col, 1.5)
    o.append(T((x1 + x2) / 2, y - 6, s, size, "bold", col, "middle"))


# ================================================================ t0 핀맵
def pinmap():
    Wd, o = 1400, []
    title(o, "ESP32-DevKitC V4 — 10주차에 쓰는 핀 찾기 (실제 J2/J3 위치)",
          "USB를 아래로, 안테나를 위로 놓고 본 모습. 왼쪽 J2 · 오른쪽 J3, 각 열 위에서부터 1번")
    used = {"3V3": (RED, ""), "GND@J2": (BLK, ""), "GND@J3a": (BLK, ""), "IO4": (YEL, ""), "IO25": (PUR, ""),
            "IO26": (GRN, ""), "IO13": (CW1, ""), "IO21": (CI2, ""), "IO22": (BLU, ""), "5V": ("#a33", "")}
    p = devkit(o, 80, 120, used)
    rows = [("3V3", "J2-1", "센서 전원(3.3V)", "DHT11/DS18B20 VCC"),
            ("GND", "J2-14·J3-1·J3-7", "공통 접지", "모든 부품 GND"),
            ("IO4", "J3-13", "DHT11 DATA", "실습 1~5, Wokwi A·B·C"),
            ("IO25", "J2-9", "부저 출력(tone)", "실습 2~4, Wokwi A"),
            ("IO26", "J2-10", "LED 출력", "실습 3·4, Wokwi A"),
            ("IO13", "J2-15", "1-Wire 버스(DQ)", "Wokwi C · DS18B20"),
            ("IO21 / IO22", "J3-6 / J3-3", "I²C SDA / SCL", "Wokwi C · LCD1602"),
            ("5V", "J2-19", "USB 5V 출력", "LCD 백라이트 전원(Wokwi)")]
    ye = table(o, 500, 150, ["핀", "헤더 위치", "역할", "사용 실습"], rows, [140, 190, 210, 230], size=14)
    y = ye + 40
    o.append(T(500, y, "핀 선택 원칙", 18, "bold", C["ink"]))
    tips = [("회색", "#9aa3ad", "D0~D3·CMD·CLK(GPIO6~11) = 내부 플래시 → 절대 사용 금지"),
            ("주황", ORG, "IO0·IO2·IO5·IO12·IO15 = 부팅 모드 결정(스트래핑) → 센서 입력은 피함"),
            ("파랑", BLU, "IO34·35·36·39 = 입력 전용(출력·내부 풀업 없음) → 부저/LED/DHT에 부적합"),
            ("빨강", RED, "GPIO는 3.3V 논리. 5V 신호를 GPIO에 직접 넣지 않는다"),
            ("초록", GRN, "IO4·IO25·IO26·IO13 = 부팅에 영향 없는 일반 GPIO → 이번 주 선택 이유")]
    for i, (n, col, s) in enumerate(tips):
        yy = y + 34 + i * 34
        o.append(f'<rect x="500" y="{yy-16}" width="54" height="24" rx="12" fill="{col}"/>')
        o.append(T(527, yy + 1, n, 12, "bold", "white", "middle"))
        o.append(T(568, yy + 1, s, 14))
    y2 = y + 34 + 5 * 34 + 20
    o.append(rect(500, y2, 770, 120, "#fff4f4", "#e09090", 2, 10))
    o.append(T(520, y2 + 30, "Arduino UNO와 다른 점", 16, "bold", "#a40000"))
    o.append(T(520, y2 + 58, "• UNO: 5V 논리, D2·D8·D9 / ESP32: 3.3V 논리, 'IO번호' 그대로 코드에 사용(DHT_PIN = 4)", 14))
    o.append(T(520, y2 + 84, "• 보드 실크인쇄 이름(IO4)과 J3-13 같은 헤더 번호를 함께 확인하면 실수하지 않는다", 14))
    o.append(T(520, y2 + 108, "• ESP32 tone()은 LEDC(PWM) 채널을 사용 → 다른 PWM과 채널이 겹치지 않게 주의", 14))
    return "t0_devkitc_week10_pinmap_ko.svg", svg(Wd, max(120 + p["_H"] + 40, y2 + 160), o)


# ================================================================ t1 DHT11 구조
def anatomy():
    Wd, o = 1400, []
    title(o, "DHT11 — 내부 구조와 두 가지 형태(4핀 단품 / 3핀 모듈)",
          "습도 → 정전용량 변화, 온도 → NTC 저항 변화. 내부 8비트 MCU가 측정·보정 후 40비트 디지털 값으로 보낸다")
    card(o, 40, 110, 520, 430, "① 센서 안쪽에서 일어나는 일", BLU)
    # 블록
    def blk(x, y, w, h, t1, t2, col):
        o.append(rect(x, y, w, h, "#f4f8fd", col, 2, 8)); o.append(T(x + w / 2, y + 26, t1, 15, "bold", col, "middle"))
        o.append(T(x + w / 2, y + 48, t2, 12, fill="#444", anchor="middle"))
    blk(70, 170, 200, 70, "습도 감지 소자", "고분자 흡습 → 용량 C 변화", BLU)
    blk(70, 270, 200, 70, "NTC 서미스터", "온도↑ → 저항 R↓", ORG)
    blk(330, 200, 200, 110, "8비트 MCU", "ADC 변환 · 보정계수(OTP)", PUR)
    o.append(T(430, 290, "· 40비트 프레임 생성", 12, fill="#444", anchor="middle"))
    o += arrow(270, 205, 328, 235, BLU); o += arrow(270, 305, 328, 280, ORG)
    o += arrow(430, 312, 430, 380, CD, 4)
    o.append(rect(340, 382, 180, 46, "#fff4d6", CD, 2, 8)); o.append(T(430, 411, "DATA 핀 1개", 15, "bold", "#8a6200", "middle"))
    o.append(T(60, 465, "• 측정 시간이 걸리고 자체 가열을 피하려고 1초에 한 번 이하로 읽는다", 13))
    o.append(T(60, 490, "  → 수업 코드는 2초 간격", 13))
    o.append(T(60, 518, "• 통신 중이 아닐 때 DATA는 풀업 저항 때문에 HIGH(대기)", 13))
    card(o, 580, 110, 370, 430, "② 4핀 단품", "#1f6aa8")
    bp = dht11_bare(o, 710, 170)
    for lab, s in (("VCC", "3.3~5.5V"), ("DATA", "신호"), ("NC", "연결 안 함"), ("GND", "0V")):
        pass
    o.append(T(600, 410, "1 VCC · 2 DATA · 3 NC · 4 GND", 15, "bold", "#1f6aa8"))
    o.append(T(600, 440, "격자 면을 앞으로, 다리를 아래로 두고", 13))
    o.append(T(600, 462, "왼쪽부터 1번", 13))
    o.append(T(600, 494, "풀업 저항 없음 → DATA–VCC 사이", 13, "bold", "#c62828"))
    o.append(T(600, 516, "4.7k~10kΩ 직접 연결", 13, "bold", "#c62828"))
    card(o, 970, 110, 390, 430, "③ 3핀 모듈(PCB)", "#1f6aa8")
    dht11_module(o, 1100, 165)
    o.append(T(990, 410, "풀업 저항·(전원 LED)이 기판에 있음", 13))
    o.append(T(990, 438, "핀 표기 예: S·+·− / +·OUT·− / VCC·DAT·GND", 13, "bold"))
    o.append(T(990, 466, "→ 제조사마다 순서가 다르다!", 13, "bold", "#c62828"))
    o.append(T(990, 494, "사진 모양이 아니라 기판의 글자를", 13))
    o.append(T(990, 516, "보고 연결한다", 13))
    rows = [("측정 범위(온도)", "0 ~ 50 °C", "−40 ~ 80 °C"),
            ("정확도(온도)", "±2 °C", "±0.5 °C"),
            ("측정 범위(습도)", "20 ~ 90 %RH", "0 ~ 100 %RH"),
            ("정확도(습도)", "±5 %RH", "±2 %RH (최대 ±5)"),
            ("분해능", "1 °C · 1 %RH (정수)", "0.1 °C · 0.1 %RH"),
            ("최소 읽기 간격", "1 초", "2 초"),
            ("시작 신호(LOW)", "≥ 18 ms", "≈ 1 ms (1~10 ms)"),
            ("데이터 형식", "정수 8비트 + 소수 8비트", "16비트 값 × 0.1, 온도 MSB = 부호"),
            ("전원 / 통신", "3.3~5.5 V / 단일선 40비트", "3.3~5.5 V / 단일선 40비트 (같음)")]
    table(o, 40, 600, ["항목", "DHT11 (실물 실습)", "DHT22 = AM2302 (Wokwi 실습)"], rows, [300, 480, 540],
          "④ DHT11 vs DHT22 — Wokwi에는 DHT22만 있어 대체로 사용(프로토콜은 같고 숫자 해석만 다름)", 15)
    return "t1_dht11_anatomy_ko.svg", svg(Wd, 600 + 34 * 10 + 40, o)


# ================================================================ t2 타이밍
def timing():
    Wd, o = 1400, []
    title(o, "DHT11 통신 타이밍 — 단일 DATA선으로 '시작 → 응답 → 40비트 → 종료'",
          "파란 선 = MCU가 LOW로 당김 · 주황 선 = 센서가 당김 · 회색 = 아무도 안 당김(풀업이 HIGH로 유지). 시간 축은 구간별로 확대")
    card(o, 40, 110, 1320, 330, "① 한 번 읽기 전체 흐름 (약 22 ms + 40비트 ≈ 4 ms)", C["ink"])
    y, h = 260, 80
    hi, lo = y - h / 2, y + h / 2
    x = 80
    seg(o, [(x, hi), (x + 60, hi)], "#999")
    seg(o, [(x + 60, hi), (x + 60, lo), (x + 300, lo), (x + 300, hi)], MCU)
    o.append(T(x + 180, lo + 24, "MCU: LOW ≥ 18 ms", 14, "bold", MCU, "middle"))
    o.append(T(x + 180, lo + 44, "'측정해 줘' 요청", 12, fill=MCU, anchor="middle"))
    o.append(line([(x + 170, lo - 10), (x + 180, lo + 8)], "white", 8)); o.append(line([(x + 174, lo - 12), (x + 182, lo + 8)], "#999", 1.5))
    seg(o, [(x + 300, hi), (x + 360, hi)], "#999")
    o.append(T(x + 330, hi - 12, "20~40µs", 12, "bold", "#777", "middle"))
    o.append(T(x + 330, hi - 30, "버스 놓음", 11, fill="#777", anchor="middle"))
    seg(o, [(x + 360, hi), (x + 360, lo), (x + 460, lo), (x + 460, hi), (x + 560, hi), (x + 560, lo)], SEN)
    o.append(T(x + 410, lo + 24, "LOW 80µs", 13, "bold", SEN, "middle"))
    o.append(T(x + 510, hi - 12, "HIGH 80µs", 13, "bold", SEN, "middle"))
    o.append(T(x + 460, lo + 46, "센서 응답 '준비됨'", 12, fill=SEN, anchor="middle"))
    bits = [0, 1, 1, 0]
    xx = x + 560
    for b in bits:
        wh = 34 if b == 0 else 80
        seg(o, [(xx, lo), (xx + 60, lo), (xx + 60, hi), (xx + 60 + wh, hi), (xx + 60 + wh, lo)], SEN)
        o.append(T(xx + 60 + wh / 2, hi - 12, str(b), 15, "bold", SEN, "middle"))
        xx += 60 + wh
    o.append(T(xx + 30, y + 6, "· · ·", 22, "bold", SEN, "middle"))
    xx += 60
    seg(o, [(xx, lo), (xx + 60, lo), (xx + 60, hi)], SEN); seg(o, [(xx + 60, hi), (xx + 140, hi)], "#999")
    o.append(T(xx + 30, lo + 24, "종료 50µs", 12, "bold", SEN, "middle"))
    o.append(T(x + 560 + 200, lo + 44, "40비트(5바이트) 데이터 — 비트마다 LOW 50µs 후 HIGH 길이로 0/1", 12, fill=SEN, anchor="start"))
    o.append(T(60, 410, "코드 대응:  pinMode(OUTPUT) + LOW 20ms → pinMode(INPUT_PULLUP)로 놓기 → 응답 확인 → 40번 반복해 HIGH 길이 측정 (실습 5 / Wokwi B)", 14, "bold", C["ink"]))
    # ② 비트 확대
    card(o, 40, 460, 640, 350, "② 비트 0과 1 — HIGH 길이만 다르다", SEN)
    y2 = 600; hi2, lo2 = y2 - 50, y2 + 50
    x0 = 80
    seg(o, [(x0, hi2), (x0 + 20, hi2), (x0 + 20, lo2), (x0 + 120, lo2), (x0 + 120, hi2), (x0 + 175, hi2), (x0 + 175, lo2), (x0 + 200, lo2)], SEN)
    dim(o, x0 + 20, x0 + 120, lo2 + 26, "50µs (시작)", SEN)
    dim(o, x0 + 120, x0 + 175, hi2 - 14, "26~28µs", SEN)
    o.append(T(x0 + 110, lo2 + 60, "비트 0", 18, "bold", SEN, "middle"))
    x1 = 360
    seg(o, [(x1, hi2), (x1 + 20, hi2), (x1 + 20, lo2), (x1 + 120, lo2), (x1 + 120, hi2), (x1 + 260, hi2), (x1 + 260, lo2), (x1 + 290, lo2)], SEN)
    dim(o, x1 + 20, x1 + 120, lo2 + 26, "50µs (시작)", SEN)
    dim(o, x1 + 120, x1 + 260, hi2 - 14, "≈ 70µs", SEN)
    o.append(T(x1 + 150, lo2 + 60, "비트 1", 18, "bold", SEN, "middle"))
    o.append(line([(x1 + 120 + 48 * 140 / 70, hi2 - 30), (x1 + 120 + 48 * 140 / 70, lo2 + 10)], "#c62828", 2, "5 4"))
    o.append(T(60, 755, "판정: HIGH > 약 48µs 이면 1 (0과 1의 중간값). 비트는 MSB부터 도착한다.", 14, "bold", "#c62828"))
    # ③ 오픈드레인
    card(o, 700, 460, 660, 350, "③ 왜 풀업 저항이 필요한가 — 한 선을 둘이 함께 쓰는 법", "#555")
    bx = 760
    o.append(line([(bx + 270, 520), (bx + 270, 545)], RED, 3)); o.append(T(bx + 280, 528, "3.3V", 13, "bold", RED))
    o.append(f'<rect x="{bx+259}" y="545" width="22" height="60" rx="3" fill="#fff5da" stroke="#8a5818" stroke-width="2"/>')
    o.append(T(bx + 290, 582, "10kΩ", 13, "bold", "#8a5818"))
    o.append(line([(bx + 270, 605), (bx + 270, 640)], CD, 3)); junction(o, bx + 270, 640, CD)
    o.append(line([(bx + 40, 640), (bx + 560, 640)], CD, 4)); o.append(T(bx + 300, 632, "DATA", 13, "bold", "#8a6200"))
    for (cx, lab, col) in ((bx + 110, "MCU(IO4)", MCU), (bx + 470, "DHT11", SEN)):
        o.append(line([(cx, 640), (cx, 670)], col, 3))
        o.append(rect(cx - 40, 670, 80, 46, "#f4f8fd", col, 2, 6)); o.append(T(cx, 699, "스위치", 13, "bold", col, "middle"))
        o.append(line([(cx, 716), (cx, 736)], BLK, 3)); o += sym_gnd(cx, 736)
        o.append(T(cx, 664 - 30, lab, 14, "bold", col, "middle"))
    o.append(T(720, 772 - 2, "• 누구든 스위치를 닫으면 LOW, 아무도 안 닫으면 풀업이 HIGH → 충돌 없이 번갈아 말하기", 13))
    o.append(T(720, 792 - 6, "• 풀업이 없으면 '떠 있는(floating)' 선 → 응답 없음·NaN·체크섬 오류", 13, "bold", "#c62828"))
    return "t2_dht11_timing_ko.svg", svg(Wd, 840, o)


# ================================================================ t3 프레임
def frame():
    Wd, o = 1400, []
    title(o, "40비트 프레임 해독과 체크섬 — 예: 습도 53 %, 온도 24 °C",
          "받은 비트를 8개씩 묶어 5바이트로 만든다 → 앞 4바이트를 더한 값의 하위 8비트가 5번째 바이트와 같으면 정상")
    bytes_ = [(0x35, "습도 정수", BLU), (0x00, "습도 소수", "#6c8fc7"), (0x18, "온도 정수", ORG), (0x00, "온도 소수", "#d69a5a"), (0x4D, "체크섬", GRN)]
    x0, y0, bw = 60, 150, 252
    for i, (v, lab, col) in enumerate(bytes_):
        x = x0 + i * bw
        o.append(rect(x, y0, bw - 12, 200, "white", col, 2.5, 10))
        o.append(f'<rect x="{x}" y="{y0}" width="{bw-12}" height="38" rx="10" fill="{col}"/>')
        o.append(T(x + (bw - 12) / 2, y0 + 26, f"Byte {i} · {lab}", 15, "bold", "white", "middle"))
        bits = f"{v:08b}"
        for k, b in enumerate(bits):
            bx = x + 14 + k * 27
            o.append(rect(bx, y0 + 56, 22, 34, "#eef5ff" if b == "0" else "#ffe9c7", "#aab", 1, 4))
            o.append(mono(bx + 11, y0 + 80, b, 17, "#222", "middle", "bold"))
        o.append(T(x + (bw - 12) / 2, y0 + 112, "비트 " + f"{i*8}~{i*8+7}", 12, fill="#777", anchor="middle"))
        o.append(mono(x + (bw - 12) / 2, y0 + 146, f"0x{v:02X} = {v}", 20, col, "middle", "bold"))
        meaning = ["→ 53 %RH", "→ .0", "→ 24 °C", "→ .0", "→ 검사값 77"][i]
        o.append(T(x + (bw - 12) / 2, y0 + 180, meaning, 15, "bold", "#333", "middle"))
    card(o, 40, 390, 660, 300, "① 체크섬 계산", GRN)
    o.append(mono(70, 450, "  0x35 + 0x00 + 0x18 + 0x00", 18, w="bold"))
    o.append(mono(70, 480, "=  53  +  0  +  24  +  0  = 77", 18))
    o.append(mono(70, 510, "=  0x4D  (하위 8비트만 사용: & 0xFF)", 18))
    o.append(mono(70, 548, "받은 Byte 4 = 0x4D  →  일치 → OK", 18, GRN, w="bold"))
    o.append(T(70, 590, "비트 하나라도 잘못 읽으면? 예) Byte 2의 마지막 비트 0→1", 14, "bold", "#c62828"))
    o.append(mono(70, 618, "0x35+0x00+0x19+0x00 = 0x4E ≠ 0x4D → FAIL", 17, "#c62828", w="bold"))
    o.append(T(70, 660, "→ 라이브러리는 이 값을 버리고 NaN을 돌려준다(isnan() 검사 이유)", 14))
    card(o, 720, 390, 640, 300, "② 같은 방식, DHT22(Wokwi)는 해석만 다르다", PUR)
    o.append(mono(750, 450, "수신: 0x02 0x8C | 0x01 0x5F | 0xEE", 18, w="bold"))
    o.append(mono(750, 488, "습도 = 0x028C = 652 → ×0.1 = 65.2 %", 17))
    o.append(mono(750, 518, "온도 = 0x015F = 351 → ×0.1 = 35.1 °C", 17))
    o.append(mono(750, 548, "검사 = 02+8C+01+5F = 0xEE → OK", 17, GRN, w="bold"))
    o.append(T(750, 590, "온도 바이트 최상위 비트(0x80)가 1이면 영하(음수)", 14, "bold", PUR))
    o.append(T(750, 620, "DHT11 해석 코드를 DHT22 데이터에 쓰면", 14))
    o.append(T(750, 644, "엉뚱한 값이 나온다 → 센서 종류 설정이 중요", 14))
    o.append(T(60, 730, "코드: data[i/8] <<= 1;  if (high_us > 48) data[i/8] |= 1;   →   sum = (data[0]+data[1]+data[2]+data[3]) & 0xFF;", 15, "bold", C["ink"]))
    return "t3_dht11_frame_ko.svg", svg(Wd, 760, o)


# ================================================================ t4 비교
def compare():
    Wd, o = 1400, []
    title(o, "센서 인터페이스 비교 — DHT 단일선 · Dallas 1-Wire · I²C",
          "세 방식 모두 '풀업 + 오픈드레인' 위에서 동작한다. 차이는 비트를 구분하는 방법과 장치를 구별(주소)하는 방법")
    cols = [("DHT 단일선 (DHT11/22)", CD, "실물 실습 1~5"), ("Dallas 1-Wire (DS18B20)", CW1, "Wokwi C"), ("I²C (LCD1602·BH1750 등)", CI2, "Wokwi C")]
    for i, (nm, col, tag) in enumerate(cols):
        x = 40 + i * 445
        o.append(rect(x, 110, 425, 330, "white", col, 2.5, 12))
        o.append(f'<rect x="{x}" y="110" width="425" height="44" rx="12" fill="{col}"/>')
        o.append(T(x + 20, 140, nm, 17, "bold", "white")); o.append(T(x + 405, 140, tag, 12, "bold", "white", "end"))
        # mini diagram
        o.append(rect(x + 25, 190, 90, 120, C["board"], C["boardS"], 2, 8)); o.append(T(x + 70, 215, "ESP32", 13, "bold", "white", "middle"))
        o.append(line([(x + 140, 175), (x + 140, 195)], RED, 2.5)); o.append(T(x + 150, 182, "3V3", 11, "bold", RED))
        o.append(f'<rect x="{x+132}" y="195" width="16" height="40" rx="2" fill="#fff5da" stroke="#8a5818" stroke-width="1.5"/>')
        if i < 2:
            o.append(line([(x + 115, 250), (x + 400, 250)], col, 4)); o.append(line([(x + 140, 235), (x + 140, 250)], col, 2.5)); junction(o, x + 140, 250, col)
            o.append(T(x + 120, 243, "IO4" if i == 0 else "IO13", 11, "bold", col))
            n = 1 if i == 0 else 3
            for k in range(n):
                cx = x + 230 + k * 65
                o.append(line([(cx, 250), (cx, 275)], col, 2.5))
                if i == 0:
                    o.append(rect(cx - 30, 275, 60, 50, "#3d9be0", "#1f6aa8", 2, 5)); o.append(T(cx, 305, "DHT11", 11, "bold", "white", "middle"))
                else:
                    o.append(f'<path d="M{cx-22} 320 L{cx-22} 297 A22 22 0 0 1 {cx+22} 297 L{cx+22} 320 Z" fill="#222"/>')
                    o.append(T(cx, 340, ["…A1", "…B2", "…C3"][k], 11, fill="#555", anchor="middle"))
            if i == 0: o.append(T(x + 300, 300, "센서 1개만", 13, "bold", "#c62828"))
        else:
            o.append(f'<rect x="{x+162}" y="195" width="16" height="40" rx="2" fill="#fff5da" stroke="#8a5818" stroke-width="1.5"/>')
            o.append(line([(x + 115, 245), (x + 400, 245)], col, 4)); o.append(T(x + 120, 240, "SDA 21", 11, "bold", col))
            o.append(line([(x + 115, 275), (x + 400, 275)], BLU, 4)); o.append(T(x + 120, 270, "SCL 22", 11, "bold", BLU))
            o.append(line([(x + 140, 235), (x + 140, 245)], col, 2)); o.append(line([(x + 170, 235), (x + 170, 275)], BLU, 2))
            for k, ad in enumerate(("0x27", "0x23")):
                cx = x + 260 + k * 85
                o.append(line([(cx - 10, 245), (cx - 10, 290)], col, 2)); o.append(line([(cx + 10, 275), (cx + 10, 290)], BLU, 2))
                o.append(rect(cx - 32, 290, 64, 38, "#2f5d8a", "#173552", 2, 5)); o.append(T(cx, 314, ad, 12, "bold", "white", "middle"))
        facts = [["신호선 1개 + 전원·GND", "주소 없음 → 버스당 1개", "HIGH 길이(26/70µs)로 0·1"],
                 ["신호선 1개 + GND(+VDD)", "64비트 ROM 주소 → 여러 개", "LOW 길이 시간 슬롯 + 리셋"],
                 ["SDA + SCL 2선 + 전원·GND", "7비트 주소 → 여러 개", "SCL 클록에 맞춰 SDA 샘플"]][i]
        for k, s in enumerate(facts):
            o.append(T(x + 22, 370 + k * 24, "• " + s, 14, "bold" if k == 1 else "normal"))
    rows = [("신호선", "DATA 1", "DQ 1", "SDA·SCL 2"),
            ("비트 구분", "HIGH 펄스 길이", "LOW 펄스 길이(시간 슬롯)", "클록 엣지(동기식)"),
            ("시작 / 응답", "MCU LOW 18ms → 센서 80/80µs", "RESET 480µs → PRESENCE", "START → 주소 → ACK"),
            ("장치 구별", "없음(1:1)", "ROM 64비트(공장 고유)", "7비트 주소(부품 고정·점퍼)"),
            ("오류 검사", "8비트 합 체크섬", "CRC-8", "바이트마다 ACK/NACK"),
            ("속도(대략)", "1회 측정/1~2초", "~15 kbps", "100k / 400 kbps"),
            ("주도권", "MCU 요청 후 센서가 일방 전송", "마스터가 모든 슬롯 시작", "마스터가 클록 생성"),
            ("ESP32 라이브러리", "DHT sensor library / DHTesp", "OneWire + DallasTemperature", "Wire (I²C 내장)")]
    table(o, 40, 490, ["구분", "DHT 단일선", "Dallas 1-Wire", "I²C"], rows, [200, 360, 380, 380],
          "주의: DHT11은 '1-Wire처럼 선 하나'지만 Dallas 1-Wire 규격이 아니다 → OneWire 라이브러리로 읽을 수 없다", 14)
    return "t4_interface_compare_ko.svg", svg(Wd, 490 + 34 * 9 + 30, o)


# ================================================================ t5 히스테리시스
def hysteresis():
    Wd, o = 1400, []
    title(o, "임계값과 히스테리시스 — 경계에서 부저가 '삑삑삑' 떨리는 문제 막기",
          "ON 기준(30 °C)과 OFF 기준(28 °C)을 다르게 둔다. 그 사이(데드밴드)에서는 이전 상태를 유지")
    gx, gy, gw, gh = 120, 130, 1180, 320
    o.append(rect(gx, gy, gw, gh, "#fbfcfd", "#c9ced6", 1.5, 6))
    def ty(t): return gy + gh - (t - 25) / 8 * gh
    for t in range(25, 34, 1):
        o.append(line([(gx, ty(t)), (gx + gw, ty(t))], "#eef0f3", 1)); o.append(T(gx - 10, ty(t) + 5, f"{t}°C", 12, fill="#777", anchor="end"))
    o.append(line([(gx, ty(30)), (gx + gw, ty(30))], "#c62828", 2.5, "8 5")); o.append(T(gx + gw + 8, ty(30) + 5, "ON 30 °C", 14, "bold", "#c62828"))
    o.append(line([(gx, ty(28)), (gx + gw, ty(28))], BLU, 2.5, "8 5")); o.append(T(gx + gw + 8, ty(28) + 5, "OFF 28 °C", 14, "bold", BLU))
    o.append(f'<rect x="{gx}" y="{ty(30)}" width="{gw}" height="{ty(28)-ty(30)}" fill="#ffd54f" fill-opacity="0.15"/>')
    o.append(T(gx + 10, ty(29) + 5, "데드밴드(유지 구간)", 13, "bold", "#9a7400"))
    import math
    temps = [28.4 + 2.5 * math.sin(i / 14) + 0.55 * math.sin(i * 1.7) + (0.4 if 30 < i < 50 else 0) for i in range(0, 118)]
    pts = [(gx + i * gw / 117, ty(t)) for i, t in enumerate(temps)]
    o.append(line(pts, "#333", 2.5))
    o.append(T(gx, gy - 10, "측정 온도(2초마다)", 13, "bold", "#333"))
    # 상태 막대
    def strip(y, label, states, col):
        o.append(T(gx - 10, y + 20, label, 14, "bold", C["ink"], "end"))
        o.append(rect(gx, y, gw, 30, "#f1f3f5", "#c9ced6", 1, 4))
        for i, s in enumerate(states):
            if s: o.append(f'<rect x="{gx + i*gw/117:.1f}" y="{y}" width="{gw/117+0.6:.1f}" height="30" fill="{col}"/>')
    single = [t >= 30 for t in temps]
    hyst, st = [], False
    for t in temps:
        if not st and t >= 30: st = True
        elif st and t <= 28: st = False
        hyst.append(st)
    strip(490, "기준 1개(30 °C)", single, "#e57373")
    strip(540, "히스테리시스", hyst, GRN)
    flips1 = sum(1 for a, b in zip(single, single[1:]) if a != b); flips2 = sum(1 for a, b in zip(hyst, hyst[1:]) if a != b)
    o.append(T(gx + gw + 10, 510, f"전환 {flips1}회", 13, "bold", "#c62828"))
    o.append(T(gx + gw + 10, 560, f"전환 {flips2}회", 13, "bold", GRN))
    o.append(T(60, 620, "켜기: 온도 ≥ 30 °C  또는(OR)  습도 ≥ 75 %   — 위험 조건 하나만 맞아도 경보", 15, "bold", "#c62828"))
    o.append(T(60, 650, "끄기: 온도 ≤ 28 °C  그리고(AND)  습도 ≤ 70 %   — 두 조건이 모두 안전해야 해제", 15, "bold", BLU))
    o.append(T(60, 680, "DHT11은 1 °C 단위라 29↔30 °C를 자주 오간다 → 기준 1개만 쓰면 경보가 2초마다 켜졌다 꺼질 수 있다", 14))
    return "t5_hysteresis_ko.svg", svg(Wd + 40, 710, o)


# ================================================================ t6 부저
def buzzer_fig():
    Wd, o = 1400, []
    title(o, "부저의 종류와 ESP32 구동 — 능동형 vs 수동형, tone()의 정체",
          "이번 실습은 '수동형(passive) 피에조'를 tone()으로 2 kHz 사각파 구동. 큰 전류 부저는 트랜지스터로")
    card(o, 40, 110, 640, 300, "① 능동형 vs 수동형", PUR)
    rows = [("구분", "능동형(Active)", "수동형(Passive)")]
    o.append(T(70, 170, "능동형: 내부 발진회로 有 → 전원만 주면 '삐-' 한 음", 15))
    o.append(T(70, 200, "  digitalWrite(HIGH)로 ON/OFF · 음높이 바꿀 수 없음", 14, fill="#555"))
    o.append(T(70, 240, "수동형: 발진회로 無 → 사각파를 넣어야 소리남", 15, "bold", PUR))
    o.append(T(70, 270, "  tone(핀, 주파수)로 음높이 조절 · 멜로디 가능", 14, fill="#555"))
    o.append(T(70, 315, "구별법: 3V를 잠깐 연결 → 소리 나면 능동형", 14, "bold"))
    o.append(T(70, 343, "(밑면이 막혀 있고 스티커가 있으면 능동형인 경우가 많음)", 13, fill="#555"))
    o.append(T(70, 380, "능동형이면 코드의 tone() 대신 digitalWrite()로 바꿔 사용", 13, fill="#555"))
    card(o, 700, 110, 660, 300, "② tone(25, 2000) 이 만드는 신호", PUR)
    y, h = 260, 70
    o += digital(740, y, [1, 0] * 6, 46, h, PUR)
    dim(o, 740, 832, y + h / 2 + 26, "주기 T = 1/2000 Hz = 500 µs", PUR, 13)
    o.append(T(740, y - h / 2 - 14, "3.3V", 12, fill="#777")); o.append(T(740, y + h / 2 + 60, "듀티 50 % 사각파 · ESP32 LEDC(PWM) 하드웨어가 생성 → CPU는 계속 다른 일", 13))
    o.append(T(740, y + h / 2 + 84, "noTone(25) → 정지.  millis()로 120ms 켜고 480ms 끄면 '삑…삑…'", 13))
    card(o, 40, 430, 640, 330, "③ GPIO 직결 (이번 실습)", GRN)
    o.append(rect(80, 520, 120, 120, C["board"], C["boardS"], 2, 8)); o.append(T(140, 550, "ESP32", 14, "bold", "white", "middle"))
    o.append(T(190, 585, "IO25", 13, "bold", "white", "end")); o.append(T(190, 625, "GND", 13, "bold", "white", "end"))
    o.append(line([(200, 580), (420, 580)], PUR, 4)); o += sym_buzzer(446, 600, 30)
    o.append(line([(200, 620), (380, 620), (380, 650), (480, 650), (480, 620)], BLK, 3))
    o.append(T(80, 690, "조건: 3.3V에서 동작하는 소형 피에조(수 mA)", 14, "bold", GRN))
    o.append(T(80, 716, "ESP32 GPIO는 핀당 수십 mA 이하 권장 → 저전류만 직결", 14))
    o.append(T(80, 742, "모듈형(트랜지스터 내장) 부저는 VCC·GND·I/O 3핀으로 연결", 14))
    card(o, 700, 430, 660, 330, "④ 큰 전류 부저(전자기식·5V) → NPN", ORG)
    bx = 760
    o.append(line([(bx, 600), (bx + 60, 600)], PUR, 3)); o.append(T(bx, 590, "IO25", 13, "bold", PUR))
    o += sym_resistor_h(bx + 60, bx + 150, 600, "1kΩ")
    o.append(line([(bx + 150, 600), (bx + 200, 600)], "#333", 3))
    o.append(circle(bx + 225, 600, 30, "none", "#333", 2.5))
    o.append(line([(bx + 205, 580), (bx + 205, 620)], "#333", 4))
    o.append(line([(bx + 205, 590), (bx + 240, 570), (bx + 240, 540)], "#333", 3))
    o.append(line([(bx + 205, 610), (bx + 240, 630), (bx + 240, 680)], "#333", 3))
    o += arrow(bx + 222, 620, bx + 238, 629, "#333", 2)
    o.append(T(bx + 255, 610, "NPN (2N2222·S8050)", 13, "bold"))
    o += sym_gnd(bx + 240, 680)
    o.append(rect(bx + 215, 490, 50, 50, "#2b2b2b", "#000", 2, 25)); o.append(T(bx + 240, 520, "BZ", 12, "bold", "white", "middle"))
    o.append(line([(bx + 240, 476), (bx + 240, 490)], RED, 3)); o.append(T(bx + 230, 488, "5V", 13, "bold", RED, "end"))
    o.append(T(bx + 300, 510, "다이오드(1N4148)를 부저와 역병렬로", 13))
    o.append(T(bx + 300, 532, "→ 코일 역기전력으로부터 보호", 13))
    o.append(T(720, 742, "GPIO는 베이스 전류(수 mA)만 공급, 부저 전류는 5V에서 직접", 14, "bold", ORG))
    return "t6_buzzer_ko.svg", svg(Wd, 780, o)


# ================================================================ t7 상태기계
def state_machine():
    Wd, o = 1400, []
    title(o, "경보 시스템 소프트웨어 구조 — millis() 비차단 루프 + 3상태 상태기계(실습 4)",
          "loop()는 수 µs마다 돈다. 2초가 지났을 때만 측정하고, 부저는 매 loop마다 시간만 보고 켜고 끈다(delay 없음)")
    card(o, 40, 110, 500, 560, "① loop() 흐름", C["ink"])
    steps = [("now = millis()", "#eef2f7"), ("now − lastRead ≥ 2000 ?", "#fff4d6"), ("읽기: t, h", "#eef2f7"),
             ("유효? (NaN·범위)", "#fff4d6"), ("이동평균 3개 → 상태 판단", "#eef2f7"), ("CSV 한 줄 출력", "#eef2f7"),
             ("updateActuators(now)", "#e8f6ee")]
    for i, (s, f) in enumerate(steps):
        y = 160 + i * 70
        o.append(rect(90, y, 400, 46, f, "#9aa3ad", 1.5, 10)); o.append(T(290, y + 29, s, 15, "bold", C["ink"], "middle"))
        if i < len(steps) - 1: o += arrow(290, y + 46, 290, y + 68, "#555", 2)
    o.append(T(500, 210, "아니오 →", 12, fill="#777", anchor="end"))
    o.append(line([(490, 253), (520, 253), (520, 625), (492, 625)], "#999", 2, "5 4"))
    o.append(T(500, 390, "실패 → errors++", 12, "bold", "#c62828", "end"))
    o.append(T(100, 655, "부저: 120ms ON / 480ms OFF를 시간 비교로만 처리", 13, fill="#555"))
    card(o, 560, 110, 800, 560, "② 상태 전이도", C["ink"])
    st = {"N": (760, 300, "NORMAL", GRN, "부저 OFF · LED OFF"), "W": (1160, 300, "WARNING", "#c62828", "부저 삑삑 · LED ON"),
          "E": (960, 560, "SENSOR_ERROR", "#7a7f88", "부저 OFF · LED 깜빡")}
    for k, (x, y, n, col, s) in st.items():
        o.append(f'<ellipse cx="{x}" cy="{y}" rx="110" ry="54" fill="white" stroke="{col}" stroke-width="3"/>')
        o.append(T(x, y - 4, n, 17, "bold", col, "middle")); o.append(T(x, y + 20, s, 12, fill="#444", anchor="middle"))
    o += arrow(860, 275, 1055, 275, "#c62828", 3); o.append(T(958, 262, "평균 T≥30 또는 H≥75", 13, "bold", "#c62828", "middle"))
    o += arrow(1055, 325, 860, 325, GRN, 3); o.append(T(958, 350, "평균 T≤28 그리고 H≤70", 13, "bold", GRN, "middle"))
    o += arrow(800, 350, 900, 512, "#7a7f88", 3); o += arrow(1120, 350, 1020, 512, "#7a7f88", 3)
    o.append(T(760, 440, "연속 실패 3회", 13, "bold", "#7a7f88", "end")); o.append(T(1160, 440, "연속 실패 3회", 13, "bold", "#7a7f88"))
    o += arrow(930, 507, 780, 356, "#555", 2)
    o.append(T(860, 470, "정상값 수신", 12, fill="#555", anchor="end"))
    o.append(T(580, 170, "• 상태 = '지금 무엇을 해야 하는가'를 한 변수에 담는다", 14))
    o.append(T(580, 196, "• 실패 1~2회는 마지막 상태 유지, 3회 연속이면 오류로 구분", 14))
    o.append(T(580, 222, "• 오류 시 경보음을 끄는 이유: 측정 불가 ≠ 위험 → 별도 표시", 14))
    return "t7_state_machine_ko.svg", svg(Wd, 700, o)


# ================================================================ Wokwi 그림
def wokwi_header(o, ttl, sub):
    title(o, ttl, sub)


def wk_a():
    Wd, o = 1400, []
    wokwi_header(o, "Wokwi 워크북 A · DHT22로 온습도 경보 시뮬레이션 — 부품 배치와 연결",
                 "Wokwi에는 DHT11이 없어 DHT22 사용(같은 단일선 방식). 센서를 클릭해 슬라이더로 온도·습도를 바꿔 경보를 만든다")
    used = {"3V3": (RED, ""), "GND@J3a": (BLK, ""), "IO4": (YEL, ""), "IO25": (PUR, ""), "IO26": (GRN, ""), "GND@J2": (BLK, "")}
    p = devkit(o, 470, 140, used)
    # DHT22 (흰색 하우징)
    x, y = 1050, 340
    o.append(rect(x, y, 110, 150, "#f5f5f5", "#999", 2, 8))
    for r in range(5):
        for c in range(4):
            o.append(f'<rect x="{x+12+c*23}" y="{y+12+r*20}" width="16" height="14" rx="2" fill="#d9d9d9"/>')
    o.append(T(x + 55, y + 136, "DHT22", 14, "bold", "#333", "middle"))
    pins = {}
    for k, lab in enumerate(["VCC", "SDA", "NC", "GND"]):
        px = x + 19 + k * 24
        o.append(line([(px, y + 150), (px, y + 182)], "#a9a9a9", 4)); o.append(circle(px, y + 185, 5, "#d4af37"))
        o.append(T(px, y + 204, lab, 10, "bold", "#333", "middle")); pins[lab] = (px, y + 185)
    o.append(T(x + 55, y - 14, "dht1 (wokwi-dht22)", 13, "bold", "#555", "middle"))
    io4, g3, v33 = p["IO4"], p["GND@J3a"], p["3V3"]
    W(o, [io4, (pins["SDA"][0], io4[1]), pins["SDA"]], YEL)
    W(o, [g3, (1320, g3[1]), (1320, 600), (pins["GND"][0], 600), pins["GND"]], BLK)
    W(o, [v33, (440, v33[1]), (440, 120), (1290, 120), (1290, 575), (pins["VCC"][0], 575), pins["VCC"]], RED)
    inline_res(o, (1000, 560), (1000, 640), "", vertical=True)
    o.append(line([(1000, 530), (1000, 560)], RED, 3)); o.append(line([(1000, 530), (pins["VCC"][0], 530)], RED, 3)); junction(o, 1000, io4[1], YEL)
    o.append(line([(1000, 640), (1000, io4[1])], YEL, 3)); o.append(T(990, 600, "r_pu 10kΩ", 12, "bold", "#8a5818", "end"))
    o.append(T(1100, io4[1] - 8, "SDA(=DATA) → IO4", 13, "bold", "#9a7400", "middle"))
    bz = buzzer_part(o, 200, 400, "bz1 (wokwi-buzzer)")
    io25, gl = p["IO25"], p["GND@J2"]
    W(o, [io25, (bz["+"][0], io25[1]), bz["+"]], PUR); W(o, [gl, (300, gl[1]), (300, 470), (bz["−"][0], 470), bz["−"]], BLK)
    o.append(T(186, 500, "2번(+)", 11, "bold", PUR, "end")); o.append(T(220, 500, "1번(−)", 11, "bold", BLK))
    lp = led_part(o, 120, 610, C["red"], "led1 (red)")
    io26 = p["IO26"]; a = lp["A"]
    o.append(line([io26, (300, io26[1]), (300, 700), (a[0], 700), a], GRN, 4)); inline_res(o, (300, io26[1]), (300, 700), "r1 330Ω", vertical=True)
    W(o, [lp["K"], (lp["K"][0], 730), (340, 730), (340, gl[1]), gl], BLK)
    y0 = 140 + p["_H"] + 40
    rows = [("esp", "board-esp32-devkit-c-v4", "보드", "—"),
            ("dht1", "wokwi-dht22", "VCC→3V3, SDA→esp:4, GND→GND.1", "temperature 24, humidity 45"),
            ("r_pu", "wokwi-resistor 10kΩ", "3V3 ↔ esp:4 (풀업 습관)", "실물과 동일하게"),
            ("bz1", "wokwi-buzzer", "2(+)→esp:25, 1(−)→GND.1", "volume 0.2"),
            ("r1, led1", "wokwi-resistor 330Ω, wokwi-led", "esp:26→r1→A, C→GND.1", "color red")]
    table(o, 40, y0 + 20, ["id", "부품(type)", "연결(diagram.json)", "속성"], rows, [130, 340, 480, 370], None, 14)
    return "wk_a_wokwi_dht22_alarm_ko.svg", svg(Wd, y0 + 20 + 34 * 6 + 30, o)


def wk_b():
    Wd, o = 1400, []
    wokwi_header(o, "Wokwi 워크북 B · 로직 분석기로 DHT 타이밍 직접 보기",
                 "D0 = DATA(IO4), D1 = 판정 표시(IO13). 시뮬레이션 정지 → .vcd 파일 다운로드 → PulseView에서 펄스 폭 측정")
    # 연결 블록
    o.append(rect(60, 120, 200, 200, C["board"], C["boardS"], 3, 12)); o.append(T(160, 150, "ESP32", 17, "bold", "white", "middle"))
    for lab, yy in (("3V3", 180), ("IO4", 220), ("IO13", 260), ("GND", 300)):
        o.append(T(245, yy + 5, lab, 14, "bold", "white", "end"))
    o.append(rect(560, 120, 170, 120, "#f5f5f5", "#999", 2, 8)); o.append(T(645, 170, "DHT22", 16, "bold", "#333", "middle"))
    o.append(T(645, 194, "VCC SDA NC GND", 11, fill="#555", anchor="middle"))
    o.append(rect(900, 120, 300, 200, "#1c1c1c", "#000", 2, 8)); o.append(T(1050, 152, "Logic Analyzer (la1)", 15, "bold", "white", "middle"))
    for k in range(8):
        o.append(circle(930 + k * 34, 190, 7, "#d4af37")); o.append(T(930 + k * 34, 215, f"D{k}", 11, "bold", "white", "middle"))
    o.append(circle(1170, 260, 7, "#d4af37")); o.append(T(1170, 285, "GND", 11, "bold", "white", "middle"))
    o.append(line([(260, 180), (520, 180), (520, 150), (560, 150)], RED, 3.5))
    o.append(line([(260, 220), (600, 220), (600, 240)], YEL, 3.5)); o.append(line([(600, 220), (860, 220), (860, 168), (930, 168), (930, 183)], YEL, 3.5))
    o.append(line([(260, 260), (964, 260), (964, 197)], "#00a3c4", 3.5))
    o.append(line([(260, 300), (1170, 300), (1170, 267)], BLK, 3.5)); o.append(line([(700, 300), (700, 240)], BLK, 3.5))
    o.append(T(420, 214, "DATA", 13, "bold", "#9a7400")); o.append(T(420, 254, "MARK(비트 판정 순간 토글)", 13, "bold", "#008aa6"))
    # 기대 파형
    card(o, 40, 350, 1320, 360, "PulseView에서 보이는 모습(예상) — 커서 두 개로 폭을 재고 표에 기록", C["ink"])
    y, h = 470, 60; hi, lo = y - h / 2, y + h / 2
    o.append(T(80, y + 5, "D0", 15, "bold", "#9a7400"))
    x = 120
    seg(o, [(x, hi), (x + 30, hi), (x + 30, lo), (x + 200, lo), (x + 200, hi), (x + 225, hi), (x + 225, lo), (x + 285, lo), (x + 285, hi), (x + 345, hi)], "#d9a400")
    xx = x + 345
    marks = []
    for b in [0, 0, 1, 1, 0, 1, 0, 1]:
        wh = 25 if b == 0 else 62
        seg(o, [(xx, hi), (xx, lo), (xx + 45, lo), (xx + 45, hi), (xx + 45 + wh, hi), (xx + 45 + wh, lo)], "#d9a400")
        o.append(T(xx + 45 + wh / 2, hi - 8, str(b), 13, "bold", "#9a7400", "middle"))
        marks.append(xx + 45 + wh)
        xx += 45 + wh
    o.append(T(xx + 20, y + 6, "…", 20, "bold", "#9a7400"))
    o.append(T(x + 115, lo + 22, "MCU LOW(1.2ms, DHT22)", 12, "bold", MCU, "middle"))
    o.append(T(x + 255, lo + 22, "응답 80/80", 12, "bold", SEN, "middle"))
    y1 = 580; hi1, lo1 = y1 - 22, y1 + 22
    o.append(T(80, y1 + 5, "D1", 15, "bold", "#008aa6"))
    pts, lvl = [(120, lo1)], 0
    for m in marks:
        yy = lo1 if lvl == 0 else hi1
        pts += [(m, yy)]; lvl ^= 1; pts += [(m, lo1 if lvl == 0 else hi1)]
    pts.append((xx + 10, pts[-1][1]))
    seg(o, pts, "#00a3c4", 2.5)
    o.append(T(120, 640, "D1이 바뀌는 순간 = 코드가 HIGH 길이를 다 잰 시점 → 비트 판정이 파형과 맞는지 눈으로 확인", 14))
    o.append(T(120, 668, "※ 시뮬레이터 CPU는 실제보다 느릴 수 있어 시리얼에 찍힌 µs 값과 파형 폭이 수 µs 다를 수 있다 — 파형 값이 기준", 13, fill="#c62828"))
    rows = [("응답 LOW", "80µs", "", ""), ("응답 HIGH", "80µs", "", ""), ("비트 0의 HIGH", "26~28µs", "", ""), ("비트 1의 HIGH", "70µs", "", ""), ("비트 앞 LOW", "50µs", "", "")]
    table(o, 40, 760, ["구간", "데이터시트", "PulseView 측정값", "시리얼 출력값"], rows, [260, 220, 320, 320], "기록표", 14)
    return "wk_b_wokwi_logic_analyzer_ko.svg", svg(Wd, 760 + 34 * 6 + 30, o)


def wk_c():
    Wd, o = 1400, []
    wokwi_header(o, "Wokwi 워크북 C · 진짜 1-Wire(DS18B20 ×2)와 I²C(LCD1602) 한 회로에서 비교",
                 "1-Wire: IO13 한 선에 센서 2개(ROM 주소로 구별, 4.7kΩ 풀업) · I²C: IO21/IO22에 LCD(주소 0x27) · DHT22: IO4")
    used = {"3V3": (RED, ""), "GND@J3b": (BLK, ""), "IO4": (YEL, ""), "IO13": (CW1, ""), "IO21": (CI2, ""), "IO22": (BLU, ""),
            "GND@J2": (BLK, ""), "5V": ("#a33", "")}
    p = devkit(o, 520, 140, used)
    # 1-Wire 왼쪽
    io13, gl, v33 = p["IO13"], p["GND@J2"], p["3V3"]
    busy = 650
    o.append(line([io13, (470, io13[1]), (470, busy), (60, busy)], CW1, 4)); o.append(dot(*io13, CW1, 4.5))
    vrail = 330
    o.append(line([v33, (60, v33[1])], RED, 4)); o.append(dot(*v33, RED, 4.5))
    grail = 760
    o.append(line([gl, (490, gl[1]), (490, grail), (60, grail)], BLK, 4)); o.append(dot(*gl, BLK, 4.5))
    o.append(T(70, v33[1] - 10, "3V3 (J2-1)", 13, "bold", RED)); o.append(T(70, busy - 10, "1-Wire 버스 DQ (IO13, J2-15)", 13, "bold", CW1))
    o.append(T(70, grail + 22, "GND (J2-14)", 13, "bold", BLK))
    # 풀업
    o.append(line([(420, v33[1]), (420, 430)], RED, 3)); inline_res(o, (420, 430), (420, 520), "", vertical=True)
    o.append(line([(420, 520), (420, busy)], CW1, 3)); junction(o, 420, v33[1], RED); junction(o, 420, busy, CW1)
    o.append(T(412, 480, "4.7kΩ", 13, "bold", "#8a5818", "end"))
    for k, (cx, lab, tmp) in enumerate(((150, "ds1 …4E01", "22.5"), (300, "ds2 …4E02", "28.0"))):
        d = ds18b20(o, cx, 430, lab)
        o.append(line([d["DQ"], (d["DQ"][0], busy)], CW1, 3)); junction(o, d["DQ"][0], busy, CW1)
        o.append(line([d["VDD"], (d["VDD"][0] + 0, 560), (d["VDD"][0] + 34, 560), (d["VDD"][0] + 34, v33[1])], RED, 2.5)); junction(o, d["VDD"][0] + 34, v33[1], RED)
        o.append(line([d["GND"], (d["GND"][0], 600), (d["GND"][0] - 34, 600), (d["GND"][0] - 34, grail)], BLK, 2.5)); junction(o, d["GND"][0] - 34, grail, BLK)
        o.append(T(cx, 420, f"{tmp} °C", 13, "bold", CW1, "middle"))
    # I2C 오른쪽
    lp = lcd1602(o, 1030, 520)
    io21, io22, g3, v5 = p["IO21"], p["IO22"], p["GND@J3b"], p["5V"]
    W(o, [io22, (lp["SCL"][0], io22[1]), lp["SCL"]], BLU)
    W(o, [io21, (lp["SDA"][0], io21[1]), lp["SDA"]], CI2)
    W(o, [g3, (lp["GND"][0], g3[1]), lp["GND"]], BLK)
    W(o, [v5, (500, v5[1]), (500, 1000), (1010, 1000), (1010, 506), (lp["VCC"][0], 506), lp["VCC"]], "#a33")
    o.append(T(1310, io22[1] - 8, "SCL → IO22 (J3-3)", 13, "bold", BLU, "end")); o.append(T(1220, io21[1] - 8, "SDA → IO21 (J3-6)", 13, "bold", CI2, "end"))
    o.append(T(1080, g3[1] - 8, "GND (J3-7)", 13, "bold", BLK, "end"))
    o.append(T(1020, 990, "LCD VCC → 5V(J2-19)", 12, "bold", "#a33"))
    # DHT22 아래
    x, y = 890, 710
    o.append(rect(x, y, 90, 110, "#f5f5f5", "#999", 2, 8)); o.append(T(x + 45, y + 60, "DHT22", 13, "bold", "#333", "middle"))
    io4 = p["IO4"]
    W(o, [io4, (x + 30, io4[1]), (x + 30, y)], YEL)
    o.append(T(x + 45, y + 132, "SDA→IO4 · VCC/GND 생략", 11, fill="#555", anchor="middle"))
    H = 140 + p["_H"] + 70
    o.append(rect(40, H, 1320, 170, "#f3f4f6", "#9aa3ad", 2, 10))
    o.append(T(60, H + 32, "시리얼에서 확인할 것 (ROM 값은 예시 — deviceID 속성에 따라 달라짐)", 16, "bold"))
    o.append(mono(60, H + 64, "[I2C] 장치 발견: 0x27", 15, CI2, w="bold"))
    o.append(mono(60, H + 92, "[1-Wire] ROM = 28:01:4E:3D:2C:1B:0A:xx  family=0x28(DS18B20) CRC OK  (×2)", 15, CW1, w="bold"))
    o.append(mono(60, H + 120, "DS1=22.50C  DS2=28.00C  |  DHT22 T=24.0C RH=45.0%", 15, "#333", w="bold"))
    o.append(T(60, H + 150, "※ 실물에서 5V LCD 백팩을 ESP32에 쓰면 SDA/SCL이 5V로 풀업됨 → 3.3V LCD를 쓰거나 레벨 시프터 사용", 13, fill="#c62828"))
    return "wk_c_wokwi_onewire_i2c_ko.svg", svg(Wd, H + 190, o)


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    for f in (pinmap, anatomy, timing, frame, compare, hysteresis, buzzer_fig, state_machine, wk_a, wk_b, wk_c):
        n, s = f()
        open(os.path.join(OUT, n), "w", encoding="utf-8").write(s)
        print(n)
