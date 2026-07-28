# Experiment 2: spatial robustness, ablations, PSO, and sensitivity

This program contains the paired experiments behind the manuscript's spatial
robustness, component-ablation, controlled-fleet PSO, and parameter
sensitivity sections.

## Spatial experiment

- Layouts: four-cluster, uniform, single-hotspot, and corridor
- User counts: 100, 200, 300, and 400
- Runs: 30 paired seeds for every layout and size
- Seed formula: `10000 + 1000 * layout_index + 100 * size_index + run`
- Users are sorted by horizontal coordinate before block construction
- Methods:
  - `kmeans_grid`: fixed-fleet k-means with altitude-grid search
  - `gno`: block-centroid initializer (historical internal label)
  - `alns_no_refine`: LNS without continuous refinement
  - `fixed_gno_refine`: fixed-initializer refinement
  - `alns_sca_lite`: complete proposed LNS (historical internal label)
  - controlled-fleet PSO with 3x4, 6x8, and 12x16 budgets

Each PSO run uses the active fleet size returned by the complete LNS on the
same instance. It is therefore a placement-only controlled comparison, not a
fleet-selection baseline.

## Sensitivity experiment

- Layout: four-cluster
- Users: 400
- Common seeds: 20000--20029
- One-factor-at-a-time factors:
  - link threshold: 0.5, 2, and 5 Mbit/s
  - capacity: 10, 20, and 30 users per UAV
  - fleet-cost weight: 25, 100, and 400
  - maximum altitude: 80, 150, and 250 m
  - NLoS excess loss: 10, 20, and 30 dB

## Build and run

```bash
gcc -O2 -std=c11 \
  experiments/02_spatial_robustness_ablation_pso/run_spatial_robustness_ablation_pso.c \
  -o build/run_spatial_experiments -lm
./build/run_spatial_experiments
```

To run only the sensitivity sweep:

```bash
./build/run_spatial_experiments --sensitivity-only
```

Generated files:

```text
results/reproduced/spatial_robustness_ablation_pso_runs.csv
results/reproduced/parameter_sensitivity_runs.csv
```

Run the scripts in `analysis/` to create descriptive summaries, the
prespecified paired tests, and hover-energy indicators.
