# Cost-Aware LNS for 3D UAV-BS Deployment

This repository is the reproducibility package for:

> **Cost-Aware Large-Neighborhood Search for 3D UAV Base-Station
> Deployment in Emergency Networks**

It contains the implementations, canonical synthetic instances, per-seed
outputs, summary tables, statistical tests, and figure sources used in the
manuscript. No personal, confidential, or human-participant data are included.

## Repository layout

```text
UAVcostaware_codes/
|-- README.md
|-- LICENSE
|-- CITATION.cff
|-- requirements.txt
|-- analysis/                  # summaries, paired tests, and hover energy
|-- data/
|   `-- candidate_milp/       # canonical paired N=100 and N=200 instances
|-- docs/
|   `-- PAPER_RESULTS_MAP.md  # manuscript table/figure to file mapping
|-- experiments/
|   |-- 01_main_scaling/
|   |-- 02_spatial_robustness_ablation_pso/
|   `-- 03_candidate_milp/
|-- results/
|   |-- reference/            # outputs used in the submitted manuscript
|   |-- reproduced/           # destination for a reviewer's new run
|   `-- figures/              # PNG, vector PDF, and editable TeX
`-- scripts/                  # end-to-end reproduction scripts
```

The core simulations are C11 programs. Python is used for the independently
implemented candidate-site MILP benchmark and for statistical
post-processing. Historical internal identifiers such as `gno`,
`alns_sca_lite`, and `rahimi_ev` are retained in some CSV columns to preserve
traceability. Their manuscript-facing names are documented in
[PAPER_RESULTS_MAP.md](docs/PAPER_RESULTS_MAP.md).

## Experiments represented in the paper

1. **Main scaling:** four-cluster demand, 100--400 users, 30 independent
   seeds per size, stored user order, block-centroid initialization versus
   the proposed LNS.
2. **Spatial robustness, ablations, and controlled PSO:** four demand
   geometries, 100--400 users, 30 paired seeds, five primary methods and
   three controlled-fleet PSO budgets.
3. **Parameter sensitivity:** one-factor-at-a-time analysis at 400 users on
   the common seeds 20000--20029.
4. **Candidate-site MILP:** a time-limited compatible reimplementation
   inspired by Rahimi et al., evaluated on canonical paired four-cluster
   instances for 100 and 200 users.
5. **Post-processing:** descriptive summaries, 48 paired Wilcoxon tests with
   Holm adjustment, paired bootstrap intervals, random-sign permutation
   checks, and model-based hover-energy indicators.

See the README inside each experiment directory for its exact protocol.

## Requirements

- GCC or another C11 compiler
- Python 3.9 or later
- NumPy and SciPy (see `requirements.txt`)
- Optional: a LaTeX distribution with TikZ/PGFPlots for rebuilding figures

The submitted experiments were run on Windows 10 with an Intel Core
i7-10510U at 1.80 GHz and 16 GB RAM. CPU-time values will vary by compiler,
processor, and operating system.

Install the Python dependencies:

```bash
python -m pip install -r requirements.txt
```

## Reproduce the results

Run all commands from the repository root.

Windows PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\reproduce_all.ps1
```

Linux or macOS:

```bash
sh scripts/reproduce_all.sh
```

The candidate MILP solves 60 instances with a three-second limit per
instance. To reproduce the C experiments and analyses without that benchmark:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\reproduce_all.ps1 -SkipMilp
```

```bash
sh scripts/reproduce_all.sh --skip-milp
```

New files are written to `results/reproduced/`. The committed
`results/reference/` files are never overwritten, so reviewers can compare a
new run against the exact outputs accompanying the manuscript.

The C programs use the seeded standard-library pseudo-random generator.
Numerical results are deterministic for the tested Windows toolchain, but the
exact pseudo-random sequence is not guaranteed to be identical across C
runtime libraries. The canonical candidate-MILP instances are therefore
stored explicitly in `data/candidate_milp/`.

## Result verification

The most direct audit path is:

- inspect `results/reference/raw/` for per-seed records;
- inspect `results/reference/summary/` for manuscript aggregates and tests;
- use `docs/PAPER_RESULTS_MAP.md` to locate each reported table or figure;
- rerun the package and compare files under `results/reproduced/`.

The rate columns are full-bandwidth placement scores under the manuscript's
fixed-radio model; they are not simultaneous-transmission per-user
throughput.

## License and citation

The software and accompanying artifacts are released under the MIT License.
See `LICENSE` and `CITATION.cff`.
