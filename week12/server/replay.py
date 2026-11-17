"""센서 없이 PC 서버 수신을 검증하는 사전 제공 데이터 재생."""
import csv, json, time, urllib.request
from pathlib import Path
for row in csv.DictReader((Path(__file__).resolve().parent.parent / "data/sample_readings.csv").open()):
    row["temperature"] = float(row["temperature"])
    row["humidity"] = float(row["humidity"])
    req = urllib.request.Request("http://127.0.0.1:8000/api/readings",
        data=json.dumps(row).encode(), headers={"Content-Type":"application/json"})
    with urllib.request.urlopen(req, timeout=3) as response:
        print(response.status, response.read().decode())
    time.sleep(3)
