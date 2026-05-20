"""
5GAA-style PDR vs distance for all LOS scenarios.

Methodology (mirrors 5GAA P-190033, Sections 5.1.1 and 5.2.4):
  - For each (sender, receiver) pair, sort packets by time.
  - For each received packet at time t and distance d, compute PRR over the
    5-second window [t - 2.5s, t + 2.5s] using msgId span as proxy for sent
    count (we have no Tx log; msgId span over a 5s window is a stable estimate).
  - Record the (d, PRR) pair as one scatter point.
  - After collecting all scatter points, bin by distance and plot box + median.

Key difference from v2 (bin-first approach):
  - The window is time-based, not distance-based.
  - Each data point represents a reliable 5-second observation (~50 packets),
    not a potentially sparse distance-bin slice.
  - Boundary artifacts are suppressed because the window is continuous in time.
"""

import re
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from pathlib import Path
from matplotlib.patches import Patch

plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.size'] = 10
plt.rcParams['axes.labelsize'] = 11
plt.rcParams['axes.titlesize'] = 10
plt.rcParams['xtick.labelsize'] = 10
plt.rcParams['ytick.labelsize'] = 10
plt.rcParams['legend.fontsize'] = 9

BASE_DIR    = Path("Simulation Results/Simulation Logs Storage-selected")
OUTPUT_BASE = Path("5gaa_approach_bin_30_Th_5")
OUTPUT_BASE.mkdir(exist_ok=True)

WINDOW_SEC  = 5.0    # 5-second sliding window, per 5GAA P-190033 Section 5.1.1
MIN_PKTS    = 5     # minimum received packets in window for a reliable PRR estimate
BIN_SIZES   = [30]
MAX_DIST    = 700

COLOR_ECDSA  = '#2E86AB'
COLOR_FALCON = '#A23B72'
COLOR_MEAN   = '#E76F51'
COLOR_MEDIAN = '#2A9D8F'


def is_los_scenario(folder_name):
    match = re.search(r'_(\d+)_', folder_name)
    return match is not None and int(match.group(1)) % 2 == 1


def find_scenarios(base_dir):
    return sorted(
        f for f in base_dir.iterdir()
        if f.is_dir()
        and (f / "logs" / "appl_logs.csv").exists()
        and is_los_scenario(f.name)
    )


def calculate_pdr_5gaa(appl_df, bin_size):
    """
    5GAA sliding window PRR vs distance.

    For each received packet at time t:
      - Define window [t - 2.5s, t + 2.5s].
      - Count received packets in window and compute msgId span.
      - PRR = received_in_window / span * 100.
      - Record (dist_m of this packet, PRR) as one scatter point.

    Scatter points are then binned by distance.
    Returns bin centers, means, medians, and raw per-bin distributions.
    """
    half_w = WINDOW_SEC / 2.0
    bins = np.arange(0, MAX_DIST + bin_size, bin_size)
    bin_centers = [(bins[i] + bins[i + 1]) / 2 for i in range(len(bins) - 1)]

    scatter_dist = []
    scatter_pdr  = []

    for (sender, receiver), pair_df in appl_df.groupby(['sender', 'receiver']):
        pair_df = pair_df.sort_values('t')
        t_vals    = pair_df['t'].values
        msgid_vals = pair_df['msgId'].values
        dist_vals  = pair_df['dist_m'].values

        for idx in range(len(t_vals)):
            d = dist_vals[idx]
            if d > MAX_DIST:
                continue

            t_c = t_vals[idx]
            lo  = np.searchsorted(t_vals, t_c - half_w, side='left')
            hi  = np.searchsorted(t_vals, t_c + half_w, side='right')

            win_msgids = msgid_vals[lo:hi]
            if len(win_msgids) < MIN_PKTS:
                continue

            span = int(win_msgids.max() - win_msgids.min() + 1)
            if span <= 0:
                continue

            pdr = len(win_msgids) / span * 100.0
            scatter_dist.append(d)
            scatter_pdr.append(pdr)

    if not scatter_dist:
        return [], [], [], []

    scatter_dist = np.array(scatter_dist)
    scatter_pdr  = np.array(scatter_pdr)

    bin_idx = np.digitize(scatter_dist, bins) - 1
    # clip to valid bin range
    valid = (bin_idx >= 0) & (bin_idx < len(bin_centers))
    bin_idx     = bin_idx[valid]
    scatter_pdr = scatter_pdr[valid]

    centers, means, medians, raw = [], [], [], []
    for i, c in enumerate(bin_centers):
        vals = scatter_pdr[bin_idx == i]
        if len(vals) > 0:
            centers.append(c)
            means.append(float(np.mean(vals)))
            medians.append(float(np.median(vals)))
            raw.append(vals)

    return centers, means, medians, raw


scenarios = find_scenarios(BASE_DIR)
print(f"Found {len(scenarios)} LOS scenarios")
print(f"Window: {WINDOW_SEC}s  |  Min packets in window: {MIN_PKTS}\n")

for scenario_dir in scenarios:
    name = scenario_dir.name
    is_falcon = 'falcon' in name.lower()
    color      = COLOR_FALCON if is_falcon else COLOR_ECDSA
    algo_label = 'Falcon-512' if is_falcon else 'ECDSA P-256'

    print(f"Processing: {name}")

    try:
        df = pd.read_csv(scenario_dir / "logs" / "appl_logs.csv")
        time_limit = float(df['t'].max())
        df = df[df['t'] <= time_limit]
        df = df[df['sender'] != 'unknown']
        print(f"  t <= {time_limit:.1f}s  |  packets: {len(df):,}")

        out_dir = OUTPUT_BASE / name
        out_dir.mkdir(exist_ok=True)

        for bin_size in BIN_SIZES:
            centers, means, medians, raw = calculate_pdr_5gaa(df, bin_size)

            if not centers:
                print(f"  bin{bin_size}m: no qualifying windows, skipping")
                continue

            cx = np.array(centers)
            mn = np.array(means)
            md = np.array(medians)

            fig, ax = plt.subplots(figsize=(7, 4.5))

            bp = ax.boxplot(
                raw,
                positions=cx,
                widths=bin_size * 0.6,
                patch_artist=True,
                manage_ticks=False,
                boxprops=dict(facecolor=color, alpha=0.3, linewidth=1.2),
                medianprops=dict(color='black', linewidth=1.8),
                whiskerprops=dict(color=color, linewidth=1.2),
                capprops=dict(color=color, linewidth=1.2),
                flierprops=dict(marker='.', color=color, markersize=2, alpha=0.4),
            )
            bp_legend = Patch(facecolor=color, alpha=0.3, edgecolor=color,
                              label='Distribution')

            ax.plot(cx, mn, '-o', color=COLOR_MEAN, linewidth=2.0,
                    markersize=4, label='Mean', zorder=4)
            ax.plot(cx, md, '-s', color=COLOR_MEDIAN, linewidth=2.0,
                    markersize=4, label='Median', zorder=4)

            ax.set_xlabel('Distance (m)', fontweight='bold')
            ax.set_ylabel('PDR (%)', fontweight='bold')
            ax.set_title(
                f'{name}  [{algo_label}]\n'
                f'5GAA sliding window: {WINDOW_SEC}s  |  '
                f'Min pkts/window: {MIN_PKTS}  |  Bin: {bin_size}m',
                fontweight='bold'
            )
            handles, labels = ax.get_legend_handles_labels()
            ax.legend([bp_legend] + handles, ['Distribution'] + labels,
                      loc='upper right', framealpha=0.9)
            ax.grid(True, alpha=0.3, linestyle='--')
            ax.set_xlim([0, MAX_DIST])

            plt.tight_layout()

            fname = f"pdr_5gaa_window{int(WINDOW_SEC)}s_minpkts{MIN_PKTS}_bin{bin_size}m"
            plt.savefig(out_dir / f"{fname}.png", dpi=300, bbox_inches='tight')
            plt.close()

        print(f"  Saved to: {out_dir}")

    except Exception as e:
        print(f"  ERROR: {e}")

print(f"\nDone. Output: {OUTPUT_BASE}/")
