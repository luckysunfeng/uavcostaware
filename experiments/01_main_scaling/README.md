# Experiment 1: main scaling study

This program reproduces the manuscript's stored-order scaling experiment.

## Protocol

- Demand geometry: four Gaussian clusters centered at `(250,250)`,
  `(750,250)`, `(250,750)`, and `(750,750)` m
- Coordinate-wise standard deviation: 120 m, clipped to `[0,1000]`
- User counts: 100, 200, 300, and 400
- Runs: 30 independent seeds per user count
- Seeds: `1000 + 100 * size_index + run`, where `size_index=0..3`
- User order: generated/stored order is preserved
- Comparison: block-centroid initialization versus complete LNS
- LNS iterations: 20
- Runtime: C `clock()` around the post-initialization LNS loop only

## Build and run

From the repository root:

```bash
gcc -O2 -std=c11 experiments/01_main_scaling/run_main_scaling.c \
  -o build/run_main_scaling -lm
./build/run_main_scaling
```

Windows MinGW GCC:

```powershell
gcc -O2 -std=c11 experiments\01_main_scaling\run_main_scaling.c `
  -o build\run_main_scaling.exe
.\build\run_main_scaling.exe
```

Output:

```text
results/reproduced/main_scaling_30_seeds.csv
```

The committed counterpart used for the manuscript is
`results/reference/summary/main_scaling_30_seeds.csv`.
