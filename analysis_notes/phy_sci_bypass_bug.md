# PHY Layer Bug: TB Packets Bypass SCI Decoding Check

## Date Identified: 2026-02-25

## Summary

The PHY layer in `LtePhyVUeMode4.cc` unconditionally forwards Transport Block (TB)
packets to the upper protocol layers based solely on the TB's data-channel SINR,
completely ignoring whether the corresponding Sidelink Control Information (SCI) was
successfully decoded. This violates the fundamental C-V2X Mode 4 reception procedure
defined in 3GPP TS 36.213 Section 14.2.1.

In real C-V2X hardware, a UE **cannot decode a TB without first decoding its SCI**,
because the SCI carries the MCS, resource allocation (RIV), and retransmission
parameters needed to locate and demodulate the TB data. Without SCI, the receiver
does not even know which subchannels carry the TB or what modulation was used.

## Impact on Simulation Results

Because the SCI check is bypassed, the simulation reports unrealistically high
Packet Delivery Ratios. Furthermore, packet-size differences between cryptographic
algorithms (Falcon-512 vs ECDSA) have almost no measurable impact on PDR, because
the bottleneck (SCI-gated reception) is never enforced. This explains the anomalous
result where Falcon-512 (larger packets, ~1622 bytes) showed 98.38% PDR while
ECDSA (smaller packets, ~250 bytes) showed 74.75% in the Stress-NLOS scenario —
the difference is noise from random seeds, not a real physical effect.

## Evidence

### Statistics from Stress-NLOS run (Falcon-512):

| Metric                      | Value    |
|-----------------------------|----------|
| sciReceived                 | 771,376  |
| sciDecoded                  | 0        |
| tbReceived                  | 771,376  |
| tbDecoded                   | 0        |
| tbDecodedIgnoreSCI          | 586,981  |
| tbFailedDueToNoSCI          | 771,376  |
| tbFailedButSCIReceived      | 0        |
| tbFailedDueToProp           | 0        |
| tbFailedDueToInterference   | 0        |
| App-layer received          | 572,430  |

Key observations:
- **Zero SCIs decoded**, yet **572,430 packets delivered** to the application layer.
- ALL TB failures are classified as `tbFailedDueToNoSCI`, confirming SCI is the
  bottleneck, yet packets flow through anyway.
- `tbDecodedIgnoreSCI = 586,981` closely matches `App-layer received = 572,430`,
  confirming the app layer is receiving the "ignore SCI" packets.

### Statistics from Base run (LOS, Falcon-512):

| Metric                      | Value    |
|-----------------------------|----------|
| sciDecoded                  | 12       |
| tbDecoded                   | 12       |
| tbDecodedIgnoreSCI          | 778,116  |
| tbFailedDueToNoSCI          | 778,104  |
| App-layer received          | 758,709  |

Same pattern: near-zero legitimate SCI decodes, but hundreds of thousands of
packets delivered to the application.

## Root Cause: Code Walkthrough

### File: `src/stack/phy/layer/LtePhyVUeMode4.cc`

### Step 1: TB decoding in `decodeAirFrame()` (line ~1607)

When a TB arrives, the code searches for a matching SCI from the same sender:

```cpp
bool foundCorrespondingSci = false;
bool sciDecodedSuccessfully = false;
// ... search scis_ vector for matching source ID ...
if (sciInfo->getSourceId() == lteInfo->getSourceId()) {
    foundCorrespondingSci = true;
    if (sciInfo->getDeciderResult()) {
        sciDecodedSuccessfully = true;
    }
    // THIS IS THE PROBLEM: error_Mode4() is called even when SCI failed
    std::tuple<bool, bool> res = channelModel_->error_Mode4(
        frame, lteInfo, rsrpVector, sinrVector, correspondingSCI->getMcs());
    prop_result = get<0>(res);
    interference_result = get<1>(res);  // <-- set based on TB SINR alone
}
```

Note: `error_Mode4()` at line 1631 is called inside the `foundCorrespondingSci`
block, but OUTSIDE the `sciDecodedSuccessfully` check. So even when SCI decoding
failed, the TB's SINR is evaluated and `interference_result` may be set to `true`.

### Step 2: Statistics are recorded correctly (line ~1643)

```cpp
if (!foundCorrespondingSci || !sciDecodedSuccessfully) {
    tbFailedDueToNoSCI_ += 1;           // correctly counted as failure
    if (!prop_result) {
        tbFailedDueToPropIgnoreSCI_ += 1;
    } else if (!interference_result) {
        tbFailedDueToInterferenceIgnoreSCI_ += 1;
    } else {
        tbDecodedIgnoreSCI_ += 1;        // would have decoded IF SCI worked
    }
} else if (...) {
    // Only reaches here if SCI decoded AND TB decoded
    tbDecoded_ += 1;
    tbDecodedIgnoreSCI_ += 1;
}
```

The statistics correctly distinguish between `tbDecoded` (full success) and
`tbDecodedIgnoreSCI` (TB SINR was OK but SCI failed). However...

### Step 3: THE BUG — Packet forwarding ignores statistics (line 1745)

```cpp
// send decapsulated message along with result control info to upperGateOut_
lteInfo->setDeciderResult(interference_result);   // <-- ONLY checks TB SINR
pkt->setControlInfo(lteInfo);
send(pkt, upperGateOut_);                          // <-- ALWAYS sent upward
```

The `deciderResult` is set to `interference_result`, which reflects ONLY the
TB data-channel SINR. It does NOT account for SCI decoding failure. Since
`interference_result` is `true` for most nearby transmitters (good SINR),
almost all packets are forwarded to the upper layer with `deciderResult = true`.

### Step 4: HARQ accepts based on deciderResult (LteHarqProcessRxMode4.cc:49)

```cpp
if (lteInfo->getDeciderResult()){
    status_.at(cw) = RXHARQ_PDU_CORRECT;    // packet accepted
} else {
    status_.at(cw) = RXHARQ_PDU_CORRUPTED;  // packet dropped
}
```

Since `deciderResult = interference_result = true`, the HARQ process marks
the PDU as CORRECT, and it flows up through RLC to the application layer.

## Why SCI Decoding Fails So Often

A separate but related question: why is `sciDecoded` near-zero even in the
Base (LOS) scenario?

The SCI decoding at line 1561 has an additional gate:

```cpp
if (interference_result & !sensingWindow_[sensingWindowFront_][subchannelIndex]->getReserved())
```

The `!getReserved()` check means: only decode the SCI if that subchannel was
NOT already marked as reserved by a previously decoded SCI in the same subframe.
This is a sensing-window bookkeeping mechanism. If the very first SCI in a
subframe is decoded, it marks the subchannels as reserved, and ALL subsequent
SCIs on those same subchannels are rejected — even if they have higher SINR.

With 60 vehicles sharing 10 subchannels and RRI=100ms, heavy subchannel reuse
means most SCIs arrive on already-reserved subchannels. Combined with the
probabilistic sensing check (line 1526: `er >= packetSensingRatio`), the SCI
decode rate collapses.

This is partially a correct behavior (collision modeling), but the near-zero
decode rate across ALL subchannels suggests the sensing window reservation
logic may be too aggressive. This is a separate issue to investigate.

## Relationship Between the Two Issues

1. **Bug (must fix):** Packets bypass SCI check when forwarded to upper layers.
   The `deciderResult` must account for SCI decoding status.

2. **Potential issue (investigate separately):** SCI decoding rate is
   suspiciously low even in LOS. The `getReserved()` gate and probabilistic
   sensing may need tuning or review.

Fixing issue #1 alone will cause PDR to drop dramatically (to near-zero in
current scenarios), which may reveal that issue #2 also needs attention.
The correct approach is to fix #1 first, then investigate #2 if PDR is
unreasonably low.
