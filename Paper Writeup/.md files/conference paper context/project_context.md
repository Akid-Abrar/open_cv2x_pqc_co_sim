# Project Context: PQC C-V2X IEEE CNS 2026 Paper

This document gives a Claude instance (or a new collaborator) full context on this project so you can get up to speed immediately.

---

## Paper Identity

**Current title:** Comparative Performance Evaluation of Classical and Post-Quantum Digital Signatures on C-V2X Sidelink Communications

**Target venue:** IEEE CNS 2026 (primary); IEEE VTC-Fall 2026 under consideration

**Authors:**
- Akid Abrar (lead, PhD researcher, Dept. of Civil, Construction & Environmental Engineering, University of Alabama) — `aabrar@ua.edu`
- Sagar Dasgupta (Civil, Construction & Environmental Eng., University of Alabama)
- Abdullah Al Mamun (Glenn Dept. of Civil Engineering, Clemson University)
- Mizanur Rahman (Civil, Construction & Environmental Eng., University of Alabama)
- Mashrur Chowdhury (Glenn Dept. of Civil Engineering, Clemson University)
- Ahmad Alsharif (Dept. of Computer Science, University of Alabama)

---

## Research Question and Contribution

**Core question:** What happens to C-V2X Mode 4 sidelink performance (PDR and latency) when you replace ECDSA P-256 with Falcon-512 post-quantum signatures?

**Key finding:** Falcon-512 is computationally viable (latency is virtually unchanged: 51.82 ms vs. 51.92 ms for ECDSA) but causes severe PDR degradation (71.18% vs. 95.10% for ECDSA) due to its 5.7x larger packet size consuming 4 subchannels per transmission vs. 1 for ECDSA. Spectrum efficiency, not computational cost, is the primary bottleneck.

**Novelty:** No prior work had quantified end-to-end C-V2X sidelink KPIs under SAE J3161 radio constraints when using PQC signatures. Prior work focused on computational benchmarking on embedded hardware or protocol-level integration.

---

## Key Technical Parameters

### Algorithms Compared

| Parameter | ECDSA P-256 (baseline) | Falcon-512 (PQC candidate) |
|---|---|---|
| Signature size | 64 bytes | 666 bytes |
| Public key size | 65 bytes | 897 bytes |
| SPDU size (cert mode) | 305 bytes | 1,739 bytes |
| SPDU size (digest mode) | 143 bytes | 745 bytes |
| Subchannels needed (MCS 11) | 1 | 4 (cert), 2 (digest) |
| NIST security level | — | Level 1 (AES-128 equivalent) |

Falcon-512 is FN-DSA (FFT over NTRU-Lattice-Based Digital Signature Algorithm), also known as Falcon. It has been selected by NIST for standardization as FIPS 206, with finalization pending as of May 2026.

ML-DSA (Dilithium), FIPS 204, was finalized by NIST in August 2024. Dilithium-2 produces 2420-byte signatures (for reference).

### IEEE 1609.2 SPDU Composition (Falcon-512, cert mode)
- Protocol overhead: 57 bytes
- BSM payload (SAE J2735 kinematic data): 34 bytes
- Signature: 666 bytes
- Public key: 897 bytes
- Certificate overhead: ~85 bytes
- **Total: 1,739 bytes**

### C-V2X Radio Parameters (SAE J3161-compliant)
- Channel bandwidth: 20 MHz
- Resource blocks: 100 RBs per subframe
- RBs per subchannel: 10
- MCS range: 5 to 11 (PSSCH)
- TX power: 23 dBm
- Resource Reservation Interval (RRI): 100 ms (aligned with 10 Hz BSM rate)
- Semi-Persistent Scheduling (SPS) sensing window: 1000 ms
- p_keep: 0.8
- HARQ: disabled
- Transport block size (MCS 11, 10 subchannels): 3,496 bytes
- Certificate transmission: every 5th BSM; digest-only for remaining 4

### Traffic Scenario
- Scenario: 4-way urban intersection, line-of-sight (LoS) propagation
- Vehicle density: 30 veh/km (moderate traffic, level-of-service C per HCM)
- Traffic flow: 2,172 vehicles/hour/approach
- Average speed: 45 mph
- Simulation duration: 200 seconds

---

## Simulation Infrastructure

| Component | Role |
|---|---|
| openCV2X | Top-level simulator — open-source C-V2X Mode 4 simulator |
| SimuLTE | Network simulation layer (extends OMNeT++) |
| Veins | TraCI-based coupling between OMNeT++ and SUMO |
| SUMO | Vehicular mobility (traffic scenarios) |
| liboqs | Open Quantum Safe library used for PQC algorithm implementation |

openCV2X extends SimuLTE and uses Veins (TraCI) to connect to SUMO for realistic vehicular mobility.

---

## Performance Metrics and Results

### PDR (Packet Delivery Ratio)
- Method: 5GAA P-190033 sliding-window (5-second centered window per transmitter-receiver pair)
- Safety threshold: mean PDR >= 90% (SAE J2945/1 / industry standard)
- **ECDSA result: 95.10%** (above threshold)
- **Falcon-512 result: 71.18%** (23.92 percentage points below threshold)
- Falcon PDR degrades even at short ranges (<100 m), indicating contention-driven (not propagation-driven) failure

### Latency
- Measured from BSM creation (sender app layer) to reception (RSU app layer)
- Covers: cryptographic processing + MAC queuing + radio propagation
- Safety threshold: <= 100 ms (SAE J2945/1)
- **ECDSA mean: 51.92 ms**, 95th percentile: 96 ms
- **Falcon-512 mean: 51.82 ms**, 95th percentile: 97 ms
- Distributions virtually identical — Falcon-512 adds negligible latency overhead

### Signature Verification
- Both algorithms: 100% verification success on received packets

---

## Paper Structure (main.tex)

1. **Abstract** — summary of contribution and key numbers
2. **Background and Motivation** — C-V2X + ECDSA baseline, quantum threat, NIST PQC standards, related work (Sinell 2025, Barreto 2018, Lee 2025), contribution statement
3. **Method** — simulation setup (P1), Falcon-512 technical description (P2), SPS/radio parameters (P3), traffic scenario (P4), performance metrics (P5); includes methodology flowchart figure (fig:method)
4. **Results and Discussion** — SPDU size figure (fig:spdu_size), PDR vs distance figure (fig:pdr_dist), latency figure (fig:latency), results table (tab:results), discussion of spectrum bottleneck
5. **Conclusion and Future Work** — confirms spectral bottleneck; future: multiple densities, NLOS, Dilithium-2, hybrid schemes
6. **Acknowledgment** — TraCR / National Center for Transportation Cybersecurity and Resiliency

---

## File Structure

```
Paper Writeup/
├── IEEE_CNS_2026/
│   ├── main.tex                  # Master LaTeX file (edit via Overleaf)
│   ├── references.bib            # Bibliography
│   ├── temp.tex                  # Staging file: changes drafted here, applied to Overleaf manually
│   ├── figures/                  # All PDF/PNG figures included in paper
│   ├── conference_101719.*       # Compiled output files
│   └── methodology_flowchart.pptx  # PPTX for fig:method (editable)
├── scripts/
│   ├── conf_generate_figures_v2.py   # Generates fig_spdu_size, fig3_pdr_vs_distance, fig4_latency
│   └── gen_improved_flowchart.py     # Adds improved methodology slide to PPTX
├── Simulation Results/           # Raw simulation output data
├── .md/
│   └── project_context.md        # This file
└── notes/
```

---

## LaTeX Workflow (IMPORTANT)

The paper is maintained on **Overleaf**. The local `main.tex` is a synced copy.

**Never edit main.tex or references.bib directly.** All proposed changes are written to `temp.tex` as annotated snippets with instructions, and Akid applies them to Overleaf manually.

When writing LaTeX for this paper, follow these rules:
- No `~` (non-breaking space) — use a regular space
- No em-dashes (`---` or `--`) — use a comma or rephrase
- No `\cite{fips206}` — this entry was deleted; use `\cite{nist_pqc_2024}` or `\cite{fouque2017falcon}` instead
- Confirm with Akid before writing any edit to temp.tex

---

## Bibliography — Key Entries

| Cite key | What it is |
|---|---|
| `fips204` | FIPS 204: ML-DSA (Dilithium), finalized NIST standard, 2024 |
| `nist_pqc_2024` | NIST August 2024 announcement — states FIPS 206 (Falcon) is planned but pending |
| `fouque2017falcon` | Falcon algorithm specification paper (Fouque et al. 2017) |
| `sae_j3161` | SAE J3161 — C-V2X on-board system requirements, defines radio parameters used |
| `sae_j29451` | SAE J2945/1 — 100 ms latency threshold for V2X safety |
| `5gaa_p190033` | 5GAA P-190033 — defines the sliding-window PDR methodology |
| `ieee_16092` | IEEE 1609.2 — SPDU / ECDSA specification for V2X |
| `3gpp_36213` | 3GPP TS 36.213 — LTE-V physical layer procedures |
| `opencv2x` | openCV2X simulator (Brian McCarthy, GitHub) |
| `virdis2014simulte` | SimuLTE — OMNeT++-based LTE simulator |
| `sommer2019veins` | Veins — vehicular network simulation framework |
| `lopez2018sumo` | SUMO — vehicular traffic mobility simulator |
| `molina2017lte` | LTE-V sidelink / Mode 4 description |
| `twardokus2025chasm` | CHASM — source for 90% PDR safety threshold |
| `brecht2018scms` | SCMS — Security Credential Management System |
| `sinell2025pqc_automotive` | Sinell et al. 2025, FICC — PQC on automotive MCU (Falcon verification meets real-time, key gen unpredictable) |
| `barreto2018qscms` | Barreto et al. 2018 — post-quantum SCMS using lattice crypto, no radio analysis |
| `lee2025pqc_ami` | Lee et al. 2025, IEEE CNS — PQC for AMI gateways, QEMU-based |
| `MAMUN2026101028` | Mamun et al. 2026, Vehicular Communications — PQC for ITS review (co-authored by team) |
| `HCM2016` | Highway Capacity Manual 6th ed. — source for level-of-service C definition |
| `shor1994algorithms` | Shor 1994 — quantum algorithm breaking ECC/RSA |

**Deleted entries (do not re-add):**
- `fips206` — had wrong title (said SLH-DSA instead of Falcon); standard not yet finalized
- `kumar2024pqc_automotive` — was a fabricated placeholder
- `so2018pqc_protocols` — was a fabricated placeholder

---

## Figures

| File | Label | Caption summary |
|---|---|---|
| `figures/methodology_flowchart.pdf` | `fig:method` | Step-by-step evaluation process flowchart |
| `figures/fig_spdu_size.pdf` | `fig:spdu_size` | SPDU size comparison, cert vs. digest mode |
| `figures/fig3_pdr_vs_distance.pdf` | `fig:pdr_dist` | PDR vs. distance, both algorithms, LOS, 30 veh/km |
| `figures/fig4_latency.pdf` | `fig:latency` | Latency box plots, both algorithms |

Figures generated by `scripts/conf_generate_figures_v2.py`. Font sizes are set to 4pt (half of standard) for IEEE single-column (3.5" wide) format.

---

## Pending / Open Items (as of May 2026)

1. Apply all accumulated `temp.tex` changes to Overleaf (multiple sections have staged edits)
2. Add to references.bib: `sinell2025pqc_automotive`, `barreto2018qscms`, `nist_pqc_2024`, `fouque2017falcon`
3. Decide final venue: IEEE CNS 2026 or IEEE VTC-Fall 2026
4. Issue 4 (RSU not mentioned in Background despite being central to latency results) — under review by Akid

---

## Future Work Planned (VTC Journal Extension)

If accepted at VTC-Fall, the plan is to extend to **IEEE Transactions on Vehicular Technology (TVT)**:
- Multiple traffic densities (low, moderate, high)
- NLOS propagation scenarios
- Additional PQC candidates: Dilithium-2, hybrid ECDSA+Falcon
- More rigorous statistical treatment (confidence intervals, multiple runs)
- Density-aware deployment guidelines
- IEEE policy: journal version must have >= 30% new content; cite conference paper in journal submission
