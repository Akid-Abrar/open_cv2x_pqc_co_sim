# V2I Per-Link PDR Analysis

## Overview

The `calculate_v2i_pdr_per_link.py` script analyzes Vehicle-to-Infrastructure (V2I) communication from `appl_logs.csv` files and computes PDR for each individual communication link.

## Features

- **Per-Link Analysis**: Computes PDR for each (vehicle, RSU) pair
- **Bidirectional**: Identifies both V2I (vehicle→RSU) and I2V (RSU→vehicle) links
- **Two PDR Methods**:
  1. **Overall PDR**: `received / expected * 100`
  2. **Sliding Window PDR**: Median PRR using 5GAA 5-second window method
- **Comprehensive Statistics**: Distance, delay, packet counts per link

## Usage

### Basic Usage
```bash
cd /home/veins/src/simulte/analysis

python3 scripts/calculate_v2i_pdr_per_link.py \
    "../Paper Writeup/Simulation Logs/1_ECDSA_A_LOS"
```

### Save Results to CSV
```bash
python3 scripts/calculate_v2i_pdr_per_link.py \
    "../Paper Writeup/Simulation Logs/1_ECDSA_A_LOS" \
    v2i_results_ecdsa_a_los.csv
```

### Batch Process Multiple Scenarios
```bash
# Process all LOS scenarios
for scenario in ../Paper\ Writeup/Simulation\ Logs/*_LOS; do
    name=$(basename "$scenario")
    python3 scripts/calculate_v2i_pdr_per_link.py "$scenario" "v2i_${name}.csv"
done
```

## Output Format

### Console Output
The script prints:
- Total number of V2I and I2V links
- PDR statistics (mean, median, min, max, std dev)
- Distance statistics
- Top 10 best and worst performing links

### CSV Output (if specified)
Columns include:
- `direction`: V2I or I2V
- `rsu`: RSU node ID
- `vehicle`: Vehicle node ID
- `sender`, `receiver`: Link endpoints
- `packets_received`: Number of packets received
- `packets_expected`: Expected packets based on msgId range
- `pdr_overall`: Overall PDR (%)
- `pdr_sliding_window`: 5GAA sliding window PDR (%)
- `mean_distance_m`, `median_distance_m`: Link distance stats
- `mean_delay_ms`, `median_delay_ms`: Packet delay stats
- `simulation_duration_s`: Total time span of communication

## Example Results

From ECDSA A LOS scenario (238 V2I links):
- **Mean PDR**: 82.91%
- **Median PDR**: 83.56%
- **Distance**: 96-1005m (mean: 233m)
- **Best Links**: 100% PDR at 55-172m
- **Worst Links**: 17-68% PDR at 258-664m

**Observation**: PDR strongly correlates with distance. Links beyond ~400m show significant PDR degradation.

## Configuration

Edit the script to adjust:
```python
WINDOW_SEC = 5.0      # Sliding window size (seconds)
MIN_PKTS = 5          # Minimum packets for valid PRR calculation
RSU_PATTERN = 'rsu'   # Pattern to identify RSU nodes
```

## Comparison with PDR vs Distance Script

| Feature | `pdr_vs_distance_los.py` | `calculate_v2i_pdr_per_link.py` |
|---------|-------------------------|--------------------------------|
| Purpose | Aggregate PDR by distance bins | Per-link PDR analysis |
| Granularity | Distance bins (30m) | Individual links |
| Output | Plot (PDF) | CSV + console summary |
| Method | 5GAA sliding window | Both overall + sliding window |
| Use case | Publication figures | Detailed link analysis |

## Applications

1. **Link Quality Analysis**: Identify problematic vehicle-RSU pairs
2. **Coverage Analysis**: Determine effective RSU communication range
3. **Scenario Comparison**: Compare V2I performance across traffic levels
4. **Troubleshooting**: Find specific links with low PDR for investigation
5. **Statistical Analysis**: Export CSV for further processing in R/Excel

## Example: Compare All Traffic Levels

```bash
cd /home/veins/src/simulte/analysis

# Process all ECDSA LOS scenarios
for level in A B C D E F; do
    python3 scripts/calculate_v2i_pdr_per_link.py \
        "../Paper Writeup/Simulation Logs/${level}_ECDSA_${level}_LOS" \
        "current_results/v2i_ecdsa_${level}_los.csv"
done

# Combine results
head -1 current_results/v2i_ecdsa_A_los.csv > current_results/v2i_all_ecdsa_los.csv
tail -n +2 -q current_results/v2i_ecdsa_*.csv >> current_results/v2i_all_ecdsa_los.csv
```

## Notes

- The script automatically detects RSU nodes by looking for 'rsu' in the node ID
- If no V2I links are found, check that RSU is enabled in the scenario
- PDR > 100% can occur due to packet retransmissions (capped at 100%)
- Sliding window PDR may differ from overall PDR due to temporal variations
