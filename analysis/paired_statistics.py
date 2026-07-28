import csv
from collections import defaultdict
import numpy as np
from scipy.stats import wilcoxon


INPUT = "results/reproduced/spatial_robustness_ablation_pso_runs.csv"
OUTPUT = "results/reproduced/paired_statistics.csv"
SCENARIOS = ("four_cluster", "uniform", "hotspot", "corridor")
COMPARATORS = ("gno", "alns_no_refine", "fixed_gno_refine", "pso_high_12x16")
METRICS = ("coverage", "K", "p5_rate_Mbps")


rows = defaultdict(dict)
with open(INPUT, newline="", encoding="utf-8") as f:
    for row in csv.DictReader(f):
        if int(row["N"]) != 400 or row["scenario"] not in SCENARIOS:
            continue
        key = (row["scenario"], int(row["seed"]))
        rows[key][row["method"]] = row


tests = []
rng = np.random.default_rng(20260718)
for scenario in SCENARIOS:
    paired = [v for (s, _), v in rows.items() if s == scenario]
    for comparator in COMPARATORS:
        for metric in METRICS:
            pairs = [
                (float(v["alns_sca_lite"][metric]), float(v[comparator][metric]))
                for v in paired
                if "alns_sca_lite" in v and comparator in v
            ]
            a = np.array([p[0] for p in pairs])
            b = np.array([p[1] for p in pairs])
            d = a - b
            if np.allclose(d, 0):
                statistic, p_value = 0.0, 1.0
            else:
                result = wilcoxon(d, alternative="two-sided", zero_method="wilcox", method="auto")
                statistic, p_value = float(result.statistic), float(result.pvalue)
            indices = rng.integers(0, len(d), size=(20000, len(d)))
            boot = d[indices].mean(axis=1)
            lo, hi = np.quantile(boot, [0.025, 0.975])
            signs = rng.choice((-1.0, 1.0), size=(50000, len(d)))
            permuted = np.abs((signs * d).mean(axis=1))
            permutation_p = (np.count_nonzero(permuted >= abs(np.mean(d))) + 1) / 50001
            tests.append({
                "scenario": scenario,
                "N": 400,
                "metric": metric,
                "reference": "lns_based",
                "comparator": comparator.replace("alns_no_refine", "lns_no_refine")
                    .replace("fixed_gno_refine", "fixed_initializer_refine")
                    .replace("gno", "block_centroid"),
                "n_pairs": len(d),
                "mean_paired_difference": float(np.mean(d)),
                "bootstrap_ci95_low": float(lo),
                "bootstrap_ci95_high": float(hi),
                "wilcoxon_W": statistic,
                "p_two_sided": p_value,
                "p_signflip": float(permutation_p),
            })

def add_holm(source, target):
    order = sorted(range(len(tests)), key=lambda i: tests[i][source])
    running = 0.0
    m = len(tests)
    for rank, idx in enumerate(order):
        adjusted = min(1.0, (m - rank) * tests[idx][source])
        running = max(running, adjusted)
        tests[idx][target] = running


# Holm adjustment across the prespecified 48-test family.
add_holm("p_two_sided", "p_holm")
add_holm("p_signflip", "p_signflip_holm")

with open(OUTPUT, "w", newline="", encoding="utf-8") as f:
    writer = csv.DictWriter(f, fieldnames=list(tests[0]))
    writer.writeheader()
    writer.writerows(tests)

print(f"wrote {len(tests)} paired comparisons to {OUTPUT}")
