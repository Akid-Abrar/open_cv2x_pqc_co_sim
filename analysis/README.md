# Analysis Folder Organization

This folder contains analysis scripts and results for CTAC performance evaluation.

## Folder Structure

### `scripts/`
Python scripts for analyzing simulation results.

**Main Scripts:**
- `compare_mac_drops.py` - MAC layer drop statistics comparison (CORRECTED)
  - Usage: `python3 compare_mac_drops.py <baseline_folder> <ctac_k2_folder> <ctac_k4_folder> [output_file]`
  - Calculates: App BSMs generated, PHY transmissions, actual drops (App-PHY)
  - Accounts for CTAC deferrals correctly

- `compare_two_runs.py` - General performance comparison script
  - Computes: PDR, AoI, CBR, IPG, interference stats
  - Usage: `python3 compare_two_runs.py <baseline_folder> <ctac_folder> <output_file>`

- `compare_ctac_results.py` - CTAC visualization and plotting
  - Creates publication-quality plots for AoI and CBR

- `detailed_metrics.py` - IPG and PDR computation utilities

- `interference_comparison.py` - Interference statistics analysis

- `analyze_drop_causes.py` - Per-vehicle drop cause analysis
  - Usage: `python3 analyze_drop_causes.py <folder_path> <scenario_name>`
  - Breaks down drops by cause (DCC, timeout, unaccounted)
  - Shows drop distribution across vehicles

- `calculate_v2i_pdr_per_link.py` - V2I per-link PDR analysis
  - Usage: `python3 calculate_v2i_pdr_per_link.py <scenario_folder> [output_csv]`
  - Calculates PDR for each Vehicle↔RSU communication link
  - Two methods: Overall PDR and 5GAA Sliding Window PDR
  - Outputs per-link statistics (packets, distance, delay, PDR)

### `current_results/`
Latest validated analysis results.

**Files:**
- `mac_drops_comparison_CORRECTED.txt` - MAC drop comparison (K=2, K=4 vs Baseline)
  - Shows: 82-84% MAC drop reduction with CTAC
  - Accounts for CTAC transmission gating

- `drop_breakdown_CORRECTED.txt` - Detailed drop breakdown table
  - Clarifies: grantBreak and missedTransmission are NOT packet drops
  - Only packetDropDCC counts actual DCC drops

- `drop_cause_analysis.txt` - Root cause analysis of ~170 drops in CTAC
  - Pattern: EXACTLY 1 packet dropped per vehicle for ~50% of vehicles
  - Hypothesis: Initial transient/startup effect
  - After startup, CTAC achieves 98.7% delivery success

- `comparison_k2_vs_k3.txt` - K=2 vs K=3 cohort comparison
  - Recommends K=2 for better AoI and PDR

## Key Findings

**CTAC Performance (GRIDLOCK Scenario, 352 vehicles):**
- 82-84% fewer MAC drops vs DCC baseline
- 50% transmission gating (13.3K BSMs vs 26.6K opportunities)
- 98.7% delivery success for BSMs that ARE generated
- Eliminates all DCC drops (DCC disabled)

**Drop Calculation (CORRECTED):**
```
Actual Drops = App BSMs Generated - PHY Transmitted
App BSMs Generated = sender_summary - ctacDeferred
```

**Important:**
- grantBreak = SPS grant expirations (triggers reselection, NOT a drop)
- missedTransmission = Missed TX slots (informational, NOT a drop)
- Only packetDropDCC counts actual DCC-triggered drops
