import sys,json,urllib.request,time
from pathlib import Path
sys.path.insert(0,'/nix/store/f8a9ha154z9c8s0r0rhhl1nsyfr7pai7-python3.14-websocket-client-1.9.0/lib/python3.14/site-packages')
import websocket
w=websocket.create_connection(json.load(urllib.request.urlopen('http://127.0.0.1:9222/json/version'))['webSocketDebuggerUrl'],timeout=15,suppress_origin=True)
w.send(json.dumps({'id':1,'method':'Tracing.start','params':{'categories':'viz,cc,gpu,disabled-by-default-viz.debug','transferMode':'ReportEvents'}}))
print(w.recv())
p=json.load(urllib.request.urlopen('http://127.0.0.1:9222/json'))[0]
v=websocket.create_connection(p['webSocketDebuggerUrl'],timeout=5,suppress_origin=True)
v.send(json.dumps({'id':1,'method':'Runtime.evaluate','params':{'expression':'window.probeTimer=setInterval(()=>document.body.style.opacity=document.body.style.opacity=="1"?"0.99":"1",100);true'}}));v.recv()
time.sleep(3)
v.send(json.dumps({'id':2,'method':'Runtime.evaluate','params':{'expression':'clearInterval(window.probeTimer);document.body.style.opacity="1"'}}));v.recv();v.close()
w.send(json.dumps({'id':2,'method':'Tracing.end'}));events=[]
while True:
 r=json.loads(w.recv())
 if r.get('method')=='Tracing.dataCollected':events.extend(r['params']['value'])
 if r.get('method')=='Tracing.tracingComplete':break
Path('logs/home-debug/frame-trace.json').write_text(json.dumps(events));print('events',len(events))
from collections import Counter
print(Counter(e['name'] for e in events if any(t in e['name'].lower() for t in ['swap','draw','present','occlu'])))
