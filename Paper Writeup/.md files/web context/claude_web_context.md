# Claude Web Context — PQC C-V2X Paper (IEEE CNS / VTC 2026)

Paste everything below this line into Claude web to get full project context.

---

You are helping with an academic paper titled **"Comparative Performance Evaluation of Classical and Post-Quantum Digital Signatures on C-V2X Sidelink Communications"**, targeting **IEEE CNS 2026** (IEEE VTC-Fall 2026 also under consideration). The lead author is **Akid Abrar**, a PhD researcher at the University of Alabama (Civil, Construction & Environmental Engineering). Co-authors: Sagar Dasgupta and Mizanur Rahman (UA), Abdullah Al Mamun and Mashrur Chowdhury (Clemson University), Ahmad Alsharif (Computer Science, UA).

---

## What the paper does

It evaluates whether **Falcon-512**, a post-quantum digital signature scheme, can be deployed on **C-V2X PC5 Mode 4 sidelink** communications without degrading safety-critical performance. The comparison baseline is **ECDSA P-256**, the current standard. The simulation is SAE J3161-compliant and uses openCV2X (extending SimuLTE/OMNeT++) with Veins (TraCI) coupled to SUMO for vehicular mobility.

**Core finding:** Falcon-512 is computationally viable — latency is virtually unchanged (51.82 ms vs. 51.92 ms for ECDSA). However, its 5.7x larger packet size (1,739 bytes vs. 305 bytes in certificate mode) requires 4 subchannels per transmission vs. 1 for ECDSA, causing severe PDR degradation: **71.18% vs. 95.10%**, well below the 90% safety-critical threshold. Spectrum efficiency, not computational cost, is the primary bottleneck for PQC adoption in C-V2X.

---

## Key technical facts

**Falcon-512 (FN-DSA):** Selected by NIST for standardization as FIPS 206, finalization pending as of May 2026. NOT yet an official standard. ML-DSA (Dilithium, FIPS 204) was finalized August 2024. Always distinguish these — do not call Falcon-512 "NIST standardized."

**SPDU sizes:**
- ECDSA: 305 bytes (cert), 143 bytes (digest)
- Falcon-512: 1,739 bytes (cert), 745 bytes (digest)
- Falcon cert SPDU breakdown: 57B protocol overhead + 34B BSM payload + 666B signature + 897B public key + ~85B certificate overhead

**Radio parameters (SAE J3161):** 20 MHz, 100 RBs/subframe, 10 RBs/subchannel, MCS 5–11 (PSSCH), 23 dBm TX power, RRI = 100 ms, SPS sensing window = 1000 ms, p_keep = 0.8, HARQ disabled. TB size at MCS 11 = 3,496 bytes. Certificate transmitted every 5th BSM; digest otherwise.

**Traffic scenario:** 4-way urban intersection, line-of-sight (LoS), 30 veh/km (level-of-service C per HCM), 2,172 veh/hr/approach, 45 mph average speed, 200 s simulation.

**PDR method:** 5GAA P-190033 sliding-window — 5-second centered window per transmitter-receiver pair. Safety threshold: mean PDR >= 90%.

**Latency:** App-layer creation to RSU app-layer reception. Covers crypto + MAC queuing + propagation. Safety threshold: <= 100 ms (SAE J2945/1). Both algorithms well within threshold.

---

## Paper structure (main.tex sections)

1. Abstract
2. Background and Motivation — C-V2X/ECDSA baseline, quantum threat (Shor's algorithm), NIST PQC status, related work (Sinell 2025 automotive MCU benchmarking, Barreto 2018 qSCMS, Lee 2025 AMI gateways), contribution statement
3. Method — simulation setup with fig:method flowchart, Falcon-512 technical description, SPS/radio parameters, traffic scenario, PDR and latency metrics
4. Results and Discussion — SPDU size figure, PDR vs. distance figure, latency boxplot figure, results table, spectral bottleneck discussion
5. Conclusion and Future Work — confirms spectral bottleneck; future: multiple densities, NLOS, Dilithium-2, hybrid schemes
6. Acknowledgment — TraCR (National Center for Transportation Cybersecurity and Resiliency)

---

## LaTeX workflow rules (follow strictly)

- The paper lives on **Overleaf**. The local file is `IEEE_CNS_2026/main.tex` (read-only reference).
- **Never edit main.tex or references.bib directly.** Write all proposed changes to `IEEE_CNS_2026/temp.tex` with clear section labels and instructions. Akid applies them to Overleaf manually.
- **Always confirm before writing any edit to temp.tex.**
- No `~` (non-breaking space) anywhere in LaTeX — use a regular space.
- No em-dashes (`---` or `\textemdash`) — use a comma or rephrase.
- No `\cite{fips206}` — this bib entry was deleted. Use `\cite{nist_pqc_2024}` (for FIPS 206 status) or `\cite{fouque2017falcon}` (for Falcon technical description) instead.

---

## Key bibliography entries

| Key | Description |
|---|---|
| `fips204` | FIPS 204: ML-DSA (Dilithium), finalized August 2024 |
| `nist_pqc_2024` | NIST Aug 2024 announcement — FIPS 206 (Falcon) is planned, pending |
| `fouque2017falcon` | Falcon specification paper (Fouque et al., falcon-sign.info) |
| `sae_j3161` | SAE J3161 — C-V2X radio parameters |
| `sae_j29451` | SAE J2945/1 — 100 ms latency threshold |
| `5gaa_p190033` | 5GAA P-190033 — sliding-window PDR methodology |
| `ieee_16092` | IEEE 1609.2 — SPDU/ECDSA V2X standard |
| `3gpp_36213` | 3GPP TS 36.213 — LTE-V physical layer |
| `opencv2x` | openCV2X simulator |
| `virdis2014simulte` | SimuLTE (OMNeT++) |
| `sommer2019veins` | Veins vehicular simulation framework |
| `lopez2018sumo` | SUMO traffic simulator |
| `molina2017lte` | LTE-V Mode 4 / SPS description |
| `twardokus2025chasm` | CHASM — source for 90% PDR safety threshold |
| `brecht2018scms` | SCMS — Security Credential Management System |
| `sinell2025pqc_automotive` | Sinell et al. FICC 2025 — PQC on automotive MCU |
| `barreto2018qscms` | Barreto et al. 2018 — post-quantum SCMS, no radio analysis |
| `lee2025pqc_ami` | Lee et al. IEEE CNS 2025 — PQC for AMI gateways |
| `MAMUN2026101028` | Mamun et al. 2026, Vehicular Comms — PQC for ITS review (team co-authored) |
| `HCM2016` | Highway Capacity Manual 6th ed. — level-of-service C definition |
| `shor1994algorithms` | Shor 1994 — quantum algorithm breaking ECC |

**Deleted entries — do not re-add:** `fips206` (wrong title, non-finalized standard), `kumar2024pqc_automotive` (fabricated placeholder), `so2018pqc_protocols` (fabricated placeholder).

---

## Figures

| File | Label | Content |
|---|---|---|
| `figures/methodology_flowchart.pdf` | `fig:method` | Step-by-step evaluation flowchart |
| `figures/fig_spdu_size.pdf` | `fig:spdu_size` | SPDU size comparison (cert vs. digest) |
| `figures/fig3_pdr_vs_distance.pdf` | `fig:pdr_dist` | PDR vs. distance, both algorithms |
| `figures/fig4_latency.pdf` | `fig:latency` | End-to-end latency box plots |

Figures 2–4 generated by `scripts/conf_generate_figures_v2.py`.

---

## Results summary table

| Metric | ECDSA P-256 | Falcon-512 |
|---|---|---|
| SPDU size, cert (B) | 305 | 1,739 |
| SPDU size, digest (B) | 143 | 745 |
| Mean latency (ms) | 51.92 | 51.82 |
| 95th pct latency (ms) | 96 | 97 |
| Mean PDR (%) | 95.10 | 71.18 |
| Verification success (%) | 100 | 100 |

---

## Open items / pending

1. Apply accumulated `temp.tex` staged edits to Overleaf (multiple sections)
2. Add to references.bib: `sinell2025pqc_automotive`, `barreto2018qscms`, `nist_pqc_2024`, `fouque2017falcon`
3. Final venue decision: IEEE CNS 2026 vs. IEEE VTC-Fall 2026
4. RSU mention in Method but not setup in Background — under review

## Future extension plan

If accepted at VTC-Fall, extend to **IEEE Transactions on Vehicular Technology (TVT)**:
- Multiple traffic densities, NLOS scenarios, Dilithium-2 and hybrid ECDSA+Falcon
- IEEE policy: journal version needs >= 30% new content; must cite conference paper
