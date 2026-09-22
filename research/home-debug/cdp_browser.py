import sys,json,urllib.request,base64
from pathlib import Path
sys.path.insert(0,'/nix/store/f8a9ha154z9c8s0r0rhhl1nsyfr7pai7-python3.14-websocket-client-1.9.0/lib/python3.14/site-packages')
import websocket
pages=json.load(urllib.request.urlopen('http://127.0.0.1:9222/json'))
w=websocket.create_connection(json.load(urllib.request.urlopen('http://127.0.0.1:9222/json/version'))['webSocketDebuggerUrl'],timeout=12,suppress_origin=True)
i=0
def call(method,params={}):
 global i
 i+=1; w.send(json.dumps(dict(id=i,method=method,params=params)))
 while True:
  r=json.loads(w.recv())
  if r.get('id')==i:return r
r=call(sys.argv[1],json.loads(sys.argv[2]) if len(sys.argv)>2 else {});print(json.dumps(r,indent=2))
w.close()
