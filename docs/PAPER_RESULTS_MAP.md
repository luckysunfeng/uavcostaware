# Manuscript-to-artifact map

This file maps every quantitative experiment in the manuscript to its code,
reference data, and analysis artifact.

| Manuscript item | Experiment/code | Reference artifact |
|---|---|---|
| Simulation parameters | constants in both C experiment programs | source code and experiment READMEs |
| Main 30-seed scaling table | `experiments/01_main_scaling/run_main_scaling.c` | `results/reference/summary/main_scaling_30_seeds.csv` |
| Figure 2 scaling curves | main scaling summary | `results/figures/figure_2_scaling_results.{png,pdf,tex}` |
| Spatial robustness table at N=400 | spatial experiment, methods listed below | raw and summary spatial CSVs |
| Controlled-fleet high-budget PSO table | spatial experiment, `pso_high_12x16` versus complete LNS | raw and summary spatial CSVs |
| Candidate-MILP table | candidate-MILP runs plus four-cluster LNS rows | candidate-MILP raw/summary and spatial summary |
| Statistical analysis | `analysis/paired_statistics.py` | `results/reference/summary/paired_statistics.csv` |
| Parameter-sensitivity table | sensitivity mode of Experiment 2 | sensitivity raw and summary CSVs |
| Hover-energy discussion | `analysis/add_hover_energy.py` | `results/reference/derived/hover_energy_per_run.csv` |
| Figure 1 system model | editable TikZ schematic | `results/figures/figure_1_system_model.{png,pdf,tex}` |

## Historical output labels

Some raw files preserve identifiers used during code development. They map to
the paper as follows:

| Stored label | Manuscript name |
|---|---|
| `gno` or `GNO` | block-centroid initialization |
| `alns_no_refine` | LNS without continuous refinement |
| `fixed_gno_refine` | fixed-initializer position/altitude refinement |
| `alns_sca_lite`, `ALNS`, or `lns_based` | complete proposed LNS |
| `kmeans_grid` | fixed-fleet k-means with altitude-grid search |
| `pso_high_12x16` | controlled-fleet high-budget PSO |
| `rahimi_ev_reimplementation` | time-limited compatible candidate-MILP |

These labels are retained so that the committed CSVs remain directly
traceable to the programs that produced them.

## Exact filters for reported tables

- Spatial robustness table: select `N=400` from
  `spatial_robustness_ablation_pso_summary.csv` and the metrics `coverage`,
  `K`, and `p5_rate_Mbps` for the five primary methods.
- High-budget PSO table: select `N=400`, methods `alns_sca_lite` and
  `pso_high_12x16`, and metrics `coverage`, `p5_rate_Mbps`, and `time_s`.
- Candidate-MILP table: candidate rows come from
  `candidate_milp_summary.csv`; paired LNS rows come from the four-cluster
  `alns_sca_lite` records at `N=100` and `N=200`.
- Sensitivity table: select metrics `coverage`, `K`, and `p5_rate_Mbps` from
  `parameter_sensitivity_summary.csv`.

The manuscript reports means and sample standard deviations unless noted
otherwise. The lower-tail metric includes a zero score for each unserved user
before taking the fifth percentile.
