import csv

P0_W = 79.86
PI_W = 88.63
P_TX_W = 10 ** ((33.0 - 30.0) / 10.0)
P_CIRCUIT_W = 0.0
BATTERY_WH = 22.2 * 16.0
USABLE_FRACTION = 0.80
MISSION_H = 1.0

with open("results/reproduced/spatial_robustness_ablation_pso_runs.csv", newline="") as f:
    rows = list(csv.DictReader(f))

fields = list(rows[0]) + [
    "hover_power_per_uav_W", "radio_power_per_uav_W",
    "fleet_power_W", "mission_energy_1h_Wh",
    "energy_per_served_user_Wh", "estimated_endurance_h"
]
with open("results/reproduced/hover_energy_per_run.csv", "w", newline="") as f:
    w = csv.DictWriter(f, fieldnames=fields)
    w.writeheader()
    for r in rows:
        k = int(r["K"]); n = int(r["N"]); served = float(r["coverage"]) * n
        hover = P0_W + PI_W
        radio = P_TX_W + P_CIRCUIT_W
        fleet_power = k * (hover + radio)
        energy = fleet_power * MISSION_H
        r.update({
            "hover_power_per_uav_W": f"{hover:.4f}",
            "radio_power_per_uav_W": f"{radio:.4f}",
            "fleet_power_W": f"{fleet_power:.4f}",
            "mission_energy_1h_Wh": f"{energy:.4f}",
            "energy_per_served_user_Wh": f"{energy/served:.6f}" if served else "nan",
            "estimated_endurance_h": f"{BATTERY_WH*USABLE_FRACTION/(hover+radio):.6f}",
        })
        w.writerow(r)
print("saved results/reproduced/hover_energy_per_run.csv")
