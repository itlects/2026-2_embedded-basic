# -*- coding: utf-8 -*-
"""9주차 그림 공통 도구 (week08/images/gen_figs.py 스타일을 따른다)."""

FONT = "'Noto Sans KR','Noto Sans CJK KR','Malgun Gothic',sans-serif"
C = dict(red="#d43b3b", green="#208657", blue="#2664bf", orange="#e07b16",
         purple="#7b4bb3", black="#222", gnd="#222", board="#2f6aa8",
         boardS="#1d3f66", bb="#f4f1e8", bbS="#cfc6ad", hole="#6b6658",
         gap="#e3dccb", yellow="#f2c230", rail_neg="#2a4fa0", rail_pos="#c63a3a",
         v33="#d43b3b", teal="#178a8a", gray="#8a8f98", ink="#1d2433", sub="#444")


def esc(s):
    return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def svg(w, h, body):
    return (f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" '
            f'viewBox="0 0 {w} {h}" font-family="{FONT}"><rect width="{w}" height="{h}" fill="white"/>'
            + "".join(body) + "</svg>")


def T(x, y, s, size=18, w="normal", fill="#222", anchor="start", italic=False):
    st = ' font-style="italic"' if italic else ""
    return (f'<text x="{x}" y="{y}" font-size="{size}" font-weight="{w}" fill="{fill}" '
            f'text-anchor="{anchor}"{st}>{esc(s)}</text>')


def line(pts, color, w=4, dash=None, cap="round"):
    d = "M" + " L".join(f"{x:.1f} {y:.1f}" for x, y in pts)
    da = f' stroke-dasharray="{dash}"' if dash else ""
    return (f'<path d="{d}" stroke="{color}" stroke-width="{w}" fill="none" '
            f'stroke-linejoin="round" stroke-linecap="{cap}"{da}/>')


def rect(x, y, w, h, fill="#fafafa", stroke="#c9ced6", sw=2, rx=10, extra=""):
    return f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}"{extra}/>'


def circle(x, y, r, fill, stroke="none", sw=0):
    return f'<circle cx="{x}" cy="{y}" r="{r}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}"/>'


def dot(x, y, color, r=5):
    return f'<circle cx="{x}" cy="{y}" r="{r}" fill="{color}" stroke="white" stroke-width="1.5"/>'


def title(o, t, sub=None):
    o.append(T(40, 50, t, 30, "bold", C["ink"]))
    if sub:
        o.append(T(40, 82, sub, 17, fill=C["sub"]))


def card(o, x, y, w, h, head, color="#1d2433"):
    o.append(rect(x, y, w, h))
    o.append(T(x + 20, y + 34, head, 20, "bold", color))


def pill(x, y, s, color, size=14, fill="white"):
    w = sum(1.0 if ord(ch) > 0x2000 else 0.6 for ch in s) * size + 18
    return [f'<rect x="{x-w/2:.1f}" y="{y-size+1}" width="{w:.1f}" height="{size+8}" rx="{(size+8)/2}" fill="{fill}" stroke="{color}" stroke-width="1.5"/>',
            T(x, y + 2, s, size, "bold", color, "middle")]


def arrow(x1, y1, x2, y2, color="#3a4556", w=3):
    import math
    a = math.atan2(y2 - y1, x2 - x1)
    L = 12
    p1 = (x2 - L * math.cos(a - 0.4), y2 - L * math.sin(a - 0.4))
    p2 = (x2 - L * math.cos(a + 0.4), y2 - L * math.sin(a + 0.4))
    return [line([(x1, y1), (x2, y2)], color, w),
            f'<path d="M{x2:.1f} {y2:.1f} L{p1[0]:.1f} {p1[1]:.1f} L{p2[0]:.1f} {p2[1]:.1f} Z" fill="{color}"/>']


# ---------------------------------------------------------------- 회로 기호
def sym_resistor_h(x1, x2, y, label=None, color="#8a5818"):
    """가로 저항(상자형). x1~x2 사이."""
    mx = (x1 + x2) / 2
    o = [f'<rect x="{mx-45}" y="{y-12}" width="90" height="24" rx="4" fill="#fff5da" stroke="{color}" stroke-width="2"/>']
    if label:
        o.append(T(mx, y + 6, label, 14, anchor="middle"))
    return o


def sym_resistor_v(x, y1, y2, label=None):
    my = (y1 + y2) / 2
    o = [line([(x, y1), (x, my - 34)], "#333", 3), line([(x, my + 34), (x, y2)], "#333", 3),
         f'<rect x="{x-11}" y="{my-34}" width="22" height="68" rx="3" fill="#fff5da" stroke="#8a5818" stroke-width="2"/>']
    if label:
        o.append(T(x + 18, my + 5, label, 14, fill="#333"))
    return o


def sym_led_h(x, y, color, label_pm=True):
    """왼쪽 = 애노드(+), 오른쪽 = 캐소드(−). 폭 32."""
    o = [f'<path d="M{x} {y-16} L{x} {y+16} L{x+32} {y} Z" fill="{color}" stroke="#222" stroke-width="2"/>',
         line([(x + 32, y - 16), (x + 32, y + 16)], "#222", 3),
         line([(x + 12, y - 22), (x + 24, y - 34)], "#222", 2), line([(x + 22, y - 20), (x + 34, y - 32)], "#222", 2)]
    if label_pm:
        o.append(T(x - 8, y - 18, "+", 15, "bold", "#c62828", "end"))
        o.append(T(x + 44, y - 16, "−", 16, "bold", "#333"))
    return o


def sym_gnd(x, y):
    return [line([(x, y), (x, y + 10)], "#222", 3), line([(x - 14, y + 10), (x + 14, y + 10)], "#222", 3),
            line([(x - 9, y + 16), (x + 9, y + 16)], "#222", 3), line([(x - 4, y + 22), (x + 4, y + 22)], "#222", 3)]


def sym_switch_h(x, y, color="#222"):
    """x: 왼쪽 접점, 폭 60"""
    return [circle(x, y, 5, color), line([(x, y), (x + 44, y - 20)], "#222", 3), circle(x + 60, y, 5, "#222")]


def sym_switch_closed(x, y):
    return [circle(x, y, 5, "#222"), line([(x, y), (x + 60, y - 3)], "#222", 3), circle(x + 60, y, 5, "#222")]


def sym_buzzer(x, y, r=26):
    return [circle(x, y, r, "#2b2b2b", "#000", 2), circle(x, y, 5, "#555"),
            T(x - r * 0.62, y - 3, "+", 15, "bold", "#ff6b6b", "middle")]
