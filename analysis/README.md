# Analysis scripts

Run these scripts from the repository root after generating the corresponding
files in `results/reproduced/`.

| Script | Input | Output |
|---|---|---|
| `summarize_spatial_results.py` | spatial/ablation/PSO per-seed runs | descriptive summary |
| `paired_statistics.py` | spatial/ablation/PSO per-seed runs | 48 prespecified paired tests |
| `summarize_parameter_sensitivity.py` | sensitivity per-seed runs | sensitivity summary |
| `add_hover_energy.py` | spatial/ablation/PSO per-seed runs | model-based energy indicators |
| `summarize_candidate_milp.py` | candidate-MILP per-seed runs | MILP summary and solver diagnostics |

`paired_statistics.py` uses two-sided SciPy Wilcoxon signed-rank tests,
omits zero differences under the Wilcox convention, assigns `p=1` when every
paired difference is zero, applies Holm correction across 48 tests, computes
20,000-resample paired bootstrap intervals, and performs 50,000-draw
random-sign permutation checks. Its random seed is fixed at 20260718.
