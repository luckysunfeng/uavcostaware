"""Summarize the spatial-robustness, ablation, and PSO experiment.

The manuscript's prespecified 48 paired tests are produced separately by
``analysis/paired_statistics.py``. This script only computes descriptive
means, sample standard deviations, and t-based 95% confidence intervals.
"""

import csv
import math
import statistics
from collections import defaultdict


INPUT = "results/reproduced/spatial_robustness_ablation_pso_runs.csv"
OUTPUT = "results/reproduced/spatial_robustness_ablation_pso_summary.csv"
METRICS = (
    "coverage",
    "K",
    "throughput_Mbps",
    "mean_user_rate_Mbps",
    "p5_rate_Mbps",
    "qos_violation",
    "jain_fairness",
    "time_s",
)


with open(INPUT, newline="", encoding="utf-8") as stream:
    rows = list(csv.DictReader(stream))

bad = [
    (index + 2, row)
    for index, row in enumerate(rows)
    if any(row.get(metric) in (None, "") for metric in METRICS)
]
if bad:
    raise ValueError(f"malformed raw CSV rows (first): {bad[0]}")

groups = defaultdict(list)
for row in rows:
    groups[(row["scenario"], row["N"], row["method"])].append(row)

with open(OUTPUT, "w", newline="", encoding="utf-8") as stream:
    writer = csv.writer(stream)
    writer.writerow(
        ["scenario", "N", "method", "metric", "mean", "sd", "ci95_low", "ci95_high"]
    )
    for key, group in sorted(groups.items()):
        for metric in METRICS:
            values = [float(row[metric]) for row in group]
            mean = statistics.mean(values)
            sd = statistics.stdev(values)
            half_width = 2.045 * sd / math.sqrt(len(values))
            writer.writerow(
                [
                    *key,
                    metric,
                    f"{mean:.8g}",
                    f"{sd:.8g}",
                    f"{mean - half_width:.8g}",
                    f"{mean + half_width:.8g}",
                ]
            )

print(f"saved {OUTPUT}")
