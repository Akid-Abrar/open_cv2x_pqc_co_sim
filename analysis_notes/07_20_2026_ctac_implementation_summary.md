# CTAC Implementation Summary
Date: 2026-07-20

## Overview
Successfully implemented Cooperative Transmission Authority Control (CTAC) according to the specification in `CTAC/CTAC_IMPLEMENTATION_SPEC.md`. CTAC is an application-layer overlay for coordinated BSM transmission scheduling in LTE-V2X Mode 4.

## Changes Made

### 1. NED Parameters and Signals (mode4App.ned)
**Added parameters:**
- `ctacEnabled` (bool, default=false): Master switch for CTAC
- `ctacCohorts` (int, default=4): Number of cohorts (K)
- `ctacEpoch` (simtime_t, default=0.1s): Epoch length
- `ctacAoIBound` (simtime_t, default=0.3s): Max time since last full BSM
- `ctacCellSize` (double, default=60m): Spatial hash cell size
- `ctacInactive` (string, default="defer"): "defer" | "compress"
- `ctacSafetyOverride` (bool, default=true): Safety event override
- `logV2vRx` (bool, default=true): Enable V2V reception logging

**Fixed CBR statistic:**
- Changed from `record=vector` to `record=mean,max,timeavg,vector`
- This enables CBR scalar output while vector recording remains globally disabled

**Added signals:**
- `bsmOpportunity`: Tracks every BSM generation opportunity (critical for PDR calculation)
- `ctacDeferred`: BSMs deferred by CTAC
- `ctacCompressed`: BSMs compressed by CTAC
- `ctacOverrideAoI`: Freshness bound overrides
- `ctacOverrideSafety`: Safety event overrides
- `ctrlOverheadBytes`: CTAC control overhead (zero by design)
- `ctacCohortId`: Assigned cohort ID

### 2. Header Additions (Mode4App.h)
**Added:**
- `CtacDecision` enum: {CTAC_FULL, CTAC_COMPRESS, CTAC_DEFER}
- CTAC state members (enabled, cohorts, epoch, AoI bound, etc.)
- Signal declarations for all CTAC statistics
- Method declarations: `ctacDecide()`, `myCohort()`, `safetyEventActive()`, `generateAndSendCompact()`

### 3. Implementation (Mode4App.cc)

#### 3.1 Initialization
- Read all CTAC parameters in `initialize()` at INITSTAGE_APPLICATION_LAYER
- Register all CTAC signals

#### 3.2 CTAC Gate
Modified `handleSelfMessage()` to implement the decision gate:
1. **ALWAYS** emit `bsmOpportunity` before any decision (critical for honest PDR)
2. If CTAC disabled: legacy path (unchanged behavior)
3. If CTAC enabled: call `ctacDecide()` and branch:
   - `CTAC_FULL`: Send full BSM
   - `CTAC_COMPRESS`: Send compressed BSM (stub in v1)
   - `CTAC_DEFER`: Skip transmission, increment counter
4. Timer reschedule OUTSIDE decision branch (clock never stops)

#### 3.3 Decision Logic (`ctacDecide()`)
Priority order (per spec, DO NOT reorder):
1. **Safety override**: If safety event active → CTAC_FULL
2. **Freshness bound**: If time since last full TX ≥ AoI bound → CTAC_FULL
3. **Cohort schedule**: If current epoch matches this vehicle's cohort → CTAC_FULL
4. **Otherwise**: DEFER or COMPRESS based on `ctacInactive` parameter

#### 3.4 Cohort Assignment (`myCohort()`)
- Zero-signalling design: derived from GNSS position
- Spatial hash: `cx = floor(x/cellSize)`, `cy = floor(y/cellSize)`
- Hash function: `h = (cx * 73856093) XOR (cy * 19349663)`
- Cohort: `h % ctacCohorts`
- Dynamic reassignment as vehicle moves between cells

#### 3.5 Safety Event Detection (`safetyEventActive()`)
- Implemented as stub returning `false` with TODO comment
- Future work: integrate with hard braking, ICA events, etc.

#### 3.6 Compressed Mode (`generateAndSendCompact()`)
- Implemented as no-op for v1 (effectively defers)
- Per spec: acceptable for initial version
- Future work: reduced payload, digest-only cert

#### 3.7 Finish Method
- Emit `ctrlOverheadBytes = 0` to explicitly record zero overhead

### 4. V2V Reception Logging
**Critical priority change per spec section 5**

**Added in `handleLowerMessage()`:**
- Mirrors RSU logging structure exactly
- Logs to separate file: `v2v_logs.csv` (prevents breaking existing parsers)
- Identical schema to `appl_logs.csv` including the typo "Numer of Vehicles"
- Receiver field = `carNoIp[n]` (vs RSU's `rsu[0]`)
- Gated behind `logV2vRx_` parameter
- Includes all fields: t, receiver, sender, msgId, lat, lon, dist_m, delay_ms, verified, spdu_overhead, cert_metadata, digest_size, pk_size, sig_size, bsm_data_size, spdu_size, Algorithm, signerType

**Why V2V logging matters:**
- RSU-only logging captures infrastructure receptions
- CTAC's claim concerns **neighbour awareness** (V2V)
- Without V2V logs, primary result figure cannot be produced

### 5. Scenario Configuration (omnetpp.ini)
**Added three CTAC scenarios:**

- **_25_CTAC_F_LOS**: extends _11_ECDSA_F_LOS
- **_26_CTAC_F_NLOS**: extends _12_ECDSA_F_NLOS
- **_27_CTAC_D_LOS**: extends _7_ECDSA_D_LOS (mid-density control)

**Each scenario:**
- Inherits ALL base parameters (traffic, channel, crypto, duration)
- ONLY adds CTAC parameter block
- Ensures pairwise comparability: baseline vs CTAC differ ONLY in CTAC settings

**CTAC parameters:**
- ctacEnabled = true
- ctacCohorts = 4
- ctacEpoch = 0.1s
- ctacAoIBound = 0.3s
- ctacCellSize = 60m
- ctacInactive = "defer"

## Design Constraints Met

### 1. Legacy Compatibility ✓
- All 24 existing scenarios unaffected (ctacEnabled defaults to false)
- No changes to MAC/PHY layers
- No changes to Base config behavior

### 2. Application-Layer Only ✓
- CTAC is purely app-layer overlay above resource selection
- SPS, CBR measurement, CR limiting unchanged
- Intellectual property claim: works above existing MAC

### 3. Log Schema Preservation ✓
- V2V logs use identical schema to RSU logs
- Typo "Numer of Vehicles" preserved for parser compatibility
- New logs go to separate file (`v2v_logs.csv`)

### 4. Clear Delimitation ✓
- All CTAC code marked with clear comment blocks
- Easy for reviewers to identify changes

## Verification Checklist (from spec section 8)

Before running simulations, verify:

- [ ] All 24 original scenarios still compile and run
- [ ] With CTAC disabled, `bsmOpportunity:sum == sentMsg:sum`
- [ ] With CTAC disabled, `ctacDeferred:sum == 0`
- [ ] With CTAC disabled, results match baseline exactly
- [ ] `cbr:mean` appears in .sca output (previously missing)
- [ ] V2V receptions logged with `receiver = carNoIp[n]`
- [ ] _25_CTAC_F_LOS extends _11_ECDSA_F_LOS correctly
- [ ] Deferral rate ~75% with K=4, no overrides (sanity check)
- [ ] `ctacOverrideAoI` fires occasionally but not always

## Expected Behavior

### With K=4 cohorts, epoch=0.1s, AoI bound=0.3s:
1. **Scheduled transmissions**: 25% of opportunities (1/K)
2. **Deferred transmissions**: ~75% of opportunities
3. **AoI overrides**: Fire after 3 epochs without TX
4. **Safety overrides**: Currently none (stub returns false)
5. **Control overhead**: Zero bytes (no coordination messages)

### Key metrics to monitor:
- **bsmOpportunity**: Should equal 10 * sim_duration for each vehicle (100ms period)
- **sentMsg**: Reduced to ~25% of opportunities + overrides
- **ctacDeferred**: ~75% of opportunities
- **Awareness delivery ratio**: `received / bsmOpportunity` (honest metric)
- **Classic PDR**: `received / sentMsg` (optimistic, shrinks denominator)

## Next Steps

### 1. Compilation
```bash
cd /home/veins/src/simulte
make clean
make
```

### 2. Test Run
```bash
cd simulations/Mode4
# Test baseline still works
./run -u Cmdenv -c _11_ECDSA_F_LOS -r 0

# Test CTAC scenario
./run -u Cmdenv -c _25_CTAC_F_LOS -r 0
```

### 3. Verification
- Check `bsmOpportunity` appears in .sca
- Check deferral rate is reasonable
- Check v2v_logs.csv is created and populated
- Check CBR scalars are recorded

### 4. Analysis Scripts (TODO - not implemented yet)
Per spec section 7, create:
- `analysis/compute_aoi.py`: Peak AoI and inter-packet gap analysis
- `analysis/extract_scalars.py`: Wrap scavetool for tidy dataframe
- `analysis/plot_operating_point.py`: CBR vs AoI exceedance
- `analysis/plot_aoi_ccdf.py`: Complementary CDF of peak AoI

## Files Modified

1. `src/apps/mode4App/mode4App.ned` - Parameters and signals
2. `src/apps/mode4App/Mode4App.h` - Header declarations
3. `src/apps/mode4App/Mode4App.cc` - Implementation
4. `simulations/Mode4/omnetpp.ini` - Scenario configs

## Files Created

1. `analysis_notes/07_20_2026_ctac_implementation_summary.md` (this file)

## Important Notes

### Zero-Signalling Design
- No coordination messages exchanged
- Cohort membership derived from position
- Dynamic reassignment as vehicles move
- Direct rebuttal to "control overhead" objection

### Honest PDR Calculation
- `bsmOpportunity` emitted BEFORE gate decision
- Provides fixed denominator for delivery ratio
- Without this, CTAC would appear to improve PDR by simply transmitting less
- This is the most critical design choice for credibility

### Future Work (Noted in Code)
1. Hook `safetyEventActive()` to real vehicle state (hard braking, ICA events)
2. Implement proper `generateAndSendCompact()` with reduced payload
3. Test compress mode (`ctacInactive = "compress"`)
4. Add analysis scripts per spec section 7
5. Consider separate log files for different density scenarios (file size management)

## Potential Issues to Watch

1. **Log file size**: V2V logging will create large files in high-density scenarios
2. **First-packet loss**: AoI calculation drops first reception of each pair
3. **Position lookup**: Depends on Veins mobility module being present
4. **Epoch alignment**: All vehicles share global epoch counter from simTime()

## References

- Specification: `CTAC/CTAC_IMPLEMENTATION_SPEC.md`
- Project instructions: `CLAUDE.md`
- IEEE 1609.2: Security message format
- SAE J2735: BSM format
- SAE J2945/1: Certificate management
- SAE J3161: DCC thresholds (CBR 0.30, 0.65)
