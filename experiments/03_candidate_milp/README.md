# Experiment 3: literature-derived candidate-MILP comparison

This directory contains an independent compatible implementation inspired by
the expected-value/decomposition idea of Rahimi et al. It is not the
authors' CPLEX code and is not claimed to reproduce their full stochastic
model.

## Protocol

- Canonical paired four-cluster instances: 30 seeds at 100 users and 30 seeds
  at 200 users
- Horizontal candidates: 5x5 regular grid plus four data-driven cluster
  centroids
- Heights: 30, 90, and 150 m
- Capacity: 20 users per UAV
- Solver: `scipy.optimize.milp`
- Time limit: 3 seconds per instance
- Relative MIP gap target: 0.01
- All time-limit incumbents are retained and solver status/gap are reported

The chosen candidate facilities are evaluated with the same
interference-aware greedy admission rule used by the LNS experiments.

## Run

From the repository root:

```bash
python experiments/03_candidate_milp/run_candidate_milp.py
python analysis/summarize_candidate_milp.py
```

Outputs:

```text
results/reproduced/candidate_milp_runs.csv
results/reproduced/candidate_milp_summary.csv
```

For a quick dependency and path check without changing the default
manuscript protocol:

```bash
python experiments/03_candidate_milp/run_candidate_milp.py \
  --max-instances 1 --time-limit 1
```

`export_candidate_instances.c` documents how the canonical coordinates were
generated. It writes a separate audit copy to
`results/reproduced/generated_candidate_instances.csv` and does not overwrite
the committed canonical data. Because C-library pseudo-random sequences can
vary by platform, the stored canonical CSV is the input used for comparison.
