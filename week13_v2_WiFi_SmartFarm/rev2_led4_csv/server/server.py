"""Classroom LAN telemetry server, Python 3.11+; no pip dependencies."""
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, urlparse
from datetime import datetime, timezone
import csv, json, math, re, threading
ROOT=Path(__file__).resolve().parent
DATA=ROOT/"data.csv"
FIELDS=["timestamp","device","temperature","humidity","soil","leds","buttons","pump","mode"]
LOCK=threading.Lock()
class Handler(BaseHTTPRequestHandler):
    def reply(self, status, data, kind="application/json; charset=utf-8"):
        self.send_response(status)
        self.send_header("Content-Type",kind)
        self.send_header("Content-Length",str(len(data)))
        self.send_header("Cache-Control","no-store")
        self.end_headers()
        self.wfile.write(data)
    def do_GET(self):
        path=urlparse(self.path).path
        if path=="/":
            self.reply(200,(ROOT/"index.html").read_bytes(),"text/html; charset=utf-8")
        elif path=="/api/history":
            with LOCK:
                if DATA.exists():
                    with DATA.open(encoding="utf-8",newline="") as f:
                        from collections import deque
                        rows=list(deque(csv.DictReader(f),maxlen=100))
                else: rows=[]
            self.reply(200,json.dumps(rows).encode())
        elif path=="/data.csv":
            with LOCK:
                data=DATA.read_bytes() if DATA.exists() else (",".join(FIELDS)+"\n").encode()
            self.reply(200,data,"text/csv; charset=utf-8")
        else:self.reply(404,b'{"error":"not found"}')
    def do_POST(self):
        if urlparse(self.path).path!="/api/sample":
            self.reply(404,b'{"error":"not found"}');return
        try:
            n=int(self.headers.get("Content-Length","0"))
            if not 1<=n<=1024:raise ValueError()
            d=parse_qs(self.rfile.read(n).decode("utf-8"),strict_parsing=True)
            def val(k):return d[k][0]
            device=val("device")
            if not re.fullmatch(r"[A-Za-z0-9_-]{1,24}",device):raise ValueError()
            t,h=float(val("temperature")),float(val("humidity"))
            ints=[int(val(k)) for k in FIELDS[4:]]
            soil,leds,buttons,pump,mode=ints
            if not(math.isfinite(t) and -40<=t<=80 and math.isfinite(h) and 0<=h<=100 and -1<=soil<=100 and 0<=leds<=15 and 0<=buttons<=15 and pump in (0,1) and mode in (0,1)):raise ValueError()
        except (ValueError,KeyError,UnicodeError,IndexError):
            self.reply(400,b'{"error":"invalid sample"}');return
        with LOCK:
            new=not DATA.exists() or DATA.stat().st_size==0
            with DATA.open("a",encoding="utf-8",newline="") as f:
                w=csv.writer(f)
                if new:w.writerow(FIELDS)
                w.writerow([datetime.now(timezone.utc).isoformat(timespec="seconds"),device,t,h,*ints])
        self.reply(201,b'{"ok":true}')
if __name__=="__main__":
    print("Open http://localhost:8000 or http://PC_IPV4:8000")
    ThreadingHTTPServer(("0.0.0.0",8000),Handler).serve_forever()
