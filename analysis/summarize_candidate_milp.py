import csv,math,statistics
from collections import defaultdict
with open("results/reproduced/candidate_milp_runs.csv",newline="") as f: rows=list(csv.DictReader(f))
g=defaultdict(list)
for r in rows:g[int(r["N"])].append(r)
metrics=("K","coverage","throughput_Mbps","p5_rate_Mbps","jain_fairness","solve_time_s")
with open("results/reproduced/candidate_milp_summary.csv","w",newline="") as f:
 w=csv.writer(f);w.writerow(["N","metric","mean","sd","ci95_low","ci95_high"])
 for n,rs in sorted(g.items()):
  w.writerow([n,"optimal_status_fraction",sum(r["status"]=="0" for r in rs)/len(rs),0,"",""])
  w.writerow([n,"mean_reported_mip_gap",statistics.mean(float(r["mip_gap"]) for r in rs),statistics.stdev(float(r["mip_gap"]) for r in rs),"",""])
  for m in metrics:
   x=[float(r[m]) for r in rs];mu=statistics.mean(x);sd=statistics.stdev(x);d=2.045*sd/math.sqrt(len(x));w.writerow([n,m,mu,sd,mu-d,mu+d])
print("saved results/reproduced/candidate_milp_summary.csv")
