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
from matplotlib.backends.backend_pdf import PdfPages
from io import BytesIO

plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.size'] = 10
plt.rcParams['axes.labelsize'] = 11
plt.rcParams['axes.titlesize'] = 10
plt.rcParams['xtick.labelsize'] = 10
plt.rcParams['ytick.labelsize'] = 10
plt.rcParams['legend.fontsize'] = 9

BASE_DIR    = Path("Simulation Logs")
OUTPUT_BASE = Path("PDR VS Distance 5GAA Approach")
OUTPUT_BASE.mkdir(exist_ok=True)

WINDOW_SEC  = 5.0    # 5-second sliding window, per 5GAA P-190033 Section 5.1.1
MIN_PKTS    = 5     # minimum received packets in window for a reliable PRR estimate
BIN_SIZES   = [30]
MAX_DIST    = 700

COLOR_ECDSA  = '#2E86AB'
COLOR_FALCON = '#A23B72'
COLOR_MEAN   = '#E76F51'
COLOR_MEDIAN = '#2A9D8F'


def scenario_number(folder):
    match = re.search(r'^(\d+)_', folder.name)
    return int(match.group(1)) if match else 0


def find_scenarios(base_dir):
    return sorted(
        (f for f in base_dir.iterdir()
         if f.is_dir() and (f / "logs" / "appl_logs.csv").exists()),
        key=scenario_number
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
generated_figs  = {}   # scenario_number -> BytesIO buffer (PDR plot)
latency_data    = {}   # scenario_number -> np.array of delay_ms values
scenario_names  = {}   # scenario_number -> folder name string
print(f"Found {len(scenarios)} scenarios")
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

        num = scenario_number(scenario_dir)
        latency_data[num]   = df['delay_ms'].dropna().values
        scenario_names[num] = name

        # out_dir = OUTPUT_BASE / name
        # out_dir.mkdir(exist_ok=True)

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
                f'{name}  [{algo_label}]\n',
                fontweight='bold'
            )
            handles, labels = ax.get_legend_handles_labels()
            ax.legend([bp_legend] + handles, ['Distribution'] + labels,
                      loc='upper right', framealpha=0.9)
            ax.grid(True, alpha=0.3, linestyle='--')
            ax.set_xlim([0, MAX_DIST])

            plt.tight_layout()

            # fname = f"pdr_5gaa_window{int(WINDOW_SEC)}s_minpkts{MIN_PKTS}_bin{bin_size}m"
            # img_path = out_dir / f"{fname}.png"
            # plt.savefig(img_path, dpi=300, bbox_inches='tight')
            if bin_size == BIN_SIZES[0]:
                buf = BytesIO()
                plt.savefig(buf, format='png', dpi=150, bbox_inches='tight')
                buf.seek(0)
                generated_figs[scenario_number(scenario_dir)] = buf
            plt.close()

        # print(f"  Saved to: {out_dir}")

    except Exception as e:
        print(f"  ERROR: {e}")

print(f"\nDone. Output: {OUTPUT_BASE}/")

# -----------------------------------------------------------------------
# Helper: latency comparison page for a scenario pair
# -----------------------------------------------------------------------
def make_latency_fig(n, n2):
    d1    = latency_data.get(n,  np.array([]))
    d2    = latency_data.get(n2, np.array([]))
    name1 = scenario_names.get(n,  f'Scenario {n}')
    name2 = scenario_names.get(n2, f'Scenario {n2}')

    color1 = COLOR_FALCON if 'falcon' in name1.lower() else COLOR_ECDSA
    color2 = COLOR_FALCON if 'falcon' in name2.lower() else COLOR_ECDSA
    algo1  = 'Falcon-512' if 'falcon' in name1.lower() else 'ECDSA P-256'
    algo2  = 'Falcon-512' if 'falcon' in name2.lower() else 'ECDSA P-256'

    fig = plt.figure(figsize=(8.5, 11))
    gs  = fig.add_gridspec(2, 1, height_ratios=[3, 1], hspace=0.35)
    ax_box = fig.add_subplot(gs[0])
    ax_tbl = fig.add_subplot(gs[1])

    # --- boxplot ---
    box_data    = [d1 if len(d1) > 0 else np.array([0]),
                   d2 if len(d2) > 0 else np.array([0])]
    tick_labels = [f'{name1}\n[{algo1}]', f'{name2}\n[{algo2}]']

    bp = ax_box.boxplot(box_data, tick_labels=tick_labels, patch_artist=True,
                        medianprops=dict(color='black', linewidth=2.0),
                        whiskerprops=dict(linewidth=1.2),
                        capprops=dict(linewidth=1.2),
                        flierprops=dict(marker='.', markersize=3, alpha=0.4))
    bp['boxes'][0].set_facecolor(color1)
    bp['boxes'][0].set_alpha(0.85)
    bp['boxes'][1].set_facecolor(color2)
    bp['boxes'][1].set_alpha(0.85)

    ax_box.axhline(y=100, color='red', linestyle='--', linewidth=1.5,
                   label='100 ms limit (SAE J2945/1)')
    ax_box.set_ylabel('End-to-End Latency (ms)', fontweight='bold')
    ax_box.set_title(f'Latency Comparison — Scenario {n} vs Scenario {n2}',
                     fontweight='bold', fontsize=12)
    ax_box.grid(axis='y', alpha=0.3, linestyle='--')
    ax_box.set_axisbelow(True)
    ax_box.legend(loc='upper right', framealpha=0.9)

    # --- stats table ---
    def stats_row(arr, label):
        if len(arr) == 0:
            return [label, 'N/A', 'N/A', 'N/A', 'N/A', 'N/A']
        return [label,
                f'{np.min(arr):.2f}',
                f'{np.max(arr):.2f}',
                f'{np.mean(arr):.2f}',
                f'{np.median(arr):.2f}',
                f'{np.percentile(arr, 95):.2f}']

    col_labels = ['Scenario', 'Min (ms)', 'Max (ms)', 'Mean (ms)', 'Median (ms)', 'P95 (ms)']
    table_data = [
        stats_row(d1, f'{name1} [{algo1}]'),
        stats_row(d2, f'{name2} [{algo2}]'),
    ]

    ax_tbl.axis('off')
    tbl = ax_tbl.table(cellText=table_data, colLabels=col_labels,
                       loc='center', cellLoc='center')
    tbl.auto_set_font_size(False)
    tbl.set_fontsize(10)
    tbl.scale(1, 2.5)

    return fig


# -----------------------------------------------------------------------
# Combined PDF: PDR page then latency page for each scenario pair
# -----------------------------------------------------------------------
first_half = sorted(n for n in generated_figs if n <= 12)
pdf_path   = OUTPUT_BASE / "all_scenarios_combined.pdf"

print(f"\nGenerating combined PDF: {pdf_path}")
with PdfPages(pdf_path) as pdf:
    for page_num, n in enumerate(first_half, start=1):
        n2 = n + 12

        # --- PDR page ---
        fig, axes = plt.subplots(2, 1, figsize=(8.5, 11))
        fig.subplots_adjust(hspace=0.08)
        for ax, buf in zip(axes, [generated_figs.get(n), generated_figs.get(n2)]):
            if buf is not None:
                buf.seek(0)
                ax.imshow(plt.imread(buf))
            else:
                ax.text(0.5, 0.5, 'Figure not available',
                        ha='center', va='center', transform=ax.transAxes,
                        fontsize=10, color='gray')
            ax.axis('off')
        pdf.savefig(fig, bbox_inches='tight')
        plt.close(fig)

        # --- Latency page ---
        fig_lat = make_latency_fig(n, n2)
        pdf.savefig(fig_lat, bbox_inches='tight')
        plt.close(fig_lat)

        print(f"  Pages {page_num*2 - 1}-{page_num*2}: scenario {n} + scenario {n2}")

print(f"Combined PDF saved to: {pdf_path}")
