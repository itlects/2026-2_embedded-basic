# -*- coding: utf-8 -*-
"""9주차 Part B 시리얼 통신 그림: 공통 원리, 4방식 비교, UART/I2C/SPI/1-Wire 파형, 예제 연결도.
사용: python3 gen_serial.py OUTDIR"""
import sys, os
from figlib import *

OUT = sys.argv[1] if len(sys.argv) > 1 else "."
MONO = "Consolas,'DejaVu Sans Mono',monospace"
CU, CI, CS, CW = "#2664bf", "#208657", "#c06000", "#7b4bb3"   # UART, I2C, SPI, 1-Wire 색


def mono(x, y, s, size=14, fill="#1d2433", anchor="start", w="normal"):
    return (f'<text x="{x}" y="{y}" font-size="{size}" font-weight="{w}" font-family="{MONO}" '
            f'fill="{fill}" text-anchor="{anchor}" xml:space="preserve">{esc(s)}</text>')


def digital(x0, y, bits, w, h, color, lw=3, labels=None, lab_size=13):
    """bits: list of 0/1 per slot of width w. y = center line; HIGH = y-h/2."""
    pts = []
    for i, b in enumerate(bits):
        yy = y - h / 2 if b else y + h / 2
        xa = x0 + i * w
        if pts and pts[-1][1] != yy: pts.append((xa, yy))
        elif not pts: pts.append((xa, yy))
        pts.append((xa + w, yy))
    o = [line(pts, color, lw, cap="butt")]
    if labels:
        for i, s in enumerate(labels):
            if s: o.append(T(x0 + i * w + w / 2, y - h / 2 - 8, s, lab_size, "bold", color, "middle"))
    return o


def signal_label(o, x, y, s, color):
    o.append(T(x, y + 5, s, 15, "bold", color, "end"))


# ================================================================ s1 공통 원리
def common():
    W, H = 1240, 860
    o = []
    title(o, "시리얼 통신의 공통 원리 — 한 줄로 1비트씩, 약속(프로토콜)대로",
          "MCU와 센서·PC는 데이터를 비트 단위로 순서대로 보낸다. 방식은 달라도 아래 5가지 질문에 대한 '약속'이 프로토콜이다.")
    # 병렬 vs 직렬
    card(o, 40, 110, 560, 300, "① 병렬 vs 직렬", C["ink"])
    o.append(T(70, 175, "병렬: 8비트를 8개 선으로 동시에", 15, "bold"))
    for i in range(8):
        y = 195 + i * 11
        o.append(line([(80, y), (300, y)], "#888", 3))
        o.append(T(310, y + 5, str((0x41 >> (7 - i)) & 1), 12, "bold", "#555"))
    o.append(T(350, 240, "→ 빠르지만 선이 많다", 14, fill="#555"))
    o.append(T(70, 315, "직렬: 1개 선으로 하나씩 차례로", 15, "bold"))
    o += digital(80, 360, [1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1], 24, 30, CU, 3)
    o.append(T(350, 395, "→ 선이 적어 센서 연결에 유리", 14, fill="#555"))
    # 5가지 약속
    card(o, 620, 110, 580, 300, "② 모든 시리얼 방식이 정하는 5가지", C["ink"])
    q = [("1. 타이밍", "클럭선을 같이 보내나(동기) / 속도를 미리 맞추나(비동기)"),
         ("2. 프레임", "어디서 시작·끝나나 (START/STOP, CS, RESET)"),
         ("3. 주소", "여러 장치 중 누구에게? (주소·CS선·64비트 ID)"),
         ("4. 방향", "동시에 양방향(전이중) / 번갈아(반이중)"),
         ("5. 전기", "전압 레벨, 풀업 저항, 공통 GND, 거리")]
    for i, (a, b) in enumerate(q):
        o.append(T(645, 170 + i * 46, a, 16, "bold", C["blue"]))
        o.append(T(745, 170 + i * 46, b, 14))
    # 동기 vs 비동기
    card(o, 40, 430, 560, 390, "③ 동기식 vs 비동기식", C["ink"])
    o.append(T(70, 490, "비동기(UART): 클럭선 없음 → 양쪽 baud 일치 필요", 15, "bold", CU))
    o += digital(80, 540, [1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1], 36, 36, CU, 3,
                 ["", "S", "1", "0", "0", "0", "0", "0", "1", "0", "P", ""], 12)
    o.append(T(70, 590, "S=START, P=STOP. 비트 폭 = 1/baud (115200 → 8.68µs)", 13, fill="#555"))
    o.append(T(70, 640, "동기(I2C·SPI): 클럭선이 '지금 읽어라'를 알려줌", 15, "bold", CI))
    o += digital(80, 690, [0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1], 30, 30, "#555", 2.5)
    o.append(T(70, 690 + 5, "CLK", 12, "bold", "#555", "end"))
    o += digital(80, 740, [1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1], 30, 30, CI, 3)
    o.append(T(70, 745, "DATA", 12, "bold", CI, "end"))
    o.append(T(70, 795, "클럭이 있으므로 속도가 조금 흔들려도 된다. 1-Wire는 '시간 슬롯'으로 동기", 13, fill="#555"))
    # 토폴로지
    card(o, 620, 430, 580, 390, "④ 연결 구조(토폴로지)", C["ink"])
    def box(x, y, s, col, w=90):
        o.append(rect(x, y, w, 38, "#fff", col, 2.5, 6)); o.append(T(x + w / 2, y + 25, s, 14, "bold", col, "middle"))
    box(650, 480, "MCU", CU); box(850, 480, "장치", CU)
    o.append(line([(740, 492), (850, 492)], CU, 3)); o.append(line([(740, 508), (850, 508)], CU, 3))
    o.append(T(1000, 505, "UART: 1:1 점대점", 15, "bold", CU))
    box(650, 560, "MCU", CI)
    o.append(line([(740, 572), (1000, 572)], CI, 3)); o.append(line([(740, 588), (1000, 588)], CI, 3))
    for x in (790, 880): o.append(line([(x + 20, 588), (x + 20, 612)], CI, 2)); box(x, 612, "#" + ("1" if x == 790 else "2"), CI, 60)
    o.append(T(1010, 585, "I2C: 2선 버스+주소", 15, "bold", CI))
    box(650, 690, "MCU", CS)
    o.append(line([(740, 700), (980, 700)], CS, 3))
    for i, x in enumerate((790, 880)):
        o.append(line([(x + 30, 700), (x + 30, 740)], CS, 2)); box(x, 740, "#" + str(i + 1), CS, 60)
        o.append(line([(740, 715 + i * 8), (x + 10 + i * 6, 715 + i * 8), (x + 10 + i * 6, 740)], "#999", 1.5, "4 3"))
    o.append(T(1000, 705, "SPI: 공유선+장치별 CS", 15, "bold", CS))
    o.append(T(1000, 760, "1-Wire: 1선 버스+64비트 ID", 15, "bold", CW))
    o.append(T(1000, 782, "(아래 1-Wire 그림 참고)", 13, fill="#555"))
    return "s1_serial_common_principle_ko.svg", svg(W, H, o)


# ================================================================ s2 비교표
def compare():
    W, H = 1240, 840
    o = []
    title(o, "UART · I2C · SPI · 1-Wire 비교", "같은 '시리얼'이지만 선 개수, 클럭, 주소 방식, 속도가 다르다. 센서 데이터시트의 인터페이스 항목을 보고 고른다.")
    cols = [("항목", C["ink"]), ("UART", CU), ("I2C (TWI)", CI), ("SPI", CS), ("1-Wire", CW)]
    rows = [
        ("신호선", "TX, RX (+GND)", "SDA, SCL (+GND)", "SCK, MOSI, MISO, CS (+GND)", "DQ 1개 (+GND)"),
        ("클럭", "없음(비동기, baud 약속)", "SCL (마스터 생성)", "SCK (마스터 생성)", "없음(시간 슬롯)"),
        ("방향", "전이중", "반이중", "전이중", "반이중"),
        ("장치 선택", "1:1 연결", "7비트 주소", "장치마다 CS선", "64비트 ROM ID"),
        ("일반 속도", "9600~115200bps(~Mbps)", "100k/400kHz (HS 3.4MHz)", "수 MHz~수십 MHz", "약 15.4kbps"),
        ("통신 거리", "수 m (TTL 레벨)", "약 1m 이내(보드 위)", "수십 cm(고속일수록 짧게)", "수십 m 가능"),
        ("수신 확인", "없음(패리티는 선택)", "ACK/NACK 있음", "없음", "CRC로 확인"),
        ("풀업 저항", "불필요", "필요(SDA·SCL, 4.7k)", "불필요", "필요(DQ, 4.7k)"),
        ("ESP32 핀", "UART0 1/3(USB), UART2 16/17", "SDA21 · SCL22", "SCK18·MISO19·MOSI23·CS4", "아무 GPIO (예 13)"),
        ("UNO 핀", "D0/D1(USB 공용)", "A4 · A5", "D13·D12·D11·CS D10", "아무 핀 (예 D2)"),
        ("장점", "간단, PC와 바로 통신", "2선에 여러 장치, ACK", "가장 빠름, 풀업 불필요", "1선, 긴 배선, ID 내장"),
        ("단점", "1:1, 속도 불일치 시 깨짐", "느림, 주소 충돌", "선 많음, 장치당 CS", "느림, 타이밍 엄격"),
        ("9주차 예제", "시리얼 모니터·릴레이 명령", "BH1750 조도", "MAX6675 열전대", "DS18B20 온도"),
        ("그 밖의 센서", "GPS NEO-6M, PMS7003", "BMP280, SHT31, OLED", "MFRC522 RFID, SD카드", "DS1990 iButton"),
    ]
    xs = [40, 190, 445, 700, 955]
    ws = [150, 255, 255, 255, 245]
    y = 120
    for j, (s, col) in enumerate(cols):
        o.append(rect(xs[j], y, ws[j] - 4, 40, col if j else "#eef2f7", "none", 0, 6))
        o.append(T(xs[j] + ws[j] / 2 - 2, y + 27, s, 17, "bold", "white" if j else C["ink"], "middle"))
    for i, r in enumerate(rows):
        yy = y + 48 + i * 44
        o.append(rect(40, yy - 2, 1156, 40, "#ffffff" if i % 2 else "#f7f8fa", "#e3e7ee", 1, 4))
        for j, s in enumerate(r):
            hl = r[0] == "9주차 예제"
            o.append(T(xs[j] + 10, yy + 24, s, 14 if j else 15, "bold" if j == 0 or hl else "normal",
                       [C["ink"], CU, CI, CS, CW][j] if (j == 0 or hl) else "#222"))
    o.append(T(40, H - 18, "※ DHT11(10주차)은 1-Wire가 아니라 제조사 고유의 단일선 프로토콜이다. '선이 1개'라고 모두 1-Wire는 아니다.", 14, "bold", "#a40000"))
    return "s2_serial_compare_ko.svg", svg(W, H, o)


# ================================================================ s3 UART
def uart():
    W, H = 1240, 820
    o = []
    title(o, "UART — 비동기 시리얼: 프레임(8N1)과 TX↔RX 교차 연결",
          "클럭선이 없으므로 양쪽이 같은 속도(baud)를 쓰기로 약속한다. 쉬는 상태는 HIGH, START 비트(LOW)로 시작을 알린다.")
    card(o, 40, 110, 1160, 330, "① 'A'(0x41 = 0100 0001) 전송 — 8N1, LSB 먼저", CU)
    bits = [1, 1, 0] + [(0x41 >> i) & 1 for i in range(8)] + [1, 1, 1]
    labs = ["대기", "", "START"] + [f"D{i}" for i in range(8)] + ["STOP", "대기", ""]
    x0, w = 90, 80
    o += digital(x0, 260, bits, w, 80, CU, 4)
    for i, s in enumerate(labs):
        if s: o.append(T(x0 + i * w + w / 2, 205, s, 13, "bold", CU if s.startswith("D") else "#555", "middle"))
        if 3 <= i <= 10: o.append(T(x0 + i * w + w / 2, 330, str(bits[i]), 18, "bold", "#222", "middle"))
    for i in range(2, 12):
        o.append(line([(x0 + i * w, 190), (x0 + i * w, 340)], "#ccc", 1, "3 4"))
    o.append(T(60, 225, "HIGH", 12, fill="#888")); o.append(T(60, 305, "LOW", 12, fill="#888"))
    o += arrow(x0 + 2 * w, 360, x0 + 3 * w, 360, "#333", 2); o += arrow(x0 + 3 * w, 360, x0 + 2 * w, 360, "#333", 2)
    o.append(T(x0 + 2.5 * w, 385, "1비트 = 1/baud", 13, "bold", "#333", "middle"))
    o.append(T(560, 385, "프레임 10비트 → 115200bps면 초당 약 11,520문자", 14, fill="#333"))
    o.append(T(560, 412, "패리티(선택): 1의 개수로 오류 검출.  8N1 = 데이터 8, 패리티 None, 정지 1", 14, fill="#333"))
    # 연결
    card(o, 40, 460, 560, 330, "② 연결: TX → 상대 RX (교차), GND 공통", CU)
    def dev(x, y, nm):
        o.append(rect(x, y, 140, 150, C["board"], C["boardS"], 2, 10)); o.append(T(x + 70, y + 30, nm, 16, "bold", "white", "middle"))
        for k, p in enumerate(["TX", "RX", "GND"]): o.append(T(x + (125 if x < 300 else 15), y + 70 + k * 32, p, 15, "bold", "white", "end" if x < 300 else "start"))
    dev(70, 540, "장치 A"); dev(430, 540, "장치 B")
    o.append(line([(210, 605), (300, 605), (340, 637), (430, 637)], CU, 3.5))
    o.append(line([(210, 637), (300, 637), (340, 605), (430, 605)], CI, 3.5))
    o.append(line([(210, 669), (430, 669)], "#222", 3.5))
    o.append(T(320, 595, "A.TX → B.RX", 12, "bold", CU, "middle"))
    o.append(T(320, 660, "GND ─ GND", 12, "bold", "#222", "middle"))
    o.append(T(320, 740, "A.TX→B.RX, A.RX←B.TX (같은 이름끼리 연결하면 안 됨)", 13, "bold", "#c62828", "middle"))
    o.append(T(320, 764, "전압 레벨: ESP32 3.3V, UNO 5V, RS-232 ±12V — 직결 금지", 13, fill="#555", anchor="middle"))
    # USB 경로
    card(o, 620, 460, 580, 330, "③ 시리얼 모니터의 실제 경로", CU)
    chain = [("PC 앱", "#555"), ("USB", "#555"), ("USB-UART\n브리지", "#333"), ("UART0\nTX/RX", CU), ("ESP32", C["board"])]
    x = 640
    for i, (s, col) in enumerate(chain):
        o.append(rect(x, 530, 98, 70, "#fff", col, 2.5, 8))
        for k, part in enumerate(s.split("\n")):
            o.append(T(x + 49, (556 if "\n" in s else 570) + k * 20, part, 14, "bold", col, "middle"))
        if i < 4: o += arrow(x + 100, 565, x + 114, 565, "#333", 2)
        x += 114
    for i, s in enumerate(["• Serial.print() = UART0 송신 → PC 화면", "• PC 입력창 Enter = UART0 수신 → Serial.read()",
                           "• ESP32: UART0(USB), UART1, UART2(GPIO16/17) 3개", "• UNO: UART 1개(D0/D1) → USB와 공용, 꽂으면 업로드 방해",
                           "• 줄 끝 설정(Newline)이 '명령 한 줄'의 끝 표시가 된다"]):
        o.append(T(645, 640 + i * 28, s, 14))
    return "s3_uart_frame_ko.svg", svg(W, H, o)


# ================================================================ s4 I2C
def i2c():
    W, H = 1240, 820
    o = []
    title(o, "I2C — 2선 버스: SDA(데이터) · SCL(클럭), 주소로 장치 선택",
          "모든 장치가 같은 두 선에 붙는다. 선은 풀업 저항이 HIGH로 당기고, 장치는 LOW로 끌어내리기만 한다(오픈 드레인).")
    card(o, 40, 110, 1160, 340, "① BH1750(주소 0x23)에 명령 0x10 쓰기", CI)
    x0, q = 160, 9.5            # q = 1/4 비트 폭
    bits = [0, 1, 0, 0, 0, 1, 1] + [0] + [0] + [0, 0, 0, 1, 0, 0, 0, 0] + [0]
    scl = [1, 1, 1, 1]; sda = [1, 1, 0, 0]          # START: SCL=H에서 SDA 하강
    prev = 0
    for bt in bits:
        scl += [0, 0, 1, 1]; sda += [prev, bt, bt, bt]; prev = bt
    scl += [0, 0, 1, 1, 1, 1, 1]; sda += [prev, 0, 0, 0, 1, 1, 1]  # STOP: SCL=H에서 SDA 상승
    o += digital(x0, 210, scl, q, 40, "#555", 2.5)
    signal_label(o, x0 - 12, 210, "SCL", "#555")
    o += digital(x0, 300, sda, q, 40, CI, 3.5)
    signal_label(o, x0 - 12, 300, "SDA", CI)
    def brace(i0, n, txt, col, y=370):
        xa = x0 + (4 + 4 * i0) * q; xb = xa + 4 * n * q
        o.append(line([(xa + 2, y - 8), (xa + 2, y), (xb - 2, y), (xb - 2, y - 8)], col, 2))
        o.append(T((xa + xb) / 2, y + 22, txt, 13, "bold", col, "middle"))
    for i, bt in enumerate(bits):
        o.append(T(x0 + (4 + 4 * i + 3) * q, 345, str(bt), 11, fill="#888", anchor="middle"))
    o.append(T(x0 + 2 * q, 160, "START", 13, "bold", "#c62828", "middle"))
    o.append(T(x0 + 2 * q, 176, "SCL=H일 때 SDA↓", 11, fill="#c62828", anchor="middle"))
    brace(0, 7, "주소 0100011 = 0x23", CI); brace(7, 1, "W=0", "#555"); brace(8, 1, "ACK", "#c06000")
    brace(9, 8, "데이터 0x10 (측정 명령)", CI); brace(17, 1, "ACK", "#c06000")
    xs = x0 + (4 + 4 * 18 + 3) * q
    o.append(T(xs, 160, "STOP", 13, "bold", "#c62828", "middle"))
    o.append(T(xs, 176, "SCL=H일 때 SDA↑", 11, fill="#c62828", anchor="middle"))
    o.append(T(60, 425, "• 데이터는 SCL이 HIGH인 동안 안정, 바꾸는 것은 SCL LOW 동안 · ACK = 받는 쪽이 9번째 클럭에 SDA를 LOW로", 14, fill="#333"))
    # 버스 그림
    card(o, 40, 470, 700, 320, "② 버스 연결과 풀업", CI)
    o.append(rect(70, 560, 120, 150, C["board"], C["boardS"], 2, 10)); o.append(T(130, 590, "ESP32", 16, "bold", "white", "middle"))
    o.append(T(180, 640, "SDA 21", 13, "bold", "white", "end")); o.append(T(180, 680, "SCL 22", 13, "bold", "white", "end"))
    o.append(line([(190, 635), (700, 635)], CI, 3.5)); o.append(line([(190, 675), (700, 675)], "#555", 3.5))
    for k, (x, y) in enumerate(((250, 635), (310, 675))):
        o.append(line([(x, 548), (x, 558)], C["red"], 3))
        o.append(f'<rect x="{x-10}" y="558" width="20" height="56" rx="3" fill="#fff5da" stroke="#8a5818" stroke-width="2"/>')
        o.append(line([(x, 614), (x, y)], CI if k == 0 else "#555", 2.5)); o.append(circle(x, y, 5, CI if k == 0 else "#555"))
    o.append(line([(240, 548), (320, 548)], C["red"], 3)); o.append(T(330, 553, "3V3", 13, "bold", C["red"]))
    o.append(T(330, 595, "풀업 4.7kΩ × 2", 13, "bold", "#8a5818"))
    for i, (nm, ad) in enumerate((("BH1750", "0x23"), ("BMP280", "0x76"), ("OLED", "0x3C"))):
        x = 400 + i * 110
        o.append(line([(x + 20, 635), (x + 20, 715)], CI, 2)); o.append(line([(x + 60, 675), (x + 60, 715)], "#555", 2))
        o.append(rect(x, 715, 85, 50, "#fff", CI, 2.5, 6)); o.append(T(x + 42, 737, nm, 13, "bold", CI, "middle")); o.append(T(x + 42, 755, ad, 12, fill="#555", anchor="middle"))
    o.append(T(60, 785, "모듈 대부분은 풀업이 내장 → 여러 개를 꽂으면 병렬로 너무 강해질 수 있음", 12, fill="#555"))
    card(o, 760, 470, 440, 320, "③ 기억할 점", CI)
    for i, s in enumerate(["• 주소 = 7비트(0x08~0x77)", "• 같은 주소 2개는 충돌 → ADDR 핀으로 변경", "• 속도 100kHz(표준), 400kHz(고속)",
                           "• 짧은 거리(보드 위·수십 cm)", "• 3.3V 버스에 5V 풀업 모듈 주의", "• 장치 확인: I2C 스캐너(예제 6B)",
                           "• Wire.beginTransmission → write", "  → endTransmission (0이면 ACK)", "• Wire.requestFrom(주소, 바이트수)"]):
        o.append(T(785, 528 + i * 29, s, 14, "bold" if "requestFrom" in s or "beginTransmission" in s else "normal"))
    return "s4_i2c_protocol_ko.svg", svg(W, H, o)


# ================================================================ s5 SPI
def spi():
    W, H = 1240, 800
    o = []
    title(o, "SPI — 4선 동기식: SCK · MOSI · MISO · CS(장치 선택)",
          "마스터가 CS를 LOW로 내려 장치를 고르고, SCK 한 번에 1비트씩 동시에 주고받는다(전이중). 주소가 없는 대신 장치마다 CS선이 하나씩.")
    card(o, 40, 110, 1160, 330, "① MAX6675 읽기: CS↓ → 16클럭 → MISO로 16비트 → CS↑ (모드 0)", CS)
    x0, w = 170, 20
    cs = [1, 1] + [0] * 33 + [1, 1, 1]
    sck = [0, 0, 0] + [1, 0] * 16 + [0, 0, 0]
    val = 0x0C80  # 100.00°C → (400<<3)
    miso = [1, 1, 0]
    for i in range(15, -1, -1):
        b = (val >> i) & 1; miso += [b, b]
    miso += [1, 1, 1]
    o += digital(x0, 180, cs, w, 30, "#c62828", 3); signal_label(o, x0 - 12, 180, "CS", "#c62828")
    o += digital(x0, 250, sck, w, 30, "#555", 2.5); signal_label(o, x0 - 12, 250, "SCK", "#555")
    o += digital(x0, 320, miso, w, 30, CS, 3.5); signal_label(o, x0 - 12, 320, "MISO", CS)
    o.append(line([(x0, 380), (x0 + len(cs) * w, 380)], "#bbb", 2, "6 5")); signal_label(o, x0 - 12, 380, "MOSI", "#999")
    o.append(T(x0 + 18 * w, 404, "MAX6675는 MOSI 없음(읽기 전용)", 12, fill="#999", anchor="middle"))
    for i in range(16):
        o.append(T(x0 + (3 + 2 * i) * w + w, 352, str(15 - i), 10, fill="#888", anchor="middle"))
    o.append(T(x0 + 38 * w + 30, 300, "예: 0x0C80 → (0x0C80 >> 3) × 0.25", 14, "bold", CS))
    o.append(T(x0 + 38 * w + 30, 324, "= 400 × 0.25 = 100.00 °C", 14, "bold", CS))
    o.append(T(x0 + 38 * w + 30, 352, "bit2 = 1 이면 열전대 끊김", 13, fill="#555"))
    o.append(T(x0 + 38 * w + 30, 180, "CS LOW 동안만 통신", 13, fill="#c62828"))
    o.append(T(x0 + 38 * w + 30, 250, "SCK 상승 에지에서 읽음", 13, fill="#555"))
    card(o, 40, 460, 580, 310, "② 연결 (장치가 여러 개면 CS만 추가)", CS)
    o.append(rect(70, 530, 130, 230, C["board"], C["boardS"], 2, 10)); o.append(T(135, 558, "ESP32", 16, "bold", "white", "middle"))
    pins = [("SCK 18", 590, "#555"), ("MISO 19", 625, CS), ("MOSI 23", 660, "#999"), ("CS 4", 695, "#c62828")]
    for nm, y, col in pins:
        o.append(T(192, y + 5, nm, 13, "bold", "white", "end"))
        o.append(line([(200, y), (420 if nm.startswith("CS") else 560, y)], col, 3))
    for k, (nm, cy) in enumerate((("MAX6675", 0), ("SD카드", 1))):
        x = 330 + k * 150
        o.append(rect(x, 712, 110, 46, "#fff", CS, 2.5, 6)); o.append(T(x + 55, 741, nm, 13, "bold", CS, "middle"))
        for y in (590, 625, 660): o.append(line([(x + 20 + (y - 590) / 2, y), (x + 20 + (y - 590) / 2, 712)], "#bbb", 1.5))
    o.append(line([(420, 695), (420, 712)], "#c62828", 3))
    o.append(line([(200, 735), (230, 735), (230, 766), (535, 766), (535, 758)], "#e57373", 3, "6 4"))
    o.append(T(192, 740, "CS2 (예 15)", 12, "bold", "white", "end"))
    card(o, 640, 460, 560, 310, "③ 기억할 점", CS)
    for i, s in enumerate(["• 가장 빠름: 수 MHz~수십 MHz (I2C의 10배 이상)", "• 주소 없음 → 장치 수만큼 CS선 필요",
                           "• 모드 0~3 = 클럭 극성(CPOL)·위상(CPHA) 조합", "  → 데이터시트 모드와 SPISettings를 맞춘다",
                           "• MOSI=Master Out, MISO=Master In (COPI/CIPO)", "• ESP32 VSPI 기본: SCK18 MISO19 MOSI23 SS5",
                           "  (GPIO5는 부팅 스트랩 → 9주차 CS는 GPIO4)", "• SPI.beginTransaction → CS LOW → transfer → CS HIGH"]):
        o.append(T(660, 518 + i * 30, s, 14, "bold" if "transfer" in s else "normal"))
    return "s5_spi_protocol_ko.svg", svg(W, H, o)


# ================================================================ s6 1-Wire
def onewire():
    W, H = 1240, 850
    o = []
    title(o, "1-Wire — 데이터선 1개(+GND), 64비트 고유 ID로 장치 구별",
          "DQ선은 4.7kΩ 풀업으로 평소 HIGH. 마스터가 LOW로 끌어내리는 '시간 길이'로 0/1과 리셋을 구분한다(시간 슬롯).")
    card(o, 40, 110, 1160, 360, "① 리셋·존재 펄스와 비트 쓰기/읽기 시간 슬롯", CW)
    y, h = 240, 70
    def seg(pts, col=CW): o.append(line(pts, col, 3.5, cap="butt"))
    x0 = 80; hi = y - h / 2; lo = y + h / 2
    seg([(x0, hi), (x0 + 40, hi), (x0 + 40, lo), (x0 + 240, lo), (x0 + 240, hi), (x0 + 290, hi), (x0 + 290, lo)])
    seg([(x0 + 290, lo), (x0 + 400, lo), (x0 + 400, hi), (x0 + 470, hi)], "#c06000")
    o.append(T(x0 + 140, lo + 26, "마스터 LOW ≥ 480µs = RESET", 13, "bold", CW, "middle"))
    o.append(T(x0 + 345, lo + 26, "센서 LOW 60~240µs", 13, "bold", "#c06000", "middle"))
    o.append(T(x0 + 345, lo + 44, "= PRESENCE(나 여기 있어)", 13, "bold", "#c06000", "middle"))
    x1 = 640
    seg([(x1, hi), (x1 + 20, hi), (x1 + 20, lo), (x1 + 120, lo), (x1 + 120, hi), (x1 + 150, hi)])
    o.append(T(x1 + 80, hi - 12, "쓰기 0: LOW 60µs+", 13, "bold", CW, "middle"))
    x2 = 820
    seg([(x2, hi), (x2 + 10, hi), (x2 + 10, lo), (x2 + 30, lo), (x2 + 30, hi), (x2 + 150, hi)])
    o.append(T(x2 + 75, hi - 12, "쓰기 1: LOW 1~15µs", 13, "bold", CW, "middle"))
    x3 = 1000
    seg([(x3, hi), (x3 + 10, hi), (x3 + 10, lo), (x3 + 25, lo)])
    seg([(x3 + 25, lo), (x3 + 70, lo), (x3 + 70, hi), (x3 + 150, hi)], "#c06000")
    o.append(line([(x3 + 45, hi - 20), (x3 + 45, lo + 20)], "#c62828", 2, "4 3"))
    o.append(T(x3 + 75, hi - 28, "읽기: 15µs 시점 샘플", 13, "bold", CW, "middle"))
    o.append(T(x3 + 100, lo + 26, "센서가 LOW 유지 = 0", 12, fill="#c06000", anchor="middle"))
    o.append(T(60, 340, "한 바이트 = 8개 시간 슬롯(LSB 먼저). 슬롯 하나 ≈ 60~120µs → 약 15kbps. 타이밍이 정확해야 해서 라이브러리(OneWire)가 처리한다.", 14, fill="#333"))
    o.append(T(60, 372, "명령 흐름:  RESET → ROM 명령(0x55 MATCH ROM + 64비트 ID / 0xCC SKIP ROM) → 기능 명령(0x44 변환, 0xBE 읽기) → 데이터 9바이트(CRC 포함)", 14, "bold", CW))
    o.append(T(60, 404, "64비트 ROM ID = 패밀리코드 1바이트(DS18B20=0x28) + 일련번호 6바이트 + CRC 1바이트 → 공장에서 정해져 겹치지 않는다", 14, fill="#333"))
    card(o, 40, 490, 700, 330, "② 연결: 여러 센서를 한 선에", CW)
    o.append(rect(70, 560, 120, 150, C["board"], C["boardS"], 2, 10)); o.append(T(130, 590, "ESP32", 16, "bold", "white", "middle"))
    o.append(T(180, 645, "GPIO13", 13, "bold", "white", "end")); o.append(T(180, 695, "GND", 13, "bold", "white", "end"))
    o.append(line([(190, 640), (700, 640)], CW, 3.5)); o.append(line([(190, 690), (700, 690)], "#222", 3))
    o.append(line([(250, 545), (250, 560)], C["red"], 3)); o += sym_resistor_v(250, 560, 635, "4.7kΩ")
    o.append(circle(250, 640, 5, CW)); o.append(T(262, 552, "3V3", 13, "bold", C["red"]))
    for i in range(3):
        x = 400 + i * 110
        o.append(f'<path d="M{x} {745} L{x} {720} A22 22 0 0 1 {x+44} {720} L{x+44} {745} Z" fill="#222"/>')
        o.append(line([(x + 22, 640), (x + 22, 698)], CW, 2)); o.append(line([(x + 8, 690), (x + 8, 698)], "#222", 2))
        o.append(T(x + 22, 768, f"ID …{['A3','5F','C1'][i]}", 12, fill="#555", anchor="middle"))
    o.append(T(60, 805, "VDD는 3V3(또는 UNO 5V)에 연결(외부 전원 방식). 기생 전원 방식은 9주차에서 다루지 않음", 12, fill="#555"))
    card(o, 760, 490, 440, 330, "③ 기억할 점", CW)
    for i, s in enumerate(["• 신호선 1개 + GND (전원선 포함 3선)", "• 4.7kΩ 풀업 없으면 '센서 없음'", "• 느리지만 수십 m까지 가능",
                           "• 장치마다 고유 ID → 주소 설정 불필요", "• DS18B20: -55~125°C, ±0.5°C", "• 12비트 변환에 최대 750ms",
                           "• DHT11은 1-Wire가 아니다(10주차)"]):
        o.append(T(785, 548 + i * 33, s, 14, "bold" if "DHT11" in s else "normal", "#a40000" if "DHT11" in s else "#222"))
    return "s6_onewire_protocol_ko.svg", svg(W, H, o)


# ================================================================ s7 물리 계층 확장·선택 기준
def physical():
    W, H = 1240, 870
    o = []
    title(o, "UART를 멀리 보내기 — TTL · RS-232 · RS-485, 그리고 CAN · 방식 선택 기준",
          "UART는 '데이터 형식(프레임)'이고, RS-232·RS-485는 그 신호를 멀리 보내는 '전기 규격(물리 계층)'이다. CAN은 차동 2선 위의 메시지 버스.")
    # 단일 종단 vs 차동
    card(o, 40, 110, 560, 330, "① 단일 종단(Single-ended) vs 차동(Differential)", C["ink"])
    import math
    def noisy(x0, y, bits, col, invert=False, noise=True, w=56, h=46):
        pts = []
        for i, b in enumerate(bits):
            b = 1 - b if invert else b
            for k in range(8):
                x = x0 + i * w + k * w / 8
                n = 9 * math.sin((x - x0) / 9.0) if noise else 0
                pts.append((x, y - (h / 2 if b else -h / 2) + n))
        o.append(line(pts, col, 2.5, cap="butt"))
    bits = [1, 0, 1, 1, 0, 1, 0, 0]
    o.append(T(70, 175, "단일 종단(TTL UART, RS-232): 신호 − GND", 14, "bold", CU))
    noisy(80, 220, bits, CU)
    o.append(T(70, 270, "→ 잡음이 그대로 신호에 더해진다", 13, fill="#555"))
    o.append(T(70, 305, "차동(RS-485, CAN): A − B 두 선의 차이", 14, "bold", CI))
    noisy(80, 345, bits, CI); noisy(80, 345, bits, "#c06000", invert=True)
    o.append(T(70, 405, "→ 두 선에 같은 잡음이 들어와도 A−B에서 상쇄(공통 모드 제거)", 13, fill="#555"))
    o.append(T(70, 428, "   꼬임선(Twisted pair) + 양 끝 120Ω 종단 저항", 13, fill="#555"))
    # 표
    card(o, 620, 110, 580, 330, "② 물리 계층 비교", C["ink"])
    rows = [("", "TTL UART", "RS-232", "RS-485", "CAN"),
            ("신호", "0/3.3V·5V", "±3~15V 역논리", "차동 A/B", "차동 H/L"),
            ("구조", "1:1", "1:1", "다중(32대)", "다중 마스터"),
            ("거리", "수 m", "약 15m", "약 1200m", "40m@1M"),
            ("속도", "~Mbps", "~115kbps", "~10Mbps(단거리)", "1M(FD 8M)"),
            ("칩", "직결", "MAX3232", "MAX485", "SN65HVD230"),
            ("용도", "센서·디버그", "PLC·구형 장비", "Modbus·산업", "자동차·로봇")]
    xs = [640, 700, 820, 940, 1070]
    for i, r in enumerate(rows):
        y = 170 + i * 38
        if i == 0: o.append(rect(632, y - 22, 560, 30, "#eef2f7", "none", 0, 4))
        for j, t in enumerate(r):
            o.append(T(xs[j], y, t, 13, "bold" if i == 0 or j == 0 else "normal", C["ink"]))
    o.append(T(640, 432, "ESP32: UART2+MAX3485(RS-485), 내장 TWAI+트랜시버(CAN)", 12, fill="#555"))
    # 선택 기준
    card(o, 40, 460, 1160, 380, "③ 어떤 방식을 고를까 — 정답은 '데이터시트가 지원하는 것' + 아래 기준", C["ink"])
    choices = [("속도가 최우선(화면·SD·고속 ADC)", "SPI", CS), ("핀을 아끼며 저속 센서 여러 개", "I2C", CI),
               ("PC·모듈과 1:1, 디버그 출력", "UART", CU), ("센서 여러 개를 긴 선 하나로(온도)", "1-Wire", CW),
               ("수십~수백 m, 산업 현장 다수 장치", "RS-485", "#555"), ("실시간·고신뢰 다중 노드(차량)", "CAN", "#555")]
    for i, (q, a, col) in enumerate(choices):
        y = 552 + i * 50
        o.append(rect(70, y - 26, 560, 40, "#fff", "#c9ced6", 1.5, 8)); o.append(T(90, y, q, 15))
        o += arrow(640, y - 6, 700, y - 6, "#333", 2.5)
        o.append(rect(710, y - 26, 130, 40, col, "none", 0, 8)); o.append(T(775, y, a, 16, "bold", "white", "middle"))
    for i, t in enumerate(["• 실제 제품은 여러 방식을 섞어 쓴다", "  (예: I2C 센서 + SPI 화면 + UART GPS)",
                           "• I2C로 큰 화면을 빠르게 갱신하려다", "  속도 한계로 실패하는 경우가 흔하다",
                           "• 거리가 늘면 속도를 낮추거나", "  차동 규격으로 바꾼다"]):
        o.append(T(880, 552 + i * 30, t, 14, "bold" if t.startswith("•") else "normal"))
    return "s7_physical_layer_selection_ko.svg", svg(W, H, o)


# ================================================================ 연결도 공통
def board(o, x, y, name, pins, w=230, color=None):
    """pins: [(label, y_offset)] right side. returns dict label->(x,y)"""
    hgt = max(p[1] for p in pins) + 60
    o.append(rect(x, y, w, hgt, color or C["board"], C["boardS"], 3, 14))
    o.append(T(x + w / 2, y + 32, name, 19, "bold", "white", "middle"))
    pos = {}
    for lab, dy in pins:
        yy = y + dy
        o.append(f'<rect x="{x+w-22}" y="{yy-8}" width="16" height="16" rx="2" fill="#111"/>')
        o.append(T(x + w - 30, yy + 5, lab, 15, "bold", "white", "end"))
        pos[lab] = (x + w - 6, yy)
    return pos


def module(o, x, y, w, name, pins, color="#2f5d8a", sub=None, left=True):
    """pins on left side: [(label)] spaced 34px; returns label->(x,y)"""
    hgt = len(pins) * 34 + 50
    o.append(rect(x, y, w, hgt, color, "#173552", 2.5, 8))
    o.append(T(x + w / 2, y + 24, name, 15, "bold", "white", "middle"))
    if sub: o.append(T(x + w / 2, y + 42, sub, 11, fill="#dce9f7", anchor="middle"))
    pos = {}
    for i, lab in enumerate(pins):
        yy = y + 62 + i * 34 - 8
        px = x if left else x + w
        o.append(circle(px, yy, 6, "#d4af37", "#7a5d00", 1.5))
        o.append(T(px + (12 if left else -12), yy + 5, lab, 13, "bold", "white", "start" if left else "end"))
        pos[lab] = (px, yy)
    return pos


def wire(o, a, b, color, mid=None, w=3.5):
    (x1, y1), (x2, y2) = a, b
    mx = mid if mid is not None else (x1 + x2) / 2
    o.append(line([(x1, y1), (mx, y1), (mx, y2), (x2, y2)], color, w))
    o.append(dot(x2, y2, color, 4))


def pin_table(o, x, y, rows, wcol=(170, 260, 260, 360), head=("모듈 핀", "ESP32-DevKitC V4", "Arduino UNO", "설명")):
    xs = [x]
    for wv in wcol[:-1]: xs.append(xs[-1] + wv)
    tw = sum(wcol)
    o.append(rect(x - 10, y - 30, tw + 20, len(rows) * 30 + 48, "#f3f4f6", "#9aa3ad", 2, 10))
    for j, s in enumerate(head): o.append(T(xs[j], y, s, 14, "bold", C["ink"]))
    for i, r in enumerate(rows):
        for j, s in enumerate(r):
            o.append(T(xs[j], y + 30 + i * 30, s, 14, "bold" if j == 0 else "normal"))
    return y + len(rows) * 30 + 30


# ================================================================ w5 UART 양방향 릴레이
def w_relay(kind):
    W, H = 1240, 900
    o = []
    esp = kind == "esp32"
    bname = "ESP32-DevKitC V4" if esp else "Arduino UNO R3"
    title(o, f"예제 5 · UART 양방향 — 센서값 송신 + PC 명령으로 릴레이 제어 ({'ESP32' if esp else 'UNO'})",
          "PC ⇄ USB ⇄ 보드: 보드는 1초마다 'DATA,…'를 보내고, PC가 'ON/OFF/AUTO 60'을 보내면 릴레이를 켠다. 부하는 LED(저전압)만 사용.")
    # PC
    o.append(rect(40, 130, 170, 120, "#eef2f7", "#555", 2, 10)); o.append(T(125, 165, "PC", 18, "bold", "#333", "middle"))
    o.append(T(125, 192, "시리얼 모니터", 13, fill="#333", anchor="middle")); o.append(T(125, 212, "115200 · Newline", 12, fill="#555", anchor="middle"))
    o += arrow(215, 175, 300, 175, CU, 3); o.append(T(258, 165, "명령 ON", 12, "bold", CU, "middle"))
    o += arrow(300, 215, 215, 215, CI, 3); o.append(T(258, 238, "DATA,…", 12, "bold", CI, "middle"))
    pins = ([("5V", 70), ("3V3", 110), ("GPIO34", 150), ("GPIO32", 190), ("GND", 230)] if esp else
            [("5V", 70), ("A0", 150), ("D7", 190), ("GND", 230)])
    bp = board(o, 300, 110, bname, pins, 250)
    o.append(T(425, 365, "USB 데이터 케이블 → PC", 12, fill="#dce9f7", anchor="middle"))
    # pot
    px, py = 700, 120
    o.append(rect(px, py, 150, 120, "#2f5d8a", "#173552", 2.5, 8)); o.append(T(px + 75, py + 24, "가변저항 10kΩ", 14, "bold", "white", "middle"))
    o.append(circle(px + 75, py + 68, 22, "#d8dde3", "#555", 2))
    pp = {}
    for k, lab in enumerate(["1", "2", "3"]):
        xx = px + 30 + k * 45
        o.append(line([(xx, py + 120), (xx, py + 140)], "#999", 3)); o.append(circle(xx, py + 145, 5, "#d4af37")); pp[lab] = (xx, py + 145)
        o.append(T(xx, py + 112, lab, 12, "bold", "white", "middle"))
    vref = "3V3" if esp else "5V"
    o.append(line([bp[vref], (620, bp[vref][1]), (620, 300), (pp["1"][0], 300), pp["1"]], C["red"], 3.5))
    sig = "GPIO34" if esp else "A0"
    o.append(line([bp[sig], (640, bp[sig][1]), (640, 320), (pp["2"][0], 320), pp["2"]], C["teal"], 3.5))
    o.append(line([bp["GND"], (660, bp["GND"][1]), (660, 340), (pp["3"][0], 340), pp["3"]], "#222", 3.5))
    # relay module
    rx, ry = 700, 420
    o.append(rect(rx, ry, 260, 190, "#1b5e7e", "#0e3a4f", 2.5, 10))
    o.append(T(rx + 130, ry + 28, "1채널 릴레이 모듈", 15, "bold", "white", "middle"))
    o.append(rect(rx + 110, ry + 50, 90, 70, "#2a6fc9", "#123", 2, 4)); o.append(T(rx + 155, ry + 90, "RELAY", 12, "bold", "white", "middle"))
    o.append(T(rx + 155, ry + 108, "5V 코일", 11, fill="#dce9f7", anchor="middle"))
    rp = {}
    for k, lab in enumerate(["VCC", "GND", "IN"]):
        yy = ry + 70 + k * 40
        o.append(circle(rx, yy, 6, "#d4af37", "#7a5d00", 1.5)); o.append(T(rx + 14, yy + 5, lab, 13, "bold", "white")); rp[lab] = (rx, yy)
    for k, lab in enumerate(["NO", "COM", "NC"]):
        yy = ry + 70 + k * 40
        o.append(rect(rx + 230, yy - 12, 40, 24, "#2b7a3d", "#123", 1.5, 3)); o.append(T(rx + 250, yy + 5, lab, 11, "bold", "white", "middle")); rp[lab] = (rx + 270, yy)
    o.append(line([bp["5V"], (580, bp["5V"][1]), (580, 490), rp["VCC"]], "#a40000", 3.5))
    o.append(line([(660, 340), (660, 530), rp["GND"]], "#222", 3.5))
    ctl = "GPIO32" if esp else "D7"
    o.append(line([bp[ctl], (600, bp[ctl][1]), (600, 570), rp["IN"]], CU, 3.5))
    # load: COM ← 5V, NO → 330Ω → LED → GND
    lx = 1030
    o.append(T(lx + 70, ry - 20, "저전압 부하(예)", 14, "bold", "#333", "middle"))
    o.append(line([rp["COM"], (lx + 20, rp["COM"][1])], "#a40000", 3)); o.append(T(lx + 28, rp["COM"][1] + 5, "← 보드 5V", 12, "bold", "#a40000"))
    o.append(line([rp["NO"], (lx + 120, rp["NO"][1]), (lx + 120, ry + 50)], "#333", 3))
    o.append(f'<rect x="{lx+108}" y="{ry+50}" width="24" height="50" rx="3" fill="#fff5da" stroke="#8a5818" stroke-width="2"/>')
    o.append(T(lx + 140, ry + 80, "330Ω", 12, fill="#333"))
    o.append(line([(lx + 120, ry + 100), (lx + 120, ry + 120)], "#333", 3))
    o.append(f'<path d="M{lx+106} {ry+120} L{lx+134} {ry+120} L{lx+120} {ry+144} Z" fill="{C["green"]}" stroke="#222" stroke-width="1.5"/>')
    o.append(line([(lx + 106, ry + 144), (lx + 134, ry + 144)], "#222", 3)); o.append(T(lx + 142, ry + 138, "LED", 12, "bold", C["green"]))
    o.append(line([(lx + 120, ry + 144), (lx + 120, ry + 170)], "#222", 3)); o += sym_gnd(lx + 120, ry + 170)
    o.append(T(lx + 70, ry + 225, "릴레이 ON → COM–NO 연결 → LED 켜짐", 12, fill="#555", anchor="middle"))
    # notes
    y0 = 680
    notes = (["• 릴레이 VCC=5V 핀(USB 전원), GND 공통, IN=GPIO32",
              "• 5V 코일 Active-LOW 모듈 + 3.3V GPIO: HIGH(3.3V)로는 안 꺼질 수 있다",
              "  → 코드는 '끌 때 핀을 INPUT(고임피던스)'으로 바꾼다 (relayWrite 함수)",
              "• 또는 3.3V 트리거 모듈 / High-Low 점퍼형 모듈을 사용"] if esp else
             ["• UNO는 5V 논리 → 5V 릴레이 모듈 IN에 D7을 바로 연결",
              "• 대부분 Active-LOW: D7=LOW이면 ON (코드 RELAY_ACTIVE_LOW=true)",
              "• UNO의 USB 시리얼은 D0/D1과 공용 → D0/D1에 아무것도 꽂지 않는다",
              "• 같은 스케치(05_uart_bidir_relay)가 UNO에서도 동작(핀 자동 선택)"])
    o.append(rect(40, y0, 1160, 190, "#f3f4f6", "#9aa3ad", 2, 10))
    o.append(T(60, y0 + 30, "연결 주의", 16, "bold"))
    for i, s in enumerate(notes): o.append(T(60, y0 + 62 + i * 28, s, 14))
    o.append(T(60, y0 + 175, "⚠ 실습에서 릴레이 접점에 220V 등 상용 전원을 연결하지 않는다. 5V 이하 저전압 부하만.", 14, "bold", "#c62828"))
    return f"ex5_uart_relay_{kind}_ko.svg", svg(W, H, o)


def w_loopback():
    W, H = 1240, 560
    o = []
    title(o, "예제 5B · ESP32 UART2 루프백 — TX2(GPIO17) ↔ RX2(GPIO16) 점퍼 1개",
          "PC에서 쓴 글자가 UART0 → 코드 → UART2 TX → 점퍼 → UART2 RX → UART0 → PC로 되돌아온다. 점퍼를 빼면 돌아오지 않는다.")
    bp = board(o, 80, 120, "ESP32-DevKitC V4", [("GPIO17 TX2", 90), ("GPIO16 RX2", 150), ("TX0/RX0 (USB)", 230)], 280)
    a, b = bp["GPIO17 TX2"], bp["GPIO16 RX2"]
    o.append(line([a, (520, a[1]), (520, b[1]), b], "#e07b16", 5)); o.append(T(535, (a[1] + b[1]) / 2 + 5, "점퍼선(M-F 2개 또는 F-F 1개)", 15, "bold", "#e07b16"))
    o.append(line([bp["TX0/RX0 (USB)"], (470, bp["TX0/RX0 (USB)"][1])], "#555", 3)); o.append(T(480, bp["TX0/RX0 (USB)"][1] + 5, "USB → PC 시리얼 모니터", 14, fill="#555"))
    flow = ["키보드 'A'", "UART0 RX", "Serial2.write", "TX2 17", "점퍼", "RX2 16", "Serial2.read", "UART0 TX", "[UART2 RX] 0x41 'A'"]
    x = 60
    for i, s in enumerate(flow):
        wv = max(90, len(s) * 11)
        o.append(rect(x, 420, wv, 44, "#fff", CU if i not in (4,) else "#e07b16", 2, 8)); o.append(T(x + wv / 2, 447, s, 12, "bold", CU, "middle"))
        if i < len(flow) - 1: o += arrow(x + wv + 2, 442, x + wv + 14, 442, "#333", 2)
        x += wv + 16
    o.append(T(60, 510, "실험: '#'을 보내 UART2 baud를 9600 ↔ 115200으로 바꿔도 송·수신이 같은 UART2라 그대로 맞는다. 양쪽 baud가 다르면 글자가 깨진다(토론).", 14, fill="#333"))
    return "ex5b_uart2_loopback_ko.svg", svg(W, H, o)


def w_i2c():
    W, H = 1240, 760
    o = []
    title(o, "예제 6 · I2C 조도 센서 BH1750(GY-302) 연결도",
          "4선(VCC·GND·SCL·SDA)만 연결. ADDR 핀을 GND 또는 미연결 → 주소 0x23. 먼저 I2C 스캐너(6B)로 0x23이 보이는지 확인한다.")
    bp = board(o, 80, 120, "ESP32-DevKitC V4", [("3V3", 80), ("GND", 130), ("GPIO22 SCL", 190), ("GPIO21 SDA", 240)], 280)
    mp = module(o, 760, 140, 220, "BH1750 (GY-302)", ["VCC", "GND", "SCL", "SDA", "ADDR"], "#1f6f5c", "조도 1~65535 lx")
    o.append(circle(870, 120, 0, "none"))
    o.append(rect(840, 400, 60, 30, "#f2f2f2", "#aaa", 1.5, 15)); o.append(T(870, 450, "수광부를 빛 쪽으로", 12, fill="#555", anchor="middle"))
    wire(o, bp["3V3"], mp["VCC"], C["red"], 560)
    wire(o, bp["GND"], mp["GND"], "#222", 590)
    wire(o, bp["GPIO22 SCL"], mp["SCL"], "#555", 620)
    wire(o, bp["GPIO21 SDA"], mp["SDA"], CI, 650)
    o.append(T(700, mp["ADDR"][1] + 5, "ADDR: 미연결(=0x23)", 12, fill="#555", anchor="end"))
    yt = pin_table(o, 60, 520, [("VCC", "3V3", "5V (또는 3.3V)", "모듈에 3.3V 레귤레이터 내장"), ("GND", "GND", "GND", "공통 GND"),
                                ("SCL", "GPIO22", "A5", "클럭(마스터가 생성)"), ("SDA", "GPIO21", "A4", "데이터(양방향)"),
                                ("ADDR", "미연결/GND", "미연결/GND", "VCC로 올리면 주소 0x5C")])
    return "ex6_i2c_bh1750_ko.svg", svg(W, yt + 20, o)


def w_spi():
    W, H = 1240, 760
    o = []
    title(o, "예제 7 · SPI 열전대 온도 센서 MAX6675 연결도",
          "5선(VCC·GND·SCK·CS·SO). SO = MISO(센서→MCU). K형 열전대의 +/− 극성을 단자에 맞게 끼운다(빨강 +, 파랑/노랑 −는 제품마다 확인).")
    bp = board(o, 80, 120, "ESP32-DevKitC V4", [("3V3", 80), ("GND", 130), ("GPIO18 SCK", 190), ("GPIO4 CS", 240), ("GPIO19 MISO", 290)], 280)
    mp = module(o, 760, 140, 220, "MAX6675 모듈", ["GND", "VCC", "SCK", "CS", "SO"], "#7a3e12", "K형 0~1024°C · 0.25°C")
    wire(o, bp["GND"], mp["GND"], "#222", 560)
    wire(o, bp["3V3"], mp["VCC"], C["red"], 590)
    wire(o, bp["GPIO18 SCK"], mp["SCK"], "#555", 620)
    wire(o, bp["GPIO4 CS"], mp["CS"], "#c62828", 650)
    wire(o, bp["GPIO19 MISO"], mp["SO"], CS, 680)
    # thermocouple
    o.append(rect(980, 160, 40, 70, "#2b7a3d", "#123", 1.5, 3)); o.append(T(1000, 185, "+", 16, "bold", "white", "middle")); o.append(T(1000, 220, "−", 16, "bold", "white", "middle"))
    o.append(line([(1020, 180), (1120, 180), (1150, 290)], "#c62828", 4)); o.append(line([(1020, 215), (1110, 215), (1150, 290)], "#2664bf", 4))
    o.append(circle(1150, 292, 7, "#888")); o.append(T(1150, 320, "열전대 끝(측정점)", 12, fill="#555", anchor="middle"))
    yt = pin_table(o, 60, 540, [("VCC", "3V3", "5V", "3.0~5.5V"), ("GND", "GND", "GND", "공통 GND"), ("SCK", "GPIO18", "D13", "클럭(최대 4.3MHz)"),
                                ("CS", "GPIO4", "D10", "LOW=선택·읽기, HIGH=변환"), ("SO", "GPIO19 (MISO)", "D12 (MISO)", "데이터 출력 16비트")])
    return "ex7_spi_max6675_ko.svg", svg(W, yt + 20, o)


def w_ow():
    W, H = 1240, 760
    o = []
    title(o, "예제 8 · 1-Wire 온도 센서 DS18B20 연결도",
          "TO-92 패키지: 평평한 면을 앞으로, 다리를 아래로 두면 왼쪽부터 GND · DQ · VDD. DQ와 VDD 사이에 4.7kΩ 풀업을 반드시 넣는다.")
    bp = board(o, 80, 120, "ESP32-DevKitC V4", [("3V3", 80), ("GPIO13", 160), ("GND", 240)], 280)
    # TO-92
    cx = 860
    o.append(f'<path d="M{cx-50} 290 L{cx-50} 200 A50 50 0 0 1 {cx+50} 200 L{cx+50} 290 Z" fill="#222"/>')
    o.append(T(cx, 250, "DS18B20", 14, "bold", "white", "middle")); o.append(T(cx, 270, "평평한 면", 11, fill="#bbb", anchor="middle"))
    leg = {}
    for k, (lab, dx) in enumerate((("GND", -30), ("DQ", 0), ("VDD", 30))):
        o.append(line([(cx + dx, 290), (cx + dx, 380)], "#999", 4)); o.append(circle(cx + dx, 384, 6, "#d4af37"))
        o.append(T(cx + dx, 284, lab, 10, "bold", "white", "middle")); leg[lab] = (cx + dx, 384)
    o.append(line([bp["GND"], (620, bp["GND"][1]), (620, 460), (leg["GND"][0], 460), leg["GND"]], "#222", 3.5))
    o.append(line([bp["GPIO13"], (650, bp["GPIO13"][1]), (650, 440), (leg["DQ"][0], 440), leg["DQ"]], CW, 3.5))
    o.append(line([bp["3V3"], (590, bp["3V3"][1]), (590, 135), (1000, 135), (1000, 480), (leg["VDD"][0], 480), leg["VDD"]], C["red"], 3.5))
    # pull-up between DQ line and 3V3 line
    o.append(line([(760, 135), (760, 220)], C["red"], 3))
    o.append('<rect x="748" y="220" width="24" height="120" rx="3" fill="#fff5da" stroke="#8a5818" stroke-width="2"/>')
    o.append(line([(760, 340), (760, 440)], CW, 3))
    o.append(circle(760, 135, 5, C["red"])); o.append(circle(760, 440, 5, CW))
    o.append(T(735, 290, "4.7kΩ 풀업", 14, "bold", "#8a5818", "end"))
    o.append(T(1060, 300, "방수형 프로브(3선)", 13, "bold", "#333")); o.append(T(1060, 322, "빨강=VDD, 검정=GND", 12, fill="#555"))
    o.append(T(1060, 342, "노랑(또는 흰색)=DQ", 12, fill="#555")); o.append(T(1060, 362, "모듈형은 풀업 내장 여부 확인", 12, fill="#555"))
    yt = pin_table(o, 60, 560, [("GND", "GND", "GND", "왼쪽 다리"), ("DQ", "GPIO13", "D2", "데이터(1-Wire 버스)"), ("VDD", "3V3", "5V", "오른쪽 다리, 3.0~5.5V"),
                                ("4.7kΩ", "DQ–3V3 사이", "DQ–5V 사이", "없으면 'No DS18B20' 출력")])
    return "ex8_onewire_ds18b20_ko.svg", svg(W, yt + 20, o)


def w_spi_boards():
    W, H = 1240, 1130
    o = []
    title(o, "예제 7B·7C · SPI 보드 간 통신 — 마스터가 'A'~'Z'를 보내고, 슬레이브가 동시에 응답",
          "SPI는 '같은 이름끼리' 연결한다(SS-SS, MOSI-MOSI, MISO-MISO, SCK-SCK). UART처럼 TX/RX를 교차하지 않는다. GND 공통 필수.")
    # ① UNO ↔ UNO
    card(o, 40, 110, 1160, 360, "① Arduino UNO(마스터) ↔ Arduino UNO(슬레이브)", CS)
    names = [("D10 SS", "#c62828"), ("D11 MOSI", CS), ("D12 MISO", "#2664bf"), ("D13 SCK", "#555"), ("GND", "#222")]
    lp = board(o, 80, 150, "UNO 마스터 (7B)", [(n, 80 + i * 45) for i, (n, _) in enumerate(names)], 250)
    o.append(rect(870, 150, 250, max(80 + i * 45 for i in range(5)) + 60, C["board"], C["boardS"], 3, 14))
    o.append(T(995, 182, "UNO 슬레이브 (7C)", 19, "bold", "white", "middle"))
    for i, (n, col) in enumerate(names):
        y = 150 + 80 + i * 45
        o.append(f'<rect x="876" y="{y-8}" width="16" height="16" rx="2" fill="#111"/>')
        o.append(T(900, y + 5, n, 15, "bold", "white"))
        o.append(line([lp[n], (876, y)], col, 4)); o.append(dot(876, y, col, 4))
    arrows = [("SS: 마스터 → 슬레이브 (LOW=선택)", 0), ("MOSI: 마스터 → 슬레이브 (보낼 글자)", 1),
              ("MISO: 슬레이브 → 마스터 (응답)", 2), ("SCK: 마스터가 클럭 생성", 3)]
    for t, i in arrows:
        o.append(T(600, 150 + 80 + i * 45 - 8, t, 13, "bold", names[i][1], "middle"))
    o.append(T(600, 445, "각 보드 USB → PC (시리얼 모니터 2개: IDE 창을 2개 열어 COM 포트를 각각 선택)", 14, fill="#333", anchor="middle"))
    # ② ESP32 → UNO
    card(o, 40, 490, 580, 400, "② ESP32(마스터) → UNO(슬레이브): MISO 분압", CS)
    ep = board(o, 70, 545, "ESP32 (7B)", [("GPIO4 SS", 70), ("GPIO23 MOSI", 110), ("GPIO18 SCK", 150), ("GPIO19 MISO", 190), ("GND", 230)], 210)
    ux = 480
    o.append(rect(ux, 545, 120, 290, C["board"], C["boardS"], 3, 12)); o.append(T(ux + 60, 575, "UNO", 17, "bold", "white", "middle"))
    uy = {}
    for i, n in enumerate(["D10", "D11", "D13", "D12", "GND"]):
        y = 545 + 70 + i * 40; uy[n] = y
        o.append(f'<rect x="{ux+4}" y="{y-8}" width="16" height="16" rx="2" fill="#111"/>'); o.append(T(ux + 28, y + 5, n, 14, "bold", "white"))
    for n, u, col in [("GPIO4 SS", "D10", "#c62828"), ("GPIO23 MOSI", "D11", CS), ("GPIO18 SCK", "D13", "#555"), ("GND", "GND", "#222")]:
        o.append(line([ep[n], (ux + 4, uy[u])], col, 3.5))
    ym = uy["D12"]
    o.append(line([(ux + 4, ym), (440, ym)], "#2664bf", 3.5))
    o.append(f'<rect x="395" y="{ym-10}" width="45" height="20" rx="3" fill="#fff5da" stroke="#8a5818" stroke-width="2"/>')
    o.append(T(417, ym - 14, "1kΩ", 12, "bold", "#8a5818", "middle"))
    o.append(line([(395, ym), ep["GPIO19 MISO"]], "#2664bf", 3.5)); o.append(circle(350, ym, 5, "#2664bf"))
    o.append(f'<rect x="342" y="{ym+8}" width="16" height="26" rx="3" fill="#fff5da" stroke="#8a5818" stroke-width="2"/>')
    o.append(line([(350, ym), (350, ym + 8)], "#2664bf", 2.5)); o.append(line([(350, ym + 34), (350, uy["GND"])], "#222", 2.5))
    o.append(circle(350, uy["GND"], 5, "#222")); o.append(T(362, ym + 28, "2kΩ", 12, "bold", "#8a5818"))
    o.append(T(60, 862, "ESP32→UNO 방향(3.3V)은 UNO가 HIGH로 인식(VIH≈3.0V)하지만 여유가 작다.", 12, fill="#555"))
    o.append(T(60, 880, "UNO MISO는 5V → ESP32에 직결 금지. 5V×2k/(1k+2k)≈3.3V로 낮춘다(또는 레벨 시프터).", 13, "bold", "#c62828"))
    # ③ 시프트 레지스터 교환
    card(o, 640, 490, 560, 400, "③ SPI = 두 시프트 레지스터의 맞교환", CS)
    o.append(rect(670, 560, 200, 50, "#fff", CS, 2.5, 6)); o.append(T(770, 592, "마스터 8비트", 15, "bold", CS, "middle"))
    o.append(rect(980, 560, 190, 50, "#fff", "#2664bf", 2.5, 6)); o.append(T(1075, 592, "슬레이브 SPDR", 15, "bold", "#2664bf", "middle"))
    o += arrow(870, 575, 978, 575, CS, 3); o.append(T(924, 568, "MOSI", 12, "bold", CS, "middle"))
    o += arrow(980, 598, 872, 598, "#2664bf", 3); o.append(T(924, 622, "MISO", 12, "bold", "#2664bf", "middle"))
    o.append(T(670, 652, "SCK 8번 → 두 레지스터 내용이 서로 바뀐다", 14, "bold", "#333"))
    rows = [("전송", "MOSI(보냄)", "MISO(받음)"), ("1", "'A'", "'-' (초기값)"), ("2", "'B'", "'a' (A의 응답)"),
            ("3", "'C'", "'b'"), ("…", "…", "…")]
    for i, r in enumerate(rows):
        y = 700 + i * 32
        for j, t in enumerate(r):
            o.append(T(690 + j * 150, y, t, 14, "bold" if i == 0 else "normal", [C["ink"], CS, "#2664bf"][j]))
    o.append(T(670, 868, "응답은 한 박자 늦다: 슬레이브는 받은 '뒤'에야 다음 응답을 준비", 13, fill="#555"))
    # ④ 동작 순서
    card(o, 40, 910, 1160, 200, "④ 코드 흐름", CS)
    left = ["[마스터 7B]", "SPI.beginTransaction(1MHz, 모드0)", "digitalWrite(SS, LOW)   ← 슬레이브 선택",
            "reply = SPI.transfer('A')   ← 보내며 받기", "digitalWrite(SS, HIGH)  ← 선택 해제"]
    right = ["[슬레이브 7C, UNO 레지스터]", "pinMode(MISO, OUTPUT); SPCR |= _BV(SPE);  ← 슬레이브 모드",
             "SPI.attachInterrupt();  ← SPIE=1", "ISR(SPI_STC_vect){ b = SPDR; SPDR = 응답; flag=true; }",
             "loop(): flag면 Serial.print (ISR 안에서 출력 금지)"]
    for i, t in enumerate(left): o.append(mono(70, 965 + i * 34, t, 14, w="bold" if i == 0 else "normal"))
    for i, t in enumerate(right): o.append(mono(620, 965 + i * 34, t, 14, w="bold" if i == 0 else "normal"))
    return "ex7b_spi_board_to_board_ko.svg", svg(W, H, o)

def dht_part(o, x, y, label="DHT11 모듈"):
    o.append(rect(x, y, 110, 130, "#2a7fc4", "#14507e", 2, 8))
    for r in range(4):
        for c in range(5):
            o.append(f'<rect x="{x+12+c*18}" y="{y+14+r*18}" width="12" height="12" rx="2" fill="#bfe0f7"/>')
    o.append(T(x + 55, y + 100, label, 12, "bold", "white", "middle"))
    pos = {}
    for k, lab in enumerate(["S", "+", "−"]):
        px = x + 25 + k * 30
        o.append(T(px, y + 124, lab, 13, "bold", "white", "middle"))
        o.append(line([(px, y + 130), (px, y + 160)], "#999", 4)); o.append(circle(px, y + 164, 6, "#d4af37"))
        pos[lab] = (px, y + 164)
    return pos


def lcd_part(o, x, y):
    o.append(rect(x, y, 300, 110, "#1f5f2a", "#0f3a16", 2, 8))
    o.append(rect(x + 20, y + 20, 260, 70, "#9ed36a", "#4a7a2a", 2, 4))
    o.append(mono(x + 34, y + 50, "Temp: 24°C", 18, "#123", w="bold")); o.append(mono(x + 34, y + 78, "Humi: 55 %", 18, "#123", w="bold"))
    o.append(rect(x + 300, y + 10, 60, 90, "#222", "#000", 2, 4)); o.append(T(x + 330, y + 30, "PCF", 11, "bold", "white", "middle"))
    o.append(T(x + 330, y + 44, "8574", 11, "bold", "white", "middle"))
    pos = {}
    for k, lab in enumerate(["GND", "VCC", "SDA", "SCL"]):
        py = y + 54 + k * 14
        o.append(circle(x + 360, py, 5, "#d4af37")); o.append(T(x + 352, py + 4, lab, 10, "bold", "white", "end")); pos[lab] = (x + 360, py)
    return pos


def w_dht_lcd():
    W, H = 1240, 820
    o = []
    title(o, "예제 6C · DHT11 온습도 → I2C LCD1602 (BH1750이 없을 때의 I2C 실습)",
          "DHT11은 I2C가 아니다(전용 단일선). I2C로 통신하는 장치는 LCD 백팩(PCF8574, 주소 0x27/0x3F) — SDA·SCL 2선으로 글자를 보낸다.")
    bp = board(o, 60, 130, "Arduino UNO", [("5V", 70), ("GND", 120), ("A4 SDA", 180), ("A5 SCL", 230), ("D2", 300)], 250)
    lp = lcd_part(o, 700, 140)
    dp = dht_part(o, 720, 360)
    o.append(line([bp["A4 SDA"], (520, bp["A4 SDA"][1]), (520, lp["SDA"][1] + 0), (640, lp["SDA"][1]), (640, 120), (1100, 120), (1100, lp["SDA"][1]), lp["SDA"]], CI, 3.5))
    o.append(line([bp["A5 SCL"], (540, bp["A5 SCL"][1]), (540, 108), (1120, 108), (1120, lp["SCL"][1]), lp["SCL"]], "#555", 3.5))
    o.append(line([bp["5V"], (480, bp["5V"][1]), (480, 96), (1140, 96), (1140, lp["VCC"][1]), lp["VCC"]], C["red"], 3.5))
    o.append(line([bp["GND"], (500, bp["GND"][1]), (500, 600), (1160, 600), (1160, lp["GND"][1]), lp["GND"]], "#222", 3.5))
    o.append(line([bp["D2"], (600, bp["D2"][1]), (600, 560), (dp["S"][0], 560), dp["S"]], CW, 3.5))
    o.append(line([(480, bp["5V"][1]), (480, 575), (dp["+"][0], 575), dp["+"]], C["red"], 3.5))
    o.append(line([(dp["−"][0], dp["−"][1]), (dp["−"][0], 600)], "#222", 3.5)); o.append(circle(dp["−"][0], 600, 5, "#222"))
    o.append(T(860, 410, "핀 순서(S·+·−)는 모듈마다", 13, "bold", "#c62828"))
    o.append(T(860, 430, "다르다 → 실크인쇄 확인", 13, "bold", "#c62828"))
    o.append(T(860, 460, "4핀 단품: DATA–VCC 10kΩ 풀업", 12, fill="#555"))
    yt = pin_table(o, 60, 670, [("LCD SDA / SCL", "GPIO21 / GPIO22", "A4 / A5", "I2C, 주소 0x27(또는 0x3F)"),
                                ("LCD VCC", "5V(+레벨 시프터) 또는 3.3V LCD", "5V", "5V 백팩 풀업이 SDA/SCL을 5V로 당김"),
                                ("DHT11 S / + / −", "GPIO4 / 3V3 / GND", "D2 / 5V / GND", "전용 단일선(I2C 아님)")])
    return "ex6c_dht11_i2c_lcd_ko.svg", svg(W, yt + 20, o)


def w_i2c_two_uno():
    W, H = 1240, 900
    o = []
    title(o, "예제 6D·6E · I2C 보드 간 통신 — 슬레이브 UNO가 DHT11을 측정, 마스터 UNO가 요청",
          "I2C는 '같은 이름끼리'(SDA-SDA, SCL-SCL) + GND. 마스터가 클럭을 만들고, 슬레이브는 주소 0x08로 응답한다. 추가 I2C 부품이 필요 없다.")
    names = [("A4 SDA", CI), ("A5 SCL", "#555"), ("GND", "#222")]
    lp = board(o, 60, 140, "UNO 마스터 (6D)", [(n, 80 + i * 55) for i, (n, _) in enumerate(names)], 250)
    sx = 830
    o.append(rect(sx, 140, 250, 370, C["board"], C["boardS"], 3, 14)); o.append(T(sx + 125, 172, "UNO 슬레이브 (6E)", 19, "bold", "white", "middle"))
    o.append(T(sx + 125, 196, "Wire.begin(0x08)", 13, fill="#dce9f7", anchor="middle"))
    sp = {}
    for i, (n, col) in enumerate(names):
        y = 140 + 80 + i * 55
        o.append(f'<rect x="{sx+6}" y="{y-8}" width="16" height="16" rx="2" fill="#111"/>'); o.append(T(sx + 30, y + 5, n, 15, "bold", "white"))
        o.append(line([lp[n], (sx + 6, y)], col, 4)); o.append(dot(sx + 6, y, col, 4)); sp[n] = (sx + 6, y)
    for lab, dy in (("D2", 260), ("5V", 300), ("D13 LED", 340)):
        y = 140 + dy
        o.append(f'<rect x="{sx+250-22}" y="{y-8}" width="16" height="16" rx="2" fill="#111"/>'); o.append(T(sx + 250 - 30, y + 5, lab, 14, "bold", "white", "end")); sp[lab] = (sx + 250 - 6, y)
    o.append(circle(sx + 150, 140 + 340, 0, "none"))
    dp = dht_part(o, 1110, 230)
    o.append(line([sp["D2"], (1135, sp["D2"][1]), dp["S"]], CW, 3.5))
    o.append(line([sp["5V"], (1165, sp["5V"][1]), dp["+"]], C["red"], 3.5))
    o.append(line([dp["−"], (dp["−"][0], 440)], "#222", 3.5)); o.append(T(dp["−"][0], 460, "GND", 12, "bold", "#222", "middle"))
    o.append(T(sx + 125, 530, "D13 = 보드 내장 LED (온도 경보 표시)", 13, fill="#333", anchor="middle"))
    o.append(T(570, 210, "SDA: 데이터(양방향)", 13, "bold", CI, "middle"))
    o.append(T(570, 265, "SCL: 마스터가 클럭 생성", 13, "bold", "#555", "middle"))
    o.append(T(570, 380, "UNO Wire가 내부 풀업을 켠다(짧은 배선이면 동작). 길면 4.7kΩ 외부 풀업", 12, fill="#555", anchor="middle"))
    # 시퀀스
    card(o, 40, 540, 1160, 330, "통신 순서 (2초마다)", CI)
    seq = [("① 읽기", "Wire.requestFrom(0x08, 4)", "START · 0x08+R · ACK", "onRequest() → Wire.write(packet, 4)"),
           ("", "Wire.read() ×4", "[상태][온도][습도][체크섬] · STOP", "packet은 loop에서 2초마다 DHT11로 갱신"),
           ("② 쓰기", "beginTransmission(0x08); write(1/0)", "START · 0x08+W · ACK · 명령 · ACK", "onReceive(n) → LED 명령 저장"),
           ("", "endTransmission() == 0 → ACK", "STOP", "loop에서 D13 LED ON/OFF")]
    o.append(T(80, 600, "단계", 14, "bold")); o.append(T(170, 600, "마스터 6D", 14, "bold", CI)); o.append(T(560, 600, "버스", 14, "bold", "#555")); o.append(T(860, 600, "슬레이브 6E", 14, "bold", CI))
    for i, (a, b, c, d) in enumerate(seq):
        y = 645 + i * 46
        o.append(T(80, y, a, 14, "bold")); o.append(mono(170, y, b, 13)); o.append(T(560, y, c, 13, fill="#333")); o.append(T(860, y, d, 13))
    o.append(T(80, 845, "⚠ ESP32를 마스터로 쓰려면 UNO 쪽 5V 풀업 때문에 I2C 양방향 레벨 시프터가 필요하다. 이 실습은 UNO 2대 권장.", 13, "bold", "#c62828"))
    return "ex6d_i2c_two_uno_dht11_ko.svg", svg(W, H, o)


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    for f in (common, compare, physical, uart, i2c, spi, onewire, lambda: w_relay("esp32"), lambda: w_relay("uno"), w_loopback, w_i2c, w_dht_lcd, w_i2c_two_uno, w_spi, w_spi_boards, w_ow):
        n, s = f()
        open(os.path.join(OUT, n), "w", encoding="utf-8").write(s)
        print(n)
