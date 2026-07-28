# Results

## `reference/`

These are the committed outputs used to prepare the manuscript.

- `raw/`: per-seed experiment and solver records
- `summary/`: aggregated statistics and the prespecified paired tests
- `derived/`: model-based hover-energy indicators derived from raw records

Do not overwrite these files when reproducing the experiments.

## `reproduced/`

The programs and scripts write a reviewer's new outputs here. Runtime fields
will normally differ by machine. The three-second candidate-MILP results can
also vary with SciPy/HiGHS version and system load.

## `figures/`

- `figure_1_system_model.*`: system-model schematic
- `figure_2_scaling_results.*`: scaling plot corresponding to the main
  30-seed summary

PNG files are the manuscript-ready raster versions. PDF files are vector
versions, and TeX files are editable TikZ/PGFPlots sources.
