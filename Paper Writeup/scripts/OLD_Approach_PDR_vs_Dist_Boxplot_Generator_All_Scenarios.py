"""
PDR vs distance for all scenarios with:
  - Error bars (mean +/- std) at each bin center
  - Fitted line through per-bin means (solid)
  - Fitted line through per-bin medians (dashed)
Saves to temp_figures_1/<scenario_name>/ scenariowise.
"""

import re
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from pathlib import Path

plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.size'] = 10
plt.rcParams['axes.labelsize'] = 11
plt.rcParams['axes.titlesize'] = 10
plt.rcParams['xtick.labelsize'] = 10
plt.rcParams['ytick.labelsize'] = 10
plt.rcParams['legend.fontsize'] = 9

BASE_DIR    = Path("Simulation Results/Simulation Logs Storage-selected")
OUTPUT_BASE = Path("Th_5_Bin_30")
OUTPUT_BASE.mkdir(exist_ok=True)

MIN_THRESHOLD = 5
BIN_SIZES     = [30]
MAX_DIST      = 700

COLOR_ECDSA  = '#2E86AB'
COLOR_FALCON = '#A23B72'


def is_los_scenario(folder_name):
    """Return True if the scenario number is odd (LOS scenarios)."""
    match = re.search(r'_(\d+)_', folder_name)
    return match is not None and int(match.group(1)) % 2 == 1


def find_scenarios(base_dir):
    return sorted(
        f for f in base_dir.iterdir()
        if f.is_dir()
        and (f / "logs" / "appl_logs.csv").exists()
        and is_los_scenario(f.name)
    )


def calculate_pdr_msgid(appl_df, bin_size, min_threshold):
    """
    For each (sender, receiver) pair, compute per-bin PDR using msgId span.
    Cross-bin boundary correction: missing packets at the boundary between
    consecutive traversed bins are split between them. For n missing packets,
    ceil(n/2) are attributed to the earlier bin and floor(n/2) to the later bin.
    If n is odd, the extra packet goes to the earlier bin.
    Returns bin centers plus per-bin mean, median, and raw distributions.
    """
    bins = np.arange(0, MAX_DIST + bin_size, bin_size)
    bin_centers = [(bins[i] + bins[i + 1]) / 2 for i in range(len(bins) - 1)]

    df = appl_df.copy()
    df['bin_idx'] = pd.cut(df['dist_m'], bins=bins, labels=False)
    df = df.dropna(subset=['bin_idx'])
    df['bin_idx'] = df['bin_idx'].astype(int)

    bin_pdrs = {i: [] for i in range(len(bin_centers))}

    for (sender, receiver), pair_df in df.groupby(['sender', 'receiver']):
        # Collect per-bin stats for this link
        bin_data = {}
        for b, grp in pair_df.groupby('bin_idx'):
            if len(grp) < min_threshold:
                continue
            span = int(grp['msgId'].max() - grp['msgId'].min() + 1)
            if span <= 0:
                continue
            bin_data[b] = {
                'received':    len(grp),
                'span':        span,
                'first_msgId': int(grp['msgId'].min()),
                'last_msgId':  int(grp['msgId'].max()),
                'extra_missed': 0,
            }

        # Cross-bin boundary correction over consecutive traversed bins
        sorted_bins = sorted(bin_data.keys())
        for k in range(len(sorted_bins) - 1):
            b_prev = sorted_bins[k]
            b_next = sorted_bins[k + 1]
            n_gap = bin_data[b_next]['first_msgId'] - bin_data[b_prev]['last_msgId'] - 1
            if n_gap > 0:
                bin_data[b_prev]['extra_missed'] += (n_gap + 1) // 2  # ceil(n/2)
                bin_data[b_next]['extra_missed'] += n_gap // 2         # floor(n/2)

        # Compute PDR with adjusted denominator
        for b, data in bin_data.items():
            adjusted_span = data['span'] + data['extra_missed']
            pdr = data['received'] / adjusted_span * 100.0
            bin_pdrs[b].append(pdr)

    centers, means, medians, raw = [], [], [], []
    for i, c in enumerate(bin_centers):
        vals = bin_pdrs[i]
        if vals:
            arr = np.array(vals)
            centers.append(c)
            means.append(float(np.mean(arr)))
            medians.append(float(np.median(arr)))
            raw.append(arr)

    return centers, means, medians, raw


COLOR_MEAN   = '#E76F51'   # orange-red for mean line
COLOR_MEDIAN = '#2A9D8F'   # teal for median line


scenarios = find_scenarios(BASE_DIR)
print(f"Found {len(scenarios)} scenarios\n")

for scenario_dir in scenarios:
    name = scenario_dir.name
    is_falcon = 'falcon' in name.lower()
    color = COLOR_FALCON if is_falcon else COLOR_ECDSA
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
            centers, means, medians, raw = calculate_pdr_msgid(df, bin_size, MIN_THRESHOLD)

            if not centers:
                print(f"  bin{bin_size}m: no qualifying bins, skipping")
                continue

            cx = np.array(centers)
            mn = np.array(means)
            md = np.array(medians)

            fig, ax = plt.subplots(figsize=(7, 4.5))

            # Box plots at each bin center
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
            # Add a dummy handle for the legend
            from matplotlib.patches import Patch
            bp_legend = Patch(facecolor=color, alpha=0.3, edgecolor=color, label='Distribution')

            # Line connecting mean values
            ax.plot(cx, mn, '-o', color=COLOR_MEAN, linewidth=2.0,
                    markersize=4, label='Mean', zorder=4)

            # Line connecting median values
            ax.plot(cx, md, '-s', color=COLOR_MEDIAN, linewidth=2.0,
                    markersize=4, label='Median', zorder=4)

            ax.set_xlabel('Distance (m)', fontweight='bold')
            ax.set_ylabel('PDR (%)', fontweight='bold')
            ax.set_title(
                f'{name}  [{algo_label}]\n'
                f'Bin: {bin_size} m  |  Min threshold: {MIN_THRESHOLD} pkts  |  t ≤ {time_limit:.0f} s',
                fontweight='bold'
            )
            handles, labels = ax.get_legend_handles_labels()
            ax.legend([bp_legend] + handles, ['Distribution'] + labels,
                      loc='upper right', framealpha=0.9)
            ax.grid(True, alpha=0.3, linestyle='--')
            ax.set_xlim([0, MAX_DIST])

            plt.tight_layout()

            fname = f"pdr_bin{bin_size}m_thresh{MIN_THRESHOLD}"
            plt.savefig(out_dir / f"{fname}.png", dpi=300, bbox_inches='tight')
            plt.close()

        print(f"  Saved to: {out_dir}")

    except Exception as e:
        print(f"  ERROR: {e}")

print(f"\nDone. Output: {OUTPUT_BASE}/")
