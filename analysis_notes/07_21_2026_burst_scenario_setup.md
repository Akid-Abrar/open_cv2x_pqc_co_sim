# Burst Scenario Setup for CTAC Testing
Date: 2026-07-21

## Objective
Create immediate congestion scenarios for faster CTAC evaluation with identical traffic conditions for fair baseline vs CTAC comparison.

## Problem with Original Scenarios
- Original LOS_F scenarios spawn vehicles over **400 seconds**
- Congestion builds gradually
- Long simulation time needed to see CTAC benefits
- CBR takes time to reach saturation levels (>0.65)

## Solution: Burst Spawning

### Traffic Compression
**Original F scenario:**
- Spawn window: 0-400 seconds
- Flow rate: 5792 veh/h (straight), 724 veh/h (turns)
- Total vehicles per flow over 400s: ~643 vehicles

**Burst F scenario:**
- Spawn window: 0-30 seconds (13.33x compression)
- Flow rate: 77,227 veh/h (straight), 9,653 veh/h (turns)
- Total vehicles: **SAME as original** (~643 per flow)
- Result: Immediate saturation, CBR >0.65 by t=10s

### Files Created

1. **intersection_LOS_F_BURST.rou.xml**
   - SUMO route file with compressed spawn window
   - Flow rates multiplied by 13.33x (400/30)
   - Same total vehicle count as original F

2. **intersection_LOS_F_BURST.launchd.xml**
   - Veins launch config pointing to burst route file
   - Reuses existing intersection.net.xml and sumocfg

3. **Updated omnetpp.ini**
   - Removed old scenarios: _25_CTAC_F_LOS, _26_CTAC_F_NLOS, _27_CTAC_D_LOS
   - Added new scenarios:
     - **_25_ECDSA_F_BURST**: Baseline without CTAC
     - **_26_CTAC_F_BURST**: CTAC enabled
   - Both extend Base and use identical traffic (BURST route file)
   - Shorter sim-time: 90s (vs 120s in original)

## Scenario Comparison

### _25_ECDSA_F_BURST (Baseline)
```ini
extends = Base
sim-time-limit = 90s
*.carNoIp[*].appl.cryptoAlgo = "ecdsa"
*.manager.launchConfig = xmldoc("intersections/intersection_LOS_F_BURST.launchd.xml")
```

### _26_CTAC_F_BURST (Test)
```ini
extends = _25_ECDSA_F_BURST
*.carNoIp[*].appl.ctacEnabled   = true
*.carNoIp[*].appl.ctacCohorts   = 4
*.carNoIp[*].appl.ctacEpoch     = 0.1s
*.carNoIp[*].appl.ctacAoIBound  = 0.3s
*.carNoIp[*].appl.ctacCellSize  = 60m
*.carNoIp[*].appl.ctacInactive  = "defer"
```

**Critical:** Both scenarios use **IDENTICAL** traffic (same .launchd.xml). The ONLY difference is CTAC parameters.

## Expected Timeline

### Burst Scenario (_25/_26)
- **t=0-30s**: Rapid vehicle insertion (~800 vehicles total)
- **t=10s**: CBR reaches saturation (>0.65)
- **t=30s**: All vehicles spawned, full congestion active
- **t=30-90s**: Sustained congestion period for data collection

### Original Scenario (_11_ECDSA_F_LOS)
- **t=0-60s**: Gradual vehicle insertion
- **t=60s**: Congestion starts building
- **t=120s**: Peak congestion
- Requires full 120s to see effects

## Benefits of Burst Scenario

1. **Faster iteration**: 90s vs 120s simulation time
2. **Immediate results**: CBR >0.65 by t=10s
3. **More data**: 60 seconds of full congestion (t=30-90) vs ~30s in original
4. **Fair comparison**: Identical traffic, only CTAC differs
5. **Easier debugging**: Congestion effects visible immediately

## Running the Scenarios

```bash
cd simulations/Mode4

# Baseline (ECDSA, no CTAC)
./run -u Cmdenv -c _25_ECDSA_F_BURST

# CTAC test
./run -u Cmdenv -c _26_CTAC_F_BURST
```

## Expected Results

### Baseline (_25_ECDSA_F_BURST)
- High CBR (>0.70 expected)
- Lower PDR due to congestion
- High collision rate
- Packet drops from CR limiting

### CTAC (_26_CTAC_F_BURST)
- Lower CBR (~75% reduction in TX opportunities)
- **Honest awareness PDR**: received / bsmOpportunity
- **Optimistic TX PDR**: received / sentMsg (higher, but misleading)
- Lower channel load
- Expected deferral rate: ~75% (K=4 cohorts)

## Key Metrics to Compare

### Channel Load
- `cbr:mean` - Average CBR across all vehicles
- `cbr:max` - Peak CBR seen

### Transmission Behavior
- `sentMsg:sum` - Actual BSMs transmitted
- `bsmOpportunity:sum` - Total BSM generation opportunities
- `ctacDeferred:sum` - Opportunities deferred by CTAC

### Delivery Performance
- `received:sum` - Total BSMs received by all vehicles
- **Awareness PDR** = received / bsmOpportunity (honest metric)
- **Classic PDR** = received / sentMsg (optimistic, misleading for CTAC)

### CTAC-Specific
- `ctacOverrideAoI:sum` - Freshness bound overrides
- `ctacOverrideSafety:sum` - Safety event overrides (0 in v1)
- `ctrlOverheadBytes:sum` - Control overhead (0 by design)

## Validation Checklist

Before trusting results:

- [ ] Both scenarios spawn ~800 vehicles
- [ ] Both reach CBR >0.65 by t=30s
- [ ] ECDSA: bsmOpportunity == sentMsg
- [ ] CTAC: sentMsg < bsmOpportunity (deferred transmissions)
- [ ] CTAC: deferral rate ~75% (with K=4, no overrides)
- [ ] V2V logs created (v2v_logs.csv in simulation_logs_*)
- [ ] CBR scalars appear in .sca files

## Analysis Scripts Needed

1. **compare_scenarios.py**
   - Load scalars from both .sca files
   - Compute PDR both ways (awareness vs classic)
   - Plot CBR comparison
   - Show channel load reduction

2. **plot_cbr_timeseries.py**
   - If CBR vectors enabled
   - Show CBR over time for both scenarios
   - Highlight onset of congestion

3. **aoi_analysis.py**
   - Per CTAC spec section 7a
   - Parse v2v_logs.csv
   - Compute peak AoI distribution
   - Compare ECDSA vs CTAC

## Important Notes

### Why 90s instead of 120s?
- Vehicles spawn in first 30s
- 60s of sustained congestion sufficient for statistics
- Faster iteration during testing
- Can extend to 120s later if needed

### Why ECDSA instead of Falcon?
- Smaller signature size (64B vs 666B)
- Less crypto overhead, isolates CTAC effect
- CTAC benefits should be even larger with Falcon (future test)

### Fair Comparison Guarantee
Both scenarios:
- ✓ Same Base config
- ✓ Same traffic file (BURST.launchd.xml)
- ✓ Same channel model (LOS)
- ✓ Same crypto algorithm (ECDSA)
- ✓ Same sim duration (90s)
- ✓ Same subchannel config
- ✗ ONLY DIFFERENCE: CTAC enabled vs disabled

## Next Steps

1. **Compile**: Ensure clean build
   ```bash
   make clean && make
   ```

2. **Quick test**: Run single iteration
   ```bash
   ./run -u Cmdenv -c _25_ECDSA_F_BURST -r 0
   ./run -u Cmdenv -c _26_CTAC_F_BURST -r 0
   ```

3. **Check outputs**:
   - simulation_logs_*/ directories created
   - v2v_logs.csv populated
   - .sca files contain cbr:mean

4. **Full experiment**: Add repeat parameter
   ```ini
   [Config _25_ECDSA_F_BURST]
   repeat = 10

   [Config _26_CTAC_F_BURST]
   repeat = 10
   ```

5. **Analyze**: Create comparison scripts

## Troubleshooting

**If SUMO fails with "too many vehicles":**
- Reduce flow rates or extend spawn window to 45s
- SUMO has limits on insertion rate

**If CBR doesn't reach 0.65:**
- Check vehicle count (should be ~800)
- Verify routes are loading correctly
- May need higher flow rates

**If deferral rate is wrong:**
- Check ctacDecide() logic
- Verify epoch calculation
- Ensure cohort assignment working

**If V2V logs empty:**
- Check logV2vRx_ parameter (should default to true)
- Verify handleLowerMessage() logging code
- Ensure vehicles are receiving messages

## Files Modified/Created

### Created
1. `simulations/Mode4/intersections/intersection_LOS_F_BURST.rou.xml`
2. `simulations/Mode4/intersections/intersection_LOS_F_BURST.launchd.xml`
3. `analysis_notes/07_21_2026_burst_scenario_setup.md` (this file)

### Modified
1. `simulations/Mode4/omnetpp.ini` - Replaced scenarios 25-27

### Removed
- Old scenarios: _25_CTAC_F_LOS, _26_CTAC_F_NLOS, _27_CTAC_D_LOS
- (They were gradual spawn, not suitable for quick testing)
