import sys,json,urllib.request,base64
from pathlib import Path
sys.path.insert(0,'/nix/store/f8a9ha154z9c8s0r0rhhl1nsyfr7pai7-python3.14-websocket-client-1.9.0/lib/python3.14/site-packages')
import websocket
pages=json.load(urllib.request.urlopen('http://127.0.0.1:9222/json'))
w=websocket.create_connection(pages[0]['webSocketDebuggerUrl'],timeout=12,suppress_origin=True)
i=0
def call(method,params={}):
 global i
 i+=1; w.send(json.dumps(dict(id=i,method=method,params=params)))
 while True:
  r=json.loads(w.recv())
  if r.get('id')==i:return r
expr=sys.argv[1] if len(sys.argv)>1 else 'JSON.stringify({url:location.href,state:document.readyState,text:document.body.innerText,html:document.body.innerHTML.slice(0,2500),width:innerWidth,height:innerHeight,visibility:document.visibilityState})'
r=call('Runtime.evaluate',dict(expression=expr,returnByValue=True));print(json.dumps(r,indent=2))
if len(sys.argv)==1:
 r=call('Page.captureScreenshot',{'format':'png'})
 if 'data' in r.get('result',{}):Path('logs/home-debug/browser.png').write_bytes(base64.b64decode(r['result']['data']))
w.close()
