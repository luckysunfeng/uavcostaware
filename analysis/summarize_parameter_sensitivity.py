import csv, math, statistics
from collections import defaultdict

with open("results/reproduced/parameter_sensitivity_runs.csv", newline="") as f:
    rows=list(csv.DictReader(f))
groups=defaultdict(list)
for r in rows: groups[(r["factor"],r["level"])].append(r)
metrics=["coverage","K","throughput_Mbps","p5_rate_Mbps","jain_fairness","time_s"]
with open("results/reproduced/parameter_sensitivity_summary.csv","w",newline="") as f:
    w=csv.writer(f);w.writerow(["factor","level","metric","mean","sd","ci95_low","ci95_high"])
    for key,rs in sorted(groups.items()):
        for m in metrics:
            x=[float(r[m]) for r in rs];mu=statistics.mean(x);sd=statistics.stdev(x);d=2.045*sd/math.sqrt(len(x))
            w.writerow([*key,m,f"{mu:.8g}",f"{sd:.8g}",f"{mu-d:.8g}",f"{mu+d:.8g}"])
print("saved results/reproduced/parameter_sensitivity_summary.csv")
