"""
Generate paper figures for IEEE CNS 2026.
Level of Service C, LOS scenarios only — 200 s of simulation data.

Figures saved to IEEE_CNS_2026/figures/:
  fig_spdu_size.pdf/png        — Combined SPDU size bar chart (cert + digest side-by-side)
  fig3_pdr_vs_distance.pdf/png — 5GAA sliding-window PDR vs Distance (both algos, 90% threshold)
  fig4_latency.pdf/png         — End-to-end latency boxplot

PDR methodology: 5GAA P-190033 §5.1.1 / §5.2.4
  5-second centered window per received packet; PDR = received / msgId-span;
  scatter points binned by distance at window center.
"""

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from pathlib import Path

# ---------------------------------------------------------------------------
# Publication style (IEEE single-column, fonts <= 9pt paper body size)
# ---------------------------------------------------------------------------

fontsize = 5
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.size'] = fontsize
plt.rcParams['axes.labelsize'] = fontsize
plt.rcParams['axes.titlesize'] = fontsize
plt.rcParams['xtick.labelsize'] = fontsize
plt.rcParams['ytick.labelsize'] = fontsize
plt.rcParams['legend.fontsize'] = fontsize

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
BASE_DIR   = Path("Simulation Results/Simulation Logs Storage-selected")
ECDSA_DIR  = BASE_DIR / "simulation_logs__5_ECDSA_C_LOS"  / "logs"
FALCON_DIR = BASE_DIR / "simulation_logs__17_Falcon_C_LOS" / "logs"
OUTPUT_DIR = Path("IEEE_CNS_2026/figures")
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

TIME_LIMIT  = 200.0   # seconds of simulation data to use
WINDOW_SEC  = 5.0     # 5GAA sliding window length (§5.1.1)
MIN_PKTS    = 5       # minimum received packets in window
BIN_SIZE    = 30      # distance bin width (m)
MAX_DIST    = 700     # maximum distance to plot (m)

COLOR_ECDSA  = '#2E86AB'
COLOR_FALCON = '#A23B72'

FIG_SIZE          = (3.5, 2.6)   # IEEE single-column figure size (inches)
FIG_SIZE_COMBINED = (3.5, 2.4)   # two-panel combined figure

labels = ['ECDSA\nP-256', 'Falcon-512']

# ---------------------------------------------------------------------------
# Load and filter data
# ---------------------------------------------------------------------------
print("Loading data...")

ecdsa_rx = pd.read_csv(ECDSA_DIR / "appl_logs.csv")
ecdsa_rx = ecdsa_rx[(ecdsa_rx['t'] <= TIME_LIMIT) & (ecdsa_rx['sender'] != 'unknown')]

falcon_rx = pd.read_csv(FALCON_DIR / "appl_logs.csv")
falcon_rx = falcon_rx[(falcon_rx['t'] <= TIME_LIMIT) & (falcon_rx['sender'] != 'unknown')]

print(f"  ECDSA  packets: {len(ecdsa_rx):,}")
print(f"  Falcon packets: {len(falcon_rx):,}")


# ---------------------------------------------------------------------------
# 5GAA sliding-window PDR helper
# ---------------------------------------------------------------------------
def calculate_pdr_5gaa(appl_df, bin_size=BIN_SIZE, window_sec=WINDOW_SEC,
                        min_pkts=MIN_PKTS, max_dist=MAX_DIST):
    """
    For each received packet at time t:
      - Window [t - half_w, t + half_w] over this (sender, receiver) pair.
      - PDR = received_in_window / msgId_span * 100.
      - Record (dist_m of center packet, PDR) as one scatter point.
    Bin scatter by distance; return bin centers, means, stds, medians.
    """
    half_w = window_sec / 2.0
    bins = np.arange(0, max_dist + bin_size, bin_size)
    bin_centers = np.array([(bins[i] + bins[i + 1]) / 2 for i in range(len(bins) - 1)])

    scatter_dist = []
    scatter_pdr  = []

    for _, pair_df in appl_df.groupby(['sender', 'receiver']):
        pair_df  = pair_df.sort_values('t')
        t_vals   = pair_df['t'].values
        mid_vals = pair_df['msgId'].values
        d_vals   = pair_df['dist_m'].values

        for idx in range(len(t_vals)):
            if d_vals[idx] > max_dist:
                continue
            t_c = t_vals[idx]
            lo  = np.searchsorted(t_vals, t_c - half_w, side='left')
            hi  = np.searchsorted(t_vals, t_c + half_w, side='right')
            win = mid_vals[lo:hi]
            if len(win) < min_pkts:
                continue
            span = int(win.max() - win.min() + 1)
            if span <= 0:
                continue
            scatter_dist.append(d_vals[idx])
            scatter_pdr.append(len(win) / span * 100.0)

    if not scatter_dist:
        return bin_centers, np.full(len(bin_centers), np.nan), \
               np.full(len(bin_centers), np.nan), np.full(len(bin_centers), np.nan)

    sd = np.array(scatter_dist)
    sp = np.array(scatter_pdr)
    bi = np.digitize(sd, bins) - 1
    ok = (bi >= 0) & (bi < len(bin_centers))
    bi, sp = bi[ok], sp[ok]

    means   = np.full(len(bin_centers), np.nan)
    stds    = np.full(len(bin_centers), np.nan)
    medians = np.full(len(bin_centers), np.nan)
    for i in range(len(bin_centers)):
        vals = sp[bi == i]
        if len(vals) > 0:
            means[i]   = np.mean(vals)
            stds[i]    = np.std(vals)
            medians[i] = np.median(vals)

    return bin_centers, means, stds, medians


# ---------------------------------------------------------------------------
# Figure 1: Combined SPDU size — certificate and digest side-by-side
# ---------------------------------------------------------------------------
print("Generating fig_spdu_size ...")

cert_sizes = [
    ecdsa_rx[ecdsa_rx['signerType'] == 1]['spdu_size'].median(),
    falcon_rx[falcon_rx['signerType'] == 1]['spdu_size'].median(),
]
digest_sizes = [
    ecdsa_rx[ecdsa_rx['signerType'] == 0]['spdu_size'].median(),
    falcon_rx[falcon_rx['signerType'] == 0]['spdu_size'].median(),
]

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=FIG_SIZE_COMBINED)

y_max = max(max(cert_sizes), max(digest_sizes)) * 1.3

# Left panel: with certificate
bars1 = ax1.bar(labels, cert_sizes, color=[COLOR_ECDSA, COLOR_FALCON],
                alpha=0.85, edgecolor='black', linewidth=1.0, width=0.5)
for bar, val in zip(bars1, cert_sizes):
    ax1.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + 20,
             f'{int(val)} B', ha='center', va='bottom', fontweight='bold', fontsize=4)
ax1.set_ylabel('SPDU Size (bytes)', fontweight='bold')
ax1.set_title('With Certificate', fontsize=6)
ax1.set_ylim([0, y_max])
ax1.grid(axis='y', alpha=0.3, linestyle='--')
ax1.set_axisbelow(True)

# Right panel: digest only
bars2 = ax2.bar(labels, digest_sizes, color=[COLOR_ECDSA, COLOR_FALCON],
                alpha=0.85, edgecolor='black', linewidth=1.0, width=0.5)
for bar, val in zip(bars2, digest_sizes):
    ax2.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + 8,
             f'{int(val)} B', ha='center', va='bottom', fontweight='bold', fontsize=4)
ax2.set_title('Digest Only', fontsize=6)
ax2.set_ylim([0, y_max])
ax2.grid(axis='y', alpha=0.3, linestyle='--')
ax2.set_axisbelow(True)

plt.tight_layout()
plt.savefig(OUTPUT_DIR / 'fig_spdu_size.pdf', dpi=300, bbox_inches='tight')
plt.savefig(OUTPUT_DIR / 'fig_spdu_size.png', dpi=300, bbox_inches='tight')
plt.close()
print("  Saved fig_spdu_size")


# ---------------------------------------------------------------------------
# Figure 2: PDR vs Distance (5GAA sliding window, lines only, 90% threshold)
# ---------------------------------------------------------------------------
print("Generating fig3_pdr_vs_distance ...")

cx, e_mean, e_std, e_med = calculate_pdr_5gaa(ecdsa_rx)
_,  f_mean, f_std, f_med = calculate_pdr_5gaa(falcon_rx)

fig, ax = plt.subplots(figsize=FIG_SIZE)

ok_e = ~np.isnan(e_mean)
ax.plot(cx[ok_e], e_mean[ok_e], '-o', color=COLOR_ECDSA, linewidth=1.8,
        markersize=3.5, label='ECDSA P-256')

ok_f = ~np.isnan(f_mean)
ax.plot(cx[ok_f], f_mean[ok_f], '-s', color=COLOR_FALCON, linewidth=1.8,
        markersize=3.5, label='Falcon-512')

ax.axhline(y=90, color='red', linestyle='--', linewidth=1.5, label='90% threshold')

ax.set_xlabel('Distance (m)', fontweight='bold')
ax.set_ylabel('PDR (%)', fontweight='bold')
ax.set_xlim([0, MAX_DIST])
ax.set_ylim([0, 105])
ax.legend(loc='lower left', framealpha=0.9)
ax.grid(True, alpha=0.3, linestyle='--')
plt.tight_layout()

plt.savefig(OUTPUT_DIR / 'fig3_pdr_vs_distance.pdf', dpi=300, bbox_inches='tight')
plt.savefig(OUTPUT_DIR / 'fig3_pdr_vs_distance.png', dpi=300, bbox_inches='tight')
plt.close()
print("  Saved fig3_pdr_vs_distance")


# ---------------------------------------------------------------------------
# Figure 3: End-to-end latency boxplot
# ---------------------------------------------------------------------------
print("Generating fig4_latency ...")

fig, ax = plt.subplots(figsize=FIG_SIZE)

data = [ecdsa_rx['delay_ms'].dropna().values,
        falcon_rx['delay_ms'].dropna().values]

bp = ax.boxplot(data,
                tick_labels=['ECDSA\nP-256', 'Falcon-512'],
                patch_artist=True,
                medianprops=dict(color='black', linewidth=1.8),
                whiskerprops=dict(linewidth=1.0),
                capprops=dict(linewidth=1.0),
                flierprops=dict(marker='.', markersize=2, alpha=0.4))

bp['boxes'][0].set_facecolor(COLOR_ECDSA)
bp['boxes'][0].set_alpha(0.85)
bp['boxes'][1].set_facecolor(COLOR_FALCON)
bp['boxes'][1].set_alpha(0.85)

ax.set_ylabel('Latency (ms)', fontweight='bold')
ax.grid(axis='y', alpha=0.3, linestyle='--')
ax.set_axisbelow(True)
plt.tight_layout()

plt.savefig(OUTPUT_DIR / 'fig4_latency.pdf', dpi=300, bbox_inches='tight')
plt.savefig(OUTPUT_DIR / 'fig4_latency.png', dpi=300, bbox_inches='tight')
plt.close()
print("  Saved fig4_latency")

print(f"\nDone. All figures in: {OUTPUT_DIR}/")
