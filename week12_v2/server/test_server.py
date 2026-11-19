import unittest, threading, json, urllib.request, urllib.error, tempfile
from pathlib import Path
import server
class APITest(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.tmp=tempfile.TemporaryDirectory()
  cls.original=server.ROOT
  server.ROOT=Path(cls.tmp.name)
  (server.ROOT/'dashboard.html').write_text('<h1>test</h1>')
  cls.http=server.ThreadingHTTPServer(('127.0.0.1',0),server.Handler)
  cls.base='http://127.0.0.1:'+str(cls.http.server_port)
  threading.Thread(target=cls.http.serve_forever,daemon=True).start()
 @classmethod
 def tearDownClass(cls):
  cls.http.shutdown();cls.http.server_close();server.ROOT=cls.original;cls.tmp.cleanup()
 def request(self,path,data=None):
  req=urllib.request.Request(self.base+path,data=json.dumps(data).encode() if data is not None else None,headers={'Content-Type':'application/json'})
  try:
   with urllib.request.urlopen(req,timeout=3) as r:return r.status,r.read().decode()
  except urllib.error.HTTPError as e:return e.code,e.read().decode()
 def test_valid_reading_csv(self):
  self.assertEqual(self.request('/api/readings',dict(device='team01',temperature=24,humidity=55))[0],201)
  code,body=self.request('/api/latest?device=team01')
  self.assertEqual(json.loads(body)['temperature'],24)
  self.assertIn('team01',(server.ROOT/'received.csv').read_text())
 def test_led(self):
  self.assertEqual(self.request('/api/led',dict(device='team02',state=1))[0],200)
  self.assertEqual(self.request('/api/led?device=team02')[1],'1')
  self.assertEqual(self.request('/api/led?device=other')[1],'0')
 def test_bad_values(self):
  for t,h in [(True,50),(float('nan'),50),(20,101),('20',50)]:
   self.assertEqual(self.request('/api/readings',dict(device='team01',temperature=t,humidity=h))[0],400)
  self.assertEqual(self.request('/api/led',dict(device='../x',state=1))[0],400)
  self.assertEqual(self.request('/api/led',dict(device='team01',state=True))[0],400)
 def test_routes(self):
  self.assertEqual(self.request('/')[0],200)
  self.assertEqual(self.request('/missing')[0],404)
  self.assertNotIn('temperature',json.loads(self.request('/api/latest?device=new')[1]))
 def test_storage_failure(self):
  old=server.ROOT;server.ROOT=old/'missing'
  try:self.assertEqual(self.request('/api/readings',dict(device='team01',temperature=20,humidity=40))[0],503)
  finally:server.ROOT=old
if __name__=='__main__':unittest.main(verbosity=2)
