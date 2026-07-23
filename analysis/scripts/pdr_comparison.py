#!/usr/bin/env python3
"""
V2V and V2I PDR Comparison across Baseline, CTAC K=2, CTAC K=4.

V2V PDR per vehicle (as receiver):
  For each (sender, receiver) link within 300m:
      link_PDR = packets_received / BSMs_generated_by_sender
  Per-vehicle PDR = mean of link_PDR across all senders to that vehicle

V2I PDR per vehicle (as sender):
  PDR(v) = packets_received_by_RSU / BSMs_generated_by_vehicle

Usage:
    python3 pdr_comparison.py
"""

import sys
import pandas as pd
import numpy as np
from pathlib import Path

SCENARIOS = {
    'Baseline': Path('/home/veins/src/simulte/simulations/Mode4/res_base_3'),
    'CTAC K=2': Path('/home/veins/src/simulte/simulations/Mode4/res_ctac_3(k2)'),
    'CTAC K=4': Path('/home/veins/src/simulte/simulations/Mode4/res_ctac_3'),
}

V2V_MAX_DIST = 300  # meters


def get_bsms_generated(folder):
    """Return dict: vehicle -> BSMs actually generated (opportunities - deferred)."""
    sender_df = pd.read_csv(folder / 'sender_summary.csv')
    sca_file = list(folder.glob('*.sca'))[0]

    deferred = {}
    with open(sca_file) as f:
        for line in f:
            if 'ctacDeferred:sum' in line and 'carNoIp[' in line:
                parts = line.strip().split()
                for p in parts[1].split('.'):
                    if 'carNoIp[' in p:
                        try:
                            deferred[p] = int(parts[3])
                        except:
                            pass
                        break

    generated = {}
    for _, row in sender_df.iterrows():
        v = row['sender']
        generated[v] = row['total_sent'] - deferred.get(v, 0)

    return generated


def compute_v2v(folder):
    """
    Compute V2V PDR: per-vehicle and overall.

    Algorithm:
      1. For each (sender, receiver) link within 300m:
           link_PDR = packets_received / BSMs_generated_by_sender
      2. Per-vehicle PDR(v) = mean(link_PDR) across all senders to v
      3. Overall PDR = total_received / sum(BSMs_generated per sender per link)
    """
    generated = get_bsms_generated(folder)
    v2v_df = pd.read_csv(folder / 'v2v_logs.csv')

    total_records = len(v2v_df)
    v2v_df = v2v_df[v2v_df['dist_m'] <= V2V_MAX_DIST]
    filtered_records = len(v2v_df)
    max_dist_in_data = v2v_df['dist_m'].max() if len(v2v_df) > 0 else 0

    # Per-link counts
    link_rx = v2v_df.groupby(['sender', 'receiver']).size().reset_index(name='received')
    link_rx['sent'] = link_rx['sender'].map(generated)
    link_rx = link_rx[link_rx['sent'] > 0]
    link_rx['link_pdr'] = link_rx['received'] / link_rx['sent']

    # Per-vehicle (receiver) PDR
    vehicle_pdr = link_rx.groupby('receiver')['link_pdr'].mean()

    # Overall PDR across all links
    overall_received = link_rx['received'].sum()
    overall_expected = link_rx['sent'].sum()
    overall_pdr = (overall_received / overall_expected * 100) if overall_expected > 0 else 0

    return {
        'vehicle_pdr': vehicle_pdr,
        'n_vehicles': len(vehicle_pdr),
        'n_links': len(link_rx),
        'total_records': total_records,
        'filtered_records': filtered_records,
        'max_dist': max_dist_in_data,
        'overall_received': overall_received,
        'overall_expected': overall_expected,
        'overall_pdr': overall_pdr,
    }


def compute_v2i(folder):
    """
    Compute V2I PDR: per-vehicle and overall.

    Algorithm:
      1. BSMs_generated(v) = sender_summary - ctacDeferred
      2. received_by_RSU(v) = count of packets from v received by rsu[0]
      3. PDR(v) = received_by_RSU(v) / BSMs_generated(v)
      4. Overall PDR = total_received_by_RSU / total_BSMs_generated
    """
    generated = get_bsms_generated(folder)
    appl_df = pd.read_csv(folder / 'appl_logs.csv')
    v2i = appl_df[appl_df['receiver'] == 'rsu[0]']
    received = v2i.groupby('sender').size().to_dict()

    # Distance stats
    max_dist = v2i['dist_m'].max() if len(v2i) > 0 else 0
    mean_dist = v2i['dist_m'].mean() if len(v2i) > 0 else 0

    pdr_dict = {}
    total_received = 0
    total_generated = 0
    for v, gen in generated.items():
        if gen > 0:
            rcv = received.get(v, 0)
            pdr_dict[v] = rcv / gen
            total_received += rcv
            total_generated += gen

    overall_pdr = (total_received / total_generated * 100) if total_generated > 0 else 0

    return {
        'vehicle_pdr': pd.Series(pdr_dict),
        'n_vehicles': len(pdr_dict),
        'total_received': total_received,
        'total_generated': total_generated,
        'overall_pdr': overall_pdr,
        'max_dist': max_dist,
        'mean_dist': mean_dist,
    }


def pdr_stats(series):
    """Return min, mean, median, p95, max."""
    return {
        'min':    series.min() * 100,
        'mean':   series.mean() * 100,
        'median': series.median() * 100,
        'p95':    series.quantile(0.95) * 100,
        'max':    series.max() * 100,
    }


def main():
    out_path = Path('/home/veins/src/simulte/analysis/current_results/pdr_v2v_v2i_comparison.txt')
    out = open(out_path, 'w')

    def p(s=''):
        print(s)
        out.write(s + '\n')

    names = list(SCENARIOS.keys())

    # Compute everything
    v2v = {}
    v2i = {}
    for name, folder in SCENARIOS.items():
        v2v[name] = compute_v2v(folder)
        v2i[name] = compute_v2i(folder)

    # ──────────────── V2V TABLE ────────────────
    p("=" * 80)
    p("TABLE 1: V2V PDR (GRIDLOCK, 352 VEHICLES)")
    p("=" * 80)
    p()
    p("Distance Filter: <= 300m")
    p("  Rationale: 300m is the standard safety-critical communication")
    p("  range for C-V2X Mode 4 sidelink per 3GPP TR 36.885 and")
    p("  5GAA evaluation methodology (P-190033 Section 5.2.4).")
    p("  Beyond 300m, V2X safety messages are not expected to be")
    p("  reliably decoded, and links are excluded from PDR evaluation.")
    p()
    p("Calculation:")
    p("  1. For each (sender, receiver) link within 300m:")
    p("       link_PDR = packets_received / BSMs_generated_by_sender")
    p("  2. Per-vehicle PDR = mean(link_PDR) across all senders to that vehicle")
    p("  3. Overall PDR = total_received / total_expected across all links")
    p()

    p(f"{'Metric':<25} {'Baseline':>15} {'CTAC K=2':>15} {'CTAC K=4':>15}")
    p("-" * 72)
    p(f"{'Vehicles':<25} {v2v[names[0]]['n_vehicles']:>15} {v2v[names[1]]['n_vehicles']:>15} {v2v[names[2]]['n_vehicles']:>15}")
    p(f"{'Active Links (<= 300m)':<25} {v2v[names[0]]['n_links']:>15,} {v2v[names[1]]['n_links']:>15,} {v2v[names[2]]['n_links']:>15,}")
    p(f"{'Total V2V Receptions':<25} {v2v[names[0]]['filtered_records']:>15,} {v2v[names[1]]['filtered_records']:>15,} {v2v[names[2]]['filtered_records']:>15,}")
    p(f"{'Max Distance in Data':<25} {v2v[names[0]]['max_dist']:>14.1f}m {v2v[names[1]]['max_dist']:>14.1f}m {v2v[names[2]]['max_dist']:>14.1f}m")
    p("-" * 72)

    # Overall PDR
    vals = [v2v[n]['overall_pdr'] for n in names]
    p(f"{'OVERALL PDR':<25} {vals[0]:>14.2f}% {vals[1]:>14.2f}% {vals[2]:>14.2f}%")
    p("-" * 72)

    # Per-vehicle stats
    for name_s in names:
        v2v[name_s]['stats'] = pdr_stats(v2v[name_s]['vehicle_pdr'])

    for label, key in [('Min', 'min'), ('Mean', 'mean'), ('Median', 'median'),
                        ('P95', 'p95'), ('Max', 'max')]:
        vals = [v2v[n]['stats'][key] for n in names]
        p(f"{label + ' PDR':<25} {vals[0]:>14.2f}% {vals[1]:>14.2f}% {vals[2]:>14.2f}%")

    p("-" * 72)
    base_overall = v2v[names[0]]['overall_pdr']
    k2_overall = v2v[names[1]]['overall_pdr']
    k4_overall = v2v[names[2]]['overall_pdr']
    p(f"{'Overall Improvement':<25} {'---':>15} {f'+{k2_overall-base_overall:.2f}%':>15} {f'+{k4_overall-base_overall:.2f}%':>15}")
    for label, key in [('Mean', 'mean'), ('Median', 'median')]:
        base = v2v[names[0]]['stats'][key]
        k2 = v2v[names[1]]['stats'][key]
        k4 = v2v[names[2]]['stats'][key]
        p(f"{label + ' Improvement':<25} {'---':>15} {f'+{k2-base:.2f}%':>15} {f'+{k4-base:.2f}%':>15}")

    p()

    # ──────────────── V2I TABLE ────────────────
    p("=" * 80)
    p("TABLE 2: V2I PDR (GRIDLOCK, 352 VEHICLES)")
    p("=" * 80)
    p()
    p("Distance Filter: None (all distances included)")
    p("  Rationale: V2I uses a single RSU (rsu[0]) at the intersection")
    p("  center. All vehicles that can reach the RSU are included.")
    p("  The RSU has no distance cutoff in the simulation; packets")
    p("  are received or lost based on PHY-layer propagation and")
    p("  interference conditions.")
    p()
    p("Calculation:")
    p("  1. BSMs_generated(v) = sender_summary - ctacDeferred")
    p("  2. received_by_RSU(v) = count of packets from v received by rsu[0]")
    p("  3. PDR(v) = received_by_RSU(v) / BSMs_generated(v)")
    p("  4. Overall PDR = total_received_by_RSU / total_BSMs_generated")
    p()

    p(f"{'Metric':<25} {'Baseline':>15} {'CTAC K=2':>15} {'CTAC K=4':>15}")
    p("-" * 72)
    p(f"{'Vehicles':<25} {v2i[names[0]]['n_vehicles']:>15} {v2i[names[1]]['n_vehicles']:>15} {v2i[names[2]]['n_vehicles']:>15}")
    p(f"{'Total BSMs Generated':<25} {v2i[names[0]]['total_generated']:>15,} {v2i[names[1]]['total_generated']:>15,} {v2i[names[2]]['total_generated']:>15,}")
    p(f"{'Received by RSU':<25} {v2i[names[0]]['total_received']:>15,} {v2i[names[1]]['total_received']:>15,} {v2i[names[2]]['total_received']:>15,}")
    p(f"{'Max Distance in Data':<25} {v2i[names[0]]['max_dist']:>14.1f}m {v2i[names[1]]['max_dist']:>14.1f}m {v2i[names[2]]['max_dist']:>14.1f}m")
    p(f"{'Mean Distance to RSU':<25} {v2i[names[0]]['mean_dist']:>14.1f}m {v2i[names[1]]['mean_dist']:>14.1f}m {v2i[names[2]]['mean_dist']:>14.1f}m")
    p("-" * 72)

    # Overall PDR
    vals = [v2i[n]['overall_pdr'] for n in names]
    p(f"{'OVERALL PDR':<25} {vals[0]:>14.2f}% {vals[1]:>14.2f}% {vals[2]:>14.2f}%")
    p("-" * 72)

    # Per-vehicle stats
    for name_s in names:
        v2i[name_s]['stats'] = pdr_stats(v2i[name_s]['vehicle_pdr'])

    for label, key in [('Min', 'min'), ('Mean', 'mean'), ('Median', 'median'),
                        ('P95', 'p95'), ('Max', 'max')]:
        vals = [v2i[n]['stats'][key] for n in names]
        p(f"{label + ' PDR':<25} {vals[0]:>14.2f}% {vals[1]:>14.2f}% {vals[2]:>14.2f}%")

    p("-" * 72)
    base_overall = v2i[names[0]]['overall_pdr']
    k2_overall = v2i[names[1]]['overall_pdr']
    k4_overall = v2i[names[2]]['overall_pdr']
    p(f"{'Overall Improvement':<25} {'---':>15} {f'+{k2_overall-base_overall:.2f}%':>15} {f'+{k4_overall-base_overall:.2f}%':>15}")
    for label, key in [('Mean', 'mean'), ('Median', 'median')]:
        base = v2i[names[0]]['stats'][key]
        k2 = v2i[names[1]]['stats'][key]
        k4 = v2i[names[2]]['stats'][key]
        p(f"{label + ' Improvement':<25} {'---':>15} {f'+{k2-base:.2f}%':>15} {f'+{k4-base:.2f}%':>15}")

    p()
    p("=" * 80)

    out.close()
    print(f"\nSaved to: {out_path}")


if __name__ == "__main__":
    main()
