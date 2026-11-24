#!/usr/bin/env python3
"""Amoeba STB Monitor API. Python standard library only."""
import json, os, re, shutil, socket, subprocess, time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

HOST=os.getenv("AMOEBA_API_HOST","0.0.0.0")
PORT=int(os.getenv("AMOEBA_API_PORT","8765"))
STORAGE_PATH=os.getenv("AMOEBA_STORAGE_PATH","/mnt/amoeba")
INTERFACES=os.getenv("AMOEBA_INTERFACES","eth0,eth1").split(",")
API_TOKEN=os.getenv("AMOEBA_API_TOKEN","").strip()
_prev_cpu=None
_prev_net={}
_prev_time=None

def read_cpu():
    with open("/proc/stat",encoding="ascii") as f: v=[int(x) for x in f.readline().split()[1:]]
    return sum(v),v[3]+(v[4] if len(v)>4 else 0)

def cpu_percent():
    global _prev_cpu
    cur=read_cpu(); out=0.0
    if _prev_cpu:
        total=cur[0]-_prev_cpu[0]; idle=cur[1]-_prev_cpu[1]
        if total>0: out=max(0.0,min(100.0,(total-idle)*100.0/total))
    _prev_cpu=cur
    return round(out,1)

def memory_info():
    d={}
    with open("/proc/meminfo",encoding="ascii") as f:
        for line in f:
            m=re.match(r"^(MemTotal|MemAvailable):\s+(\d+)",line)
            if m: d[m.group(1)]=int(m.group(2))*1024
    total=d.get("MemTotal",0); used=max(0,total-d.get("MemAvailable",0))
    return {"total_mb":round(total/1048576),"used_mb":round(used/1048576),"percent":round(used*100/total,1) if total else 0}

def temperature():
    vals=[]
    for root,_,names in os.walk("/sys/class/thermal"):
        if "temp" in names:
            try:
                raw=int(open(os.path.join(root,"temp"),encoding="ascii").read().strip())
                if 1000<=raw<=150000: vals.append(raw/1000)
            except (OSError,ValueError): pass
    return round(max(vals),1) if vals else None

def network_info():
    global _prev_net,_prev_time
    now=time.monotonic(); elapsed=now-_prev_time if _prev_time else 0
    result={}; current={}
    for iface in INTERFACES:
        iface=iface.strip()
        if not iface: continue
        base=f"/sys/class/net/{iface}/statistics"
        try:
            rx=int(open(base+"/rx_bytes",encoding="ascii").read())
            tx=int(open(base+"/tx_bytes",encoding="ascii").read())
            current[iface]=(rx,tx); old=_prev_net.get(iface,(rx,tx))
            state=open(f"/sys/class/net/{iface}/operstate",encoding="ascii").read().strip()
            result[iface]={"state":state,"rx_mbps":round(max(0,rx-old[0])*8/elapsed/1e6,2) if elapsed>0 else 0,
                           "tx_mbps":round(max(0,tx-old[1])*8/elapsed/1e6,2) if elapsed>0 else 0}
        except (OSError,ValueError): result[iface]={"state":"missing","rx_mbps":0,"tx_mbps":0}
    _prev_net=current; _prev_time=now
    return result

def service_state(name):
    try:
        p=subprocess.run(["systemctl","is-active",name],capture_output=True,text=True,timeout=1,check=False)
        return p.stdout.strip() or "unknown"
    except (OSError,subprocess.TimeoutExpired): return "unknown"

def snapshot():
    disk=shutil.disk_usage(STORAGE_PATH) if os.path.exists(STORAGE_PATH) else None
    try: uptime=int(float(open("/proc/uptime",encoding="ascii").read().split()[0]))
    except (OSError,ValueError): uptime=0
    try:
        s=socket.socket(socket.AF_INET,socket.SOCK_DGRAM); s.connect(("1.1.1.1",80))
        try: ip=s.getsockname()[0]
        finally: s.close()
    except OSError: ip="unavailable"
    return {"device":"ZTE B860H V1","hostname":socket.gethostname(),"ip":ip,"uptime_seconds":uptime,
      "cpu_percent":cpu_percent(),"temperature_c":temperature(),"memory":memory_info(),
      "storage":{"path":STORAGE_PATH,"online":disk is not None,
        "total_gb":round(disk.total/1e9,1) if disk else 0,"used_gb":round((disk.total-disk.free)/1e9,1) if disk else 0,
        "free_gb":round(disk.free/1e9,1) if disk else 0,
        "percent":round((disk.total-disk.free)*100/disk.total,1) if disk and disk.total else 0},
      "network":network_info(),"services":{"ssh":service_state("ssh"),"network_manager":service_state("NetworkManager")},
      "timestamp":int(time.time())}

class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path not in ("/","/health","/api/v1/status"): self.send_error(404); return
        if API_TOKEN and self.headers.get("Authorization")!="Bearer "+API_TOKEN: self.send_error(401); return
        body=json.dumps(snapshot(),separators=(",",":")).encode()
        self.send_response(200); self.send_header("Content-Type","application/json")
        self.send_header("Content-Length",str(len(body))); self.send_header("Cache-Control","no-store")
        self.end_headers(); self.wfile.write(body)
    def log_message(self,fmt,*args): print("%s - %s"%(self.address_string(),fmt%args),flush=True)

if __name__=="__main__":
    print(f"Amoeba Monitor API listening on {HOST}:{PORT}",flush=True)
    ThreadingHTTPServer((HOST,PORT),Handler).serve_forever()
