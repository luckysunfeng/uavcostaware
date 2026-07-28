#!/usr/bin/env sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$repo_root"

mkdir -p build results/reproduced

cc -O2 -std=c11 experiments/01_main_scaling/run_main_scaling.c \
  -o build/run_main_scaling -lm
cc -O2 -std=c11 \
  experiments/02_spatial_robustness_ablation_pso/run_spatial_robustness_ablation_pso.c \
  -o build/run_spatial_experiments -lm

./build/run_main_scaling
./build/run_spatial_experiments
python3 analysis/summarize_spatial_results.py
python3 analysis/paired_statistics.py
python3 analysis/summarize_parameter_sensitivity.py
python3 analysis/add_hover_energy.py

if [ "${1:-}" != "--skip-milp" ]; then
  python3 experiments/03_candidate_milp/run_candidate_milp.py
  python3 analysis/summarize_candidate_milp.py
fi

echo "Reproduction complete. See results/reproduced/."
