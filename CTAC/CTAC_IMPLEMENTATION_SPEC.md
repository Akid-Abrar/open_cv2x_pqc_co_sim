# CTAC Implementation Spec

Implementation instructions for adding **Cooperative Transmission Authority Control (CTAC)**
to the `open_cv2x_pqc_co_sim` OMNeT++ / Veins / SUMO project (LTE-V2X PC5 Mode 4).

Target: produce a new simulation scenario that can be compared against the existing
`_11_ECDSA_F_LOS` baseline, plus the analysis scripts to generate the result figures.

---

## 0. Repo facts the implementation depends on

Verified against the current `main` branch. Confirm these before editing.

| Item | Value |
|---|---|
| App module | `src/apps/mode4App/Mode4App.{h,cc,ned}`, extends `Mode4BaseApp` |
| MAC module | `src/stack/mac/layer/LteMacVUeMode4.{h,cc}` |
| RSU app | `src/apps/mode4App/Mode4RSUApp.{h,cc,ned}` |
| Config file | `simulations/Mode4/omnetpp.ini` (397 lines, `[Config Base]` + 24 scenarios) |
| BSM period | `*.carNoIp[*].appl.sendInterval = 0.1s`, NED param `period = default(0.1s)` |
| Existing app params | `cryptoAlgo`, `packetSize`, `priority`, `duration`, `period`, `certInterval`, `macNodeId` |
| Existing app signals | `sentMsg`, `delay`, `cbr`, `lifetime`, `received`, `verified`, `warnReceived`, `warnVerified`, `warnExpected`, `warnPdrSample`, `warnPdrDistance`, `rxWarnDist`, `icaVerifyMs`, `icaDelayMs`, `signatureTimeMs`, `verifyTimeMs` |
| Recording | `**.scalar-recording = true`, `**.vector-recording = false` |
| Existing timer members | `cMessage *selfSender_`, `simtime_t period_`, `int nextSno_` |
| Existing send path | `handleSelfMessage(cMessage*)` fires `selfSender_` then calls `generateAndSendSPDU()` |
| Log schema | `t,receiver,sender,msgId,lat,lon,dist_m,delay_ms,Numer of Vehicles,verified,spdu_overhead,cert_metadata,digest_size,pk_size,sig_size,bsm_data_size,spdu_size,Algorithm,signerType` |

Note: the log header contains the typo `Numer of Vehicles`. **Preserve it exactly**, existing
analysis scripts depend on it.

---

## 1. Hard constraints

1. **All 24 existing scenarios must reproduce bit-identically.** Every new parameter defaults
   to off/neutral. Do not change `[Config Base]` behaviour.
2. **Do not modify the MAC or PHY.** CTAC is an application-layer overlay above resource
   selection. SPS, CBR measurement, and CR limiting stay untouched. This is a design
   requirement, not a preference: the intellectual-property claim is specifically that this
   layer sits above resource selection.
3. **Do not change the existing CSV log schema.** Add columns only by appending, never
   reorder or rename.
4. Keep all CTAC logic in clearly delimited blocks so a reviewer can see exactly what was added.

---

## 2. Task 1: NED parameters and signals

Edit `src/apps/mode4App/mode4App.ned`.

### 2a. Add CTAC parameters

```ned
// ---- CTAC (Cooperative Transmission Authority Control) ----
bool   ctacEnabled   = default(false);   // master switch, off preserves legacy behaviour
int    ctacCohorts   = default(4);       // K, number of cohorts
double ctacEpoch @unit("s") = default(0.1s);   // epoch length
double ctacAoIBound @unit("s") = default(0.3s);// max time since own last full BSM
double ctacCellSize @unit("m") = default(60m); // spatial hash cell size
string ctacInactive  = default("defer"); // "defer" | "compress"
bool   ctacSafetyOverride = default(true);
```

### 2b. Fix the CBR statistic

The `cbr` signal is currently declared `record=vector` only, and vector recording is globally
disabled, so **CBR is emitted and silently discarded**. Change to:

```ned
@statistic[cbr](title="Channel Busy Ratio"; unit=""; source="cbr"; record=mean,max,timeavg,vector);
```

Apply the same fix to `warnPdrSample`, `warnPdrDistance`, `rxWarnDist` if those results are
wanted (add `mean` or `sum` alongside `vector`).

### 2c. Add new signals

```ned
@signal[bsmOpportunity];
@statistic[bsmOpportunity](title="BSM generation opportunities"; record=sum);

@signal[ctacDeferred];
@statistic[ctacDeferred](title="BSMs deferred by CTAC"; record=sum);

@signal[ctacCompressed];
@statistic[ctacCompressed](title="BSMs compressed by CTAC"; record=sum);

@signal[ctacOverrideAoI];
@statistic[ctacOverrideAoI](title="Freshness overrides"; record=sum);

@signal[ctacOverrideSafety];
@statistic[ctacOverrideSafety](title="Safety overrides"; record=sum);

@signal[ctrlOverheadBytes];
@statistic[ctrlOverheadBytes](title="CTAC control bytes"; unit="B"; record=sum);

@signal[ctacCohortId];
@statistic[ctacCohortId](title="Assigned cohort"; record=vector);
```

`ctrlOverheadBytes` will be zero in this version by design (see Task 4). Emit it anyway so the
result table has an explicit zero rather than a missing field.

---

## 3. Task 2: CTAC gate in `Mode4App`

### 3a. Header (`Mode4App.h`)

Add members and a decision enum:

```cpp
enum CtacDecision { CTAC_FULL, CTAC_COMPRESS, CTAC_DEFER };

// CTAC state
bool      ctacEnabled_ = false;
int       ctacCohorts_ = 4;
simtime_t ctacEpoch_;
simtime_t ctacAoIBound_;
double    ctacCellSize_ = 60.0;
bool      ctacCompressMode_ = false;   // from ctacInactive == "compress"
bool      ctacSafetyOverride_ = true;
simtime_t lastFullTx_ = SIMTIME_ZERO;
long      numDeferred_ = 0;

CtacDecision ctacDecide();
int  myCohort();
bool safetyEventActive();

simsignal_t bsmOpportunitySignal_, ctacDeferredSignal_, ctacCompressedSignal_;
simsignal_t ctacOverrideAoISignal_, ctacOverrideSafetySignal_;
simsignal_t ctrlOverheadBytesSignal_, ctacCohortIdSignal_;
```

Read the parameters and register the signals in `initialize()` at the same stage where existing
parameters such as `cryptoAlgo` and `certInterval` are read.

### 3b. The gate (`Mode4App.cc`)

Locate the branch in `handleSelfMessage()` that handles `selfSender_` and currently calls the
BSM generation path. Replace with:

```cpp
if (msg == selfSender_) {
    emit(bsmOpportunitySignal_, 1);          // ALWAYS, before the gate

    if (!ctacEnabled_) {
        generateAndSendSPDU();               // unchanged legacy path
        lastFullTx_ = simTime();
    } else {
        switch (ctacDecide()) {
            case CTAC_FULL:
                generateAndSendSPDU();
                lastFullTx_ = simTime();
                break;
            case CTAC_COMPRESS:
                generateAndSendCompact();    // see 3d
                emit(ctacCompressedSignal_, 1);
                break;
            case CTAC_DEFER:
                numDeferred_++;
                emit(ctacDeferredSignal_, 1);
                break;
        }
    }

    scheduleAt(simTime() + period_, selfSender_);   // OUTSIDE the branch
}
```

**Critical points.**

- `emit(bsmOpportunitySignal_, 1)` must fire on every timer tick regardless of the decision.
  This is the fixed denominator for delivery-ratio calculations. Without it, CTAC appears to
  improve PDR simply because it transmits less, which invalidates the entire study.
- The `scheduleAt` must stay outside the branch. The BSM clock never stops; CTAC only decides
  whether a tick produces a transmission.

### 3c. The decision function

Order of checks is significant and encodes the claim structure. Do not reorder.

```cpp
Mode4App::CtacDecision Mode4App::ctacDecide()
{
    // 1. Safety override, highest priority
    if (ctacSafetyOverride_ && safetyEventActive()) {
        emit(ctacOverrideSafetySignal_, 1);
        return CTAC_FULL;
    }
    // 2. Freshness (age-of-information) bound
    if (simTime() - lastFullTx_ >= ctacAoIBound_) {
        emit(ctacOverrideAoISignal_, 1);
        return CTAC_FULL;
    }
    // 3. Scheduled cohort turn
    long epoch = (long)floor(simTime().dbl() / ctacEpoch_.dbl());
    if (myCohort() == (int)(epoch % ctacCohorts_))
        return CTAC_FULL;

    // 4. Otherwise defer or compress
    return ctacCompressMode_ ? CTAC_COMPRESS : CTAC_DEFER;
}
```

For `safetyEventActive()`: implement as a stub returning `false` in this version, with a clear
`TODO` comment. Hooking it to hard-braking or ICA events is future work and is not needed for
the first result set.

### 3d. Compressed mode (optional for v1)

If `ctacInactive == "compress"`, send a reduced BSM: certificate digest only (never the full
certificate, regardless of `certInterval`) and a reduced payload. Reuse the existing digest
path already implemented for the `certInterval` logic.

If time is short, implement `generateAndSendCompact()` as a direct call to the deferral path
and leave `ctacInactive = "defer"` as the only tested mode. Record this limitation in a comment.

---

## 4. Task 3: Cohort assignment, zero signalling

Cohorts are derived implicitly from GNSS position. **No coordination messages are exchanged.**
This is deliberate: the standard objection to coordinated schemes is that control signalling
consumes the capacity it saves, and this version has zero such overhead.

```cpp
int Mode4App::myCohort()
{
    // Reuse whatever position source the existing lastIcaDist_ computation uses
    // (locate it in Mode4App.cc and mirror it; likely Veins TraCIMobility or inet IMobility).
    Coord p = <existing position accessor>;

    int cx = (int)floor(p.x / ctacCellSize_);
    int cy = (int)floor(p.y / ctacCellSize_);
    unsigned int h = ((unsigned int)cx * 73856093u) ^ ((unsigned int)cy * 19349663u);
    int cohort = (int)(h % (unsigned int)ctacCohorts_);

    emit(ctacCohortIdSignal_, cohort);
    return cohort;
}
```

Notes:
- Cohort membership changes naturally as the vehicle moves between cells, which provides the
  dynamic reassignment behaviour without any negotiation.
- Emit `ctrlOverheadBytes` as 0 once per vehicle in `finish()` so the scalar exists in results.

---

## 5. Task 4: V2V reception logging

**This is the highest-priority change and should be done first.**

Currently every row in `appl_logs.csv` has `receiver = rsu[0]`, meaning only vehicle-to-
infrastructure receptions are logged. CTAC's central claim concerns neighbour awareness, which
is vehicle-to-vehicle. Without V2V logging, the primary result figure cannot be produced.

Steps:
1. Locate the existing CSV logging code (search for the header string `Numer of Vehicles`).
   It is most likely in `Mode4RSUApp.cc`.
2. Mirror the same logging call in `Mode4App::handleLowerMessage()` so that each vehicle logs
   the BSMs it successfully receives, using the identical column schema with
   `receiver = carNoIp[n]`.
3. Gate it behind a NED parameter `bool logV2vRx = default(true);` so it can be disabled if the
   file size becomes unmanageable.

Expect a large increase in log volume (the baseline file is already about 39 MB with RSU-only
logging). Consider writing V2V receptions to a separate file `v2v_logs.csv` with the same
schema to keep existing parsers working unchanged.

---

## 6. Task 5: Scenario configuration

Edit `simulations/Mode4/omnetpp.ini`. Append after `_24_Falcon_F_NLOS`.

```ini
##########################################################
#     CTAC evaluation scenarios                          #
#     Each extends its exact ECDSA baseline so that      #
#     the ONLY difference is the CTAC parameter set.     #
##########################################################

[Config _25_CTAC_F_LOS]
extends = _11_ECDSA_F_LOS
*.carNoIp[*].appl.ctacEnabled   = true
*.carNoIp[*].appl.ctacCohorts   = 4
*.carNoIp[*].appl.ctacEpoch     = 0.1s
*.carNoIp[*].appl.ctacAoIBound  = 0.3s
*.carNoIp[*].appl.ctacCellSize  = 60m
*.carNoIp[*].appl.ctacInactive  = "defer"

[Config _26_CTAC_F_NLOS]
extends = _12_ECDSA_F_NLOS
*.carNoIp[*].appl.ctacEnabled   = true
*.carNoIp[*].appl.ctacCohorts   = 4
*.carNoIp[*].appl.ctacEpoch     = 0.1s
*.carNoIp[*].appl.ctacAoIBound  = 0.3s
*.carNoIp[*].appl.ctacCellSize  = 60m
*.carNoIp[*].appl.ctacInactive  = "defer"

# Mid-density control, needed to show where the benefit begins
[Config _27_CTAC_D_LOS]
extends = _7_ECDSA_D_LOS
*.carNoIp[*].appl.ctacEnabled   = true
*.carNoIp[*].appl.ctacCohorts   = 4
*.carNoIp[*].appl.ctacEpoch     = 0.1s
*.carNoIp[*].appl.ctacAoIBound  = 0.3s
*.carNoIp[*].appl.ctacCellSize  = 60m
*.carNoIp[*].appl.ctacInactive  = "defer"
```

Because each config extends its baseline, traffic file, channel model, crypto algorithm,
subchannel configuration, and simulation duration are all identical by construction.

### Multiple seeds

Add to `[General]` or to the CTAC and baseline configs:

```ini
repeat = 10
```

Runs must be compared **pairwise by repetition number**, not as pooled averages. Seed-to-seed
variation in SUMO traffic is large enough to hide the effect otherwise.

### Do not globally enable vector recording

Leave `**.vector-recording = false`. The `record=mean,max,timeavg` addition in Task 1 puts CBR
into the `.sca` file without the cost of full vectors. If a CBR time series is needed later,
enable it selectively:

```ini
**.carNoIp[*].appl.cbr:vector.vector-recording = true
```

---

## 7. Task 6: Analysis scripts

Create `analysis/` with the following.

### 7a. `compute_aoi.py`

Input: an `appl_logs.csv` (or `v2v_logs.csv`). Output: per-pair peak AoI and inter-packet gap
samples, plus a summary.

Method:
1. Generation time: `g = t - delay_ms / 1000.0`
2. Group by `(receiver, sender)`, sort by `t`
3. For consecutive receptions i-1, i:
   - `PAoI_i = t_i - g_{i-1}`  (equivalently: inter-packet gap plus the previous packet's delay)
   - `IPG_i  = t_i - t_{i-1}`
4. **Relevance filter**: keep a sample only if `dist_m < R` at both endpoints. Default `R = 300`
   metres, exposed as a command-line argument.
5. Report p50, p90, p95, p99, p99.9, mean, max, and `P(PAoI > threshold)` for thresholds
   200/300/500/1000 ms.

The relevance filter is mandatory. Without it, vehicles leaving communication range produce
unbounded AoI that reflects geometry rather than congestion, and the tail becomes meaningless.
Whatever value of `R` is used must be reported alongside results.

Known limitations to note in the script docstring: the first reception of each pair is dropped,
pairs with fewer than two receptions contribute no samples, and pairs from which nothing was
ever received are absent entirely. This slightly flatters whichever configuration loses more
packets, so it is a conservative bias for the baseline.

Reference values already computed for `11_ECDSA_F_LOS` at `R = 300 m`
(use these to validate the script reproduces them):

```
PAoI samples : 212543
p50          : 198.0 ms       IPG p50   : 100.0 ms
p90          : 295.0 ms       IPG p90   : 200.0 ms
p95          : 406.0 ms       IPG p95   : 399.0 ms
p99          : 681.0 ms       IPG p99   : 600.0 ms
p99.9        : 1172.0 ms      IPG p99.9 : 1100.0 ms
mean         : 218.9 ms       max       : 3569.0 ms
P(PAoI > 300 ms) = 7.91 %
```

### 7b. `extract_scalars.py`

Wrap `scavetool` to pull the scalars into a tidy dataframe:

```bash
scavetool export \
  -f 'name("sentMsg:sum") OR name("received:sum") OR name("bsmOpportunity:sum") OR name("cbr:mean") OR name("cbr:max") OR name("ctacDeferred:sum") OR name("ctacOverrideAoI:sum") OR name("ctacOverrideSafety:sum") OR name("ctrlOverheadBytes:sum")' \
  -o scalars.csv results/*.sca
```

Then compute, per run:
- Transmission-referenced PDR: `received / sentMsg`
- **Awareness-referenced delivery: `received / bsmOpportunity`**
- Mean and max CBR across vehicles
- Deferral rate: `ctacDeferred / bsmOpportunity`
- Override rates: `ctacOverrideAoI / bsmOpportunity`

Report both PDR definitions side by side. The awareness-referenced figure is the honest one,
because deferring shrinks the denominator of the classic definition.

### 7c. `plot_operating_point.py`

The primary presentation figure. Two axes, two points, one arrow.

- x axis: Channel Busy Ratio (mean across vehicles)
- y axis: percentage of samples with peak AoI above the 300 ms bound
- One point for the baseline, one for CTAC, error bars from the seed repetitions
- Vertical dashed lines at CBR 0.30 and 0.65 marking the J3161 DCC Level 1 and Level 2
  thresholds, with light shading above 0.65
- An arrow from baseline to CTAC, and a small "better" arrow pointing down and left

Down and to the left means lower channel load and fresher information simultaneously, which
directly answers the objection that the scheme merely transmits less.

### 7d. `plot_aoi_ccdf.py`

Secondary figure: complementary CDF of peak AoI on a log y axis, baseline versus CTAC, with a
vertical line at the AoI bound.

---

## 8. Verification checklist

Before reporting any results, confirm:

- [ ] All 24 original scenarios still run and produce results identical to the committed logs
- [ ] `bsmOpportunity:sum` is present and, with CTAC disabled, equals `sentMsg:sum`
- [ ] With CTAC disabled, `ctacDeferred:sum == 0` and results match the baseline exactly
- [ ] `cbr:mean` now appears in the `.sca` output (it did not before)
- [ ] `compute_aoi.py` reproduces the reference values in section 7a for `11_ECDSA_F_LOS`
- [ ] V2V receptions are being logged with `receiver = carNoIp[n]`
- [ ] `_25_CTAC_F_LOS` and `_11_ECDSA_F_LOS` differ in no parameter other than the CTAC block
- [ ] Deferral rate is non-zero and plausible: with K = 4 and no overrides firing, roughly
      75 percent of opportunities should be deferred. If it is near zero, the cohort or epoch
      logic is wrong. If it is near 100 percent, the AoI override is not firing.
- [ ] `ctacOverrideAoI` fires occasionally but not on nearly every epoch. If it fires almost
      always, K is too large relative to the AoI bound and the schedule is doing nothing.

---

## 9. Open question to resolve, not a code task

`[Config Base]` currently sets `dccMechanism = false` while `useCBR = true`, `crLimit = true`,
and `packetDropping = true`. Determine from `LteMacVUeMode4.cc` what `dccMechanism` actually
enables. If it is the J3161 rate-adaptation path, then the current baseline is
channel-occupancy limiting only, and describing it as "SPS + DCC" would overstate it. This
affects how the baseline is labelled in the paper, not the code.
