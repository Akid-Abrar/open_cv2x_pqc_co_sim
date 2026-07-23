#!/usr/bin/env python3
"""
Compare interference statistics between baseline and CTAC
"""

import re
from pathlib import Path

def extract_interference_stats(sca_file):
    """Extract interference failure statistics and transmission counts from .sca file."""

    stats = {
        'tbFailedDueToInterference': 0,
        'sciFailedDueToInterference': 0,
        'tbSent': 0,
        'sciSent': 0
    }

    with open(sca_file, 'r') as f:
        for line in f:
            # Look for: scalar <module> <metric>:sum <value>
            if 'tbFailedDueToInterference:sum' in line and 'IgnoreSCI' not in line:
                parts = line.strip().split()
                if len(parts) >= 4:
                    value = int(parts[3])
                    stats['tbFailedDueToInterference'] += value

            elif 'sciFailedDueToInterference:sum' in line:
                parts = line.strip().split()
                if len(parts) >= 4:
                    value = int(parts[3])
                    stats['sciFailedDueToInterference'] += value

            elif 'tbSent:sum' in line:
                parts = line.strip().split()
                if len(parts) >= 4:
                    value = int(parts[3])
                    stats['tbSent'] += value

            elif 'sciSent:sum' in line:
                parts = line.strip().split()
                if len(parts) >= 4:
                    value = int(parts[3])
                    stats['sciSent'] += value

    return stats

def main():
    base_dir = Path("../simulations/Mode4/results")

    baseline_sca = base_dir / "_27_ECDSA_F_BURST_DCC-#0.sca"
    ctac_sca = base_dir / "_26_CTAC_F_BURST-#0.sca"

    print("="*80)
    print("INTERFERENCE AND COLLISION STATISTICS")
    print("="*80)

    baseline_stats = extract_interference_stats(baseline_sca)
    ctac_stats = extract_interference_stats(ctac_sca)

    # Calculate percentages
    baseline_tb_pct = (baseline_stats['tbFailedDueToInterference'] / baseline_stats['tbSent'] * 100
                       if baseline_stats['tbSent'] > 0 else 0)
    baseline_sci_pct = (baseline_stats['sciFailedDueToInterference'] / baseline_stats['sciSent'] * 100
                        if baseline_stats['sciSent'] > 0 else 0)

    ctac_tb_pct = (ctac_stats['tbFailedDueToInterference'] / ctac_stats['tbSent'] * 100
                   if ctac_stats['tbSent'] > 0 else 0)
    ctac_sci_pct = (ctac_stats['sciFailedDueToInterference'] / ctac_stats['sciSent'] * 100
                    if ctac_stats['sciSent'] > 0 else 0)

    print("\nBASELINE (DCC):")
    print("-" * 80)
    print(f"  TB sent:                           {baseline_stats['tbSent']:>10,}")
    print(f"  TB failed due to interference:     {baseline_stats['tbFailedDueToInterference']:>10,}  ({baseline_tb_pct:.2f}%)")
    print(f"  SCI sent:                          {baseline_stats['sciSent']:>10,}")
    print(f"  SCI failed due to interference:    {baseline_stats['sciFailedDueToInterference']:>10,}  ({baseline_sci_pct:.2f}%)")

    print("\nCTAC:")
    print("-" * 80)
    print(f"  TB sent:                           {ctac_stats['tbSent']:>10,}")
    print(f"  TB failed due to interference:     {ctac_stats['tbFailedDueToInterference']:>10,}  ({ctac_tb_pct:.2f}%)")
    print(f"  SCI sent:                          {ctac_stats['sciSent']:>10,}")
    print(f"  SCI failed due to interference:    {ctac_stats['sciFailedDueToInterference']:>10,}  ({ctac_sci_pct:.2f}%)")

    print("\n" + "="*80)
    print("INTERFERENCE FAILURE RATE COMPARISON")
    print("="*80)

    print(f"\nTransport Block failure rate:  {baseline_tb_pct:.2f}% → {ctac_tb_pct:.2f}%")
    print(f"SCI failure rate:              {baseline_sci_pct:.2f}% → {ctac_sci_pct:.2f}%")

    print("\n" + "="*80)

if __name__ == "__main__":
    main()
