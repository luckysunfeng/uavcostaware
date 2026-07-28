param(
    [switch]$SkipMilp
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot

function Invoke-PythonScript {
    param([string]$Path)
    python $Path
    if ($LASTEXITCODE -ne 0) { throw "Python script failed: $Path" }
}

Push-Location $repoRoot
try {
    New-Item -ItemType Directory -Force -Path "build" | Out-Null
    New-Item -ItemType Directory -Force -Path "results\reproduced" | Out-Null

    gcc -O2 -std=c11 `
        "experiments\01_main_scaling\run_main_scaling.c" `
        -o "build\run_main_scaling.exe"
    if ($LASTEXITCODE -ne 0) { throw "Failed to compile the main scaling experiment." }

    gcc -O2 -std=c11 `
        "experiments\02_spatial_robustness_ablation_pso\run_spatial_robustness_ablation_pso.c" `
        -o "build\run_spatial_experiments.exe"
    if ($LASTEXITCODE -ne 0) { throw "Failed to compile the spatial experiment." }

    & ".\build\run_main_scaling.exe"
    if ($LASTEXITCODE -ne 0) { throw "Main scaling experiment failed." }

    & ".\build\run_spatial_experiments.exe"
    if ($LASTEXITCODE -ne 0) { throw "Spatial experiment failed." }

    Invoke-PythonScript "analysis\summarize_spatial_results.py"
    Invoke-PythonScript "analysis\paired_statistics.py"
    Invoke-PythonScript "analysis\summarize_parameter_sensitivity.py"
    Invoke-PythonScript "analysis\add_hover_energy.py"

    if (-not $SkipMilp) {
        Invoke-PythonScript "experiments\03_candidate_milp\run_candidate_milp.py"
        Invoke-PythonScript "analysis\summarize_candidate_milp.py"
    }

    Write-Host "Reproduction complete. See results\reproduced\."
}
finally {
    Pop-Location
}
