"""Independent, compatible reimplementation inspired by Rahimi et al. (2024).

This is not author code. It uses a finite candidate set and an expected-link
mixed-integer facility-location decomposition, then evaluates selected 3D
placements with the same interference-aware greedy admission rule as ALNS.
"""
import argparse
import csv, math, time
from collections import defaultdict
import numpy as np
from scipy.optimize import Bounds, LinearConstraint, milp
from scipy.sparse import coo_matrix

CAP=20; RMAX=500.0; RMIN=0.5; NOISE=10**(-100/10); PT=10**(33/10); W=20e6
HEIGHTS=(30.0,90.0,150.0)

def pl(d,h):
    d3=math.hypot(d,h);theta=math.degrees(math.atan2(h,d+1e-9));p=1/(1+9.61*math.exp(-.16*(theta-2)))
    return 20*math.log10(d3)+20*math.log10(3.5e9)+20*math.log10(4*math.pi/3e8)+p+(1-p)*20
def signal(q,p,h):
    d=math.hypot(q[0]-p[0],q[1]-p[1]);return 0 if d>RMAX else PT*10**(-pl(d,h)/10)
def link_rate(q,p,h):
    s=signal(q,p,h);return W*math.log2(1+s/NOISE)/1e6 if s else 0

def candidates(q,n):
    xy=[(x,y) for x in (100,300,500,700,900) for y in (100,300,500,700,900)]
    anchors=((250,250),(750,250),(250,750),(750,750))
    blocks=[[] for _ in anchors]
    for p in q:
        j=min(range(4),key=lambda z:(p[0]-anchors[z][0])**2+(p[1]-anchors[z][1])**2);blocks[j].append(p)
    for block in blocks:
        if block:xy.append((sum(x for x,_ in block)/len(block),sum(y for _,y in block)/len(block)))
    # Stable de-duplication at centimetre precision.
    seen=set();out=[]
    for p in xy:
        key=(round(p[0],2),round(p[1],2))
        if key not in seen:seen.add(key);out.append(p)
    return [(p,h) for p in out for h in HEIGHTS],len(out)

def solve(q,n,time_limit=3.0):
    fac,nxy=candidates(q,n);F=len(fac)
    edges=[]
    for i,qi in enumerate(q):
        for f,(p,h) in enumerate(fac):
            r=link_rate(qi,p,h)
            if r>=RMIN:edges.append((i,f,r))
    E=len(edges);nv=F+E+n
    # Rahimi-style first-stage decomposition: minimize uncovered users and
    # then the number of deployed candidate facilities. Link quality is
    # evaluated in the common second-stage evaluator, not blended here.
    c=np.zeros(nv);c[:F]=1.0;c[F+E:]=1000.0
    rr=[];cc=[];dd=[];lb=[];ub=[];row=0
    by_user=defaultdict(list);by_fac=defaultdict(list)
    for e,(i,f,_) in enumerate(edges):by_user[i].append(e);by_fac[f].append(e)
    for i in range(n):
        for e in by_user[i]:rr.append(row);cc.append(F+e);dd.append(1)
        rr.append(row);cc.append(F+E+i);dd.append(1);lb.append(1);ub.append(1);row+=1
    for f in range(F):
        for e in by_fac[f]:rr.append(row);cc.append(F+e);dd.append(1)
        rr.append(row);cc.append(f);dd.append(-CAP);lb.append(-np.inf);ub.append(0);row+=1
    for j in range(nxy):
        for z in range(len(HEIGHTS)):rr.append(row);cc.append(j*len(HEIGHTS)+z);dd.append(1)
        lb.append(-np.inf);ub.append(1);row+=1
    A=coo_matrix((dd,(rr,cc)),shape=(row,nv)).tocsr()
    t=time.perf_counter();res=milp(c,integrality=np.ones(nv),bounds=Bounds(0,1),constraints=LinearConstraint(A,lb,ub),options={"time_limit":time_limit,"mip_rel_gap":.01,"presolve":True});sec=time.perf_counter()-t
    selected=[] if res.x is None else [fac[f] for f in range(F) if res.x[f]>.5]
    return selected,res,sec

def evaluate(q,selected):
    load=[0]*len(selected);rates=[]
    for qi in q:
        sig=[signal(qi,p,h) for p,h in selected];total=sum(sig);best=0;bj=-1
        for j,((p,h),s) in enumerate(zip(selected,sig)):
            if load[j]>=CAP or not s:continue
            r=W*math.log2(1+s/(NOISE+total-s))/1e6
            if r>=RMIN and r>best:best=r;bj=j
        if bj>=0:load[bj]+=1
        rates.append(best)
    rates.sort();served=sum(r>0 for r in rates);total=sum(rates);ss=sum(r*r for r in rates)
    return served/len(q),total,rates[int(.05*(len(q)-1))],total*total/(len(q)*ss) if ss else 0

parser = argparse.ArgumentParser(description="Run the compatible candidate-site MILP benchmark.")
parser.add_argument("--time-limit", type=float, default=3.0, help="MILP time limit per instance in seconds (default: 3).")
parser.add_argument("--max-instances", type=int, default=None, help="Optional number of sorted instances to run for a quick environment check.")
args = parser.parse_args()
if args.time_limit <= 0:
    parser.error("--time-limit must be positive")
if args.max_instances is not None and args.max_instances <= 0:
    parser.error("--max-instances must be positive")

instances=defaultdict(list)
with open("data/candidate_milp/four_cluster_users_n100_n200.csv",newline="") as f:
    for r in csv.DictReader(f):instances[(int(r["N"]),int(r["seed"]))].append((float(r["x"]),float(r["y"])))
selected_instances = sorted(instances.items())
if args.max_instances is not None:
    selected_instances = selected_instances[:args.max_instances]
with open("results/reproduced/candidate_milp_runs.csv","w",newline="") as f:
    w=csv.writer(f);w.writerow(["N","seed","method","status","mip_gap","K","coverage","throughput_Mbps","p5_rate_Mbps","jain_fairness","solve_time_s"])
    for (n,seed),q in selected_instances:
        selected,res,sec=solve(q,n,time_limit=args.time_limit);coverage,total,p5,jain=evaluate(q,selected)
        w.writerow([n,seed,"rahimi_ev_reimplementation",res.status,getattr(res,"mip_gap",float("nan")),len(selected),coverage,total,p5,jain,sec]);print(n,seed,res.status,len(selected),round(coverage,3))
print("saved results/reproduced/candidate_milp_runs.csv")
