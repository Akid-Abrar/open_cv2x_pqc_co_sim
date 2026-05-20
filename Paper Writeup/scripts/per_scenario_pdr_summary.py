"""
Diagnostic: overall packet accounting summary per LOS scenario.
For each LOS scenario (sorted by scenario number), prints:
  - Total sent (from sender_summary.csv)
  - Received by RSU (rows in appl_logs.csv, including unknown senders)
  - Unknown-sender rows (verified=0, excluded from PDR)
  - Known-sender rows (used in PDR)
  - Known-sender rows with dist_m <= 600 (mapped to a bin)
  - Known-sender rows with dist_m > 600 (dropped due to distance cap)
  - Overall PDR (known + in-range / total sent)

Run from the "Paper Writeup" directory.
"""

import re
import pandas as pd
from pathlib import Path

BASE_DIR = Path("Simulation Results/Simulation Logs Storage-selected")
MAX_DIST = 700


def is_los_scenario(folder_name):
    match = re.search(r'_(\d+)_', folder_name)
    return match is not None and int(match.group(1)) % 2 == 1


def scenario_number(folder):
    match = re.search(r'_(\d+)_', folder.name)
    return int(match.group(1)) if match else 0


def find_scenarios(base_dir):
    candidates = [
        f for f in base_dir.iterdir()
        if f.is_dir()
        and (f / "logs" / "appl_logs.csv").exists()
        and is_los_scenario(f.name)
    ]
    return sorted(candidates, key=scenario_number)


scenarios = find_scenarios(BASE_DIR)
print(f"Found {len(scenarios)} LOS scenarios\n")
print("=" * 80)

for scenario_dir in scenarios:
    name = scenario_dir.name
    appl_path = scenario_dir / "logs" / "appl_logs.csv"
    summ_path = scenario_dir / "logs" / "sender_summary.csv"

    print(f"\nSCENARIO: {name}")
    print("-" * 70)

    try:
        df = pd.read_csv(appl_path)
    except Exception as e:
        print(f"  ERROR loading appl_logs.csv: {e}")
        continue

    total_received = len(df)
    unknown_rows = df[df['sender'] == 'unknown']
    known_rows = df[df['sender'] != 'unknown']
    known_in_range = known_rows[known_rows['dist_m'] <= MAX_DIST]
    known_out_range = known_rows[known_rows['dist_m'] > MAX_DIST]

    print(f"  Total rows in appl_logs.csv  : {total_received:>10,}")
    print(f"  Unknown sender (verified=0)  : {len(unknown_rows):>10,}  ({100*len(unknown_rows)/total_received:.2f}%)")
    print(f"  Known sender   (verified=1)  : {len(known_rows):>10,}  ({100*len(known_rows)/total_received:.2f}%)")
    print(f"    dist_m <= {MAX_DIST}m (binned)   : {len(known_in_range):>10,}  ({100*len(known_in_range)/total_received:.2f}%)")
    print(f"    dist_m >  {MAX_DIST}m (dropped)  : {len(known_out_range):>10,}  ({100*len(known_out_range)/total_received:.2f}%)")

    if summ_path.exists():
        summ = pd.read_csv(summ_path)
        total_sent = summ['total_sent'].sum()
        n_senders = len(summ)
        overall_pdr = len(known_in_range) / total_sent * 100 if total_sent > 0 else float('nan')
        print(f"\n  sender_summary.csv:")
        print(f"    Unique senders              : {n_senders:>10,}")
        print(f"    Total packets sent          : {total_sent:>10,}")
        print(f"    Overall PDR (known+in-range): {overall_pdr:>10.2f}%")
    else:
        print(f"\n  sender_summary.csv not found")

print(f"\n{'='*80}")
print("Done.")
