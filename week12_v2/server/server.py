"""Python 3.13 stdlib LAN teaching server; optional MySQL via environment."""
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import urlparse, parse_qs
from pathlib import Path
from datetime import datetime, timezone
import json, math, os, re, threading, time, csv
ROOT = Path(__file__).resolve().parent
LOCK = threading.Lock()
LATEST, COMMANDS = {}, {}

def device_id(value):
    if not isinstance(value, str) or not re.fullmatch(r"[A-Za-z0-9_-]{1,32}", value):
        raise ValueError("invalid device")
    return value

def save_reading(data):
    device = device_id(data.get("device"))
    t, h = data.get("temperature"), data.get("humidity")
    if any(isinstance(v, bool) or not isinstance(v, (int, float)) or not math.isfinite(v) for v in (t, h)):
        raise ValueError("finite numeric values required")
    if not -40 <= t <= 80 or not 0 <= h <= 100:
        raise ValueError("out of range")
    now = datetime.now(timezone.utc).isoformat()
    row = dict(device=device, temperature=t, humidity=h, received_at=now)
    # Optional persistence must succeed before reporting success.
    if os.getenv("MYSQL_ENABLED") == "1":
        import mysql.connector
        conn = mysql.connector.connect(host=os.getenv("MYSQL_HOST", "127.0.0.1"),
            user=os.getenv("MYSQL_USER", "root"), password=os.environ["MYSQL_PASSWORD"],
            database="embedded12", connection_timeout=3)
        try:
            cur = conn.cursor()
            cur.execute("INSERT INTO readings(device,temperature,humidity) VALUES (%s,%s,%s)", (device,t,h))
            conn.commit()
            cur.close()
        finally:
            conn.close()
    with LOCK:
        path = ROOT / "received.csv"
        new = not path.exists()
        with path.open("a", newline="", encoding="utf-8") as f:
            writer = csv.DictWriter(f, fieldnames=list(row))
            if new: writer.writeheader()
            writer.writerow(row)
        LATEST[device] = (row, time.time())
    return row

class Handler(BaseHTTPRequestHandler):
    def reply(self, code, data, mime="application/json"):
        body = data.encode("utf-8") if isinstance(data, str) else json.dumps(data, ensure_ascii=False).encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", mime + "; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        u = urlparse(self.path)
        if u.path == "/":
            return self.reply(200, (ROOT / "dashboard.html").read_text(encoding="utf-8"), "text/html")
        if u.path not in ("/api/latest", "/api/led"):
            return self.reply(404, {"error":"not found"})
        try:
            device = device_id(parse_qs(u.query).get("device", ["team01"])[0])
        except ValueError as e:
            return self.reply(400, {"error":str(e)})
        with LOCK:
            if u.path == "/api/led":
                return self.reply(200, str(COMMANDS.get(device, 0)), "text/plain")
            row, stamp = LATEST.get(device, ({}, time.time()))
            result = dict(row, age_seconds=round(time.time()-stamp,1))
        self.reply(200, result)

    def do_POST(self):
        try:
            length = int(self.headers.get("Content-Length", "0"))
            if not 0 < length <= 2048:
                return self.reply(413, {"error":"body size"})
            data = json.loads(self.rfile.read(length))
            if not isinstance(data, dict): raise ValueError("object required")
            path = urlparse(self.path).path
            if path == "/api/readings":
                return self.reply(201, save_reading(data))
            if path == "/api/led":
                device = device_id(data.get("device"))
                state = data.get("state")
                if type(state) is not int or state not in (0,1): raise ValueError("state must be 0 or 1")
                with LOCK: COMMANDS[device] = state
                return self.reply(200, {"accepted":state})
            self.reply(404, {"error":"not found"})
        except (ValueError, UnicodeError) as e:
            self.reply(400, {"error":str(e)})
        except Exception:
            self.reply(503, {"error":"storage unavailable; inspect server configuration"})

if __name__ == "__main__":
    print("LAN teaching server: http://<PC IPv4>:8000 (Ctrl+C stops)")
    ThreadingHTTPServer(("0.0.0.0", 8000), Handler).serve_forever()
