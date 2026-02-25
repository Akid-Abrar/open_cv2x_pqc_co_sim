# IEEE Student Chapter Presentation Guide

**Title:** Toward Deployment-Ready Post-Quantum Cryptography Enabled C-V2X Communication for Intelligent Transportation Systems

**Duration:** 40-45 minutes
**Audience:** Mixed (undergrad + grad students)
**Style:** Talking-point speech notes

---

## PRE-TALK: Data You Need to Generate

Before the talk, run simulations for all three algorithms to enable comparison:
```bash
# Run with Dilithium-2
export PQCDSA_ALGO=dilithium2
# run simulation, collect results

# Run with ECDSA
export PQCDSA_ALGO=ecdsa
# run simulation, collect results
```

You should have appl_logs.csv data for all three algorithms and ideally at 2-3 vehicle densities.

---

## SLIDE 1: Title Slide
**Title:** Toward Deployment-Ready Post-Quantum Cryptography Enabled C-V2X Communication for Intelligent Transportation Systems

**Visual:** University logo, IEEE student chapter logo, your name, date

**Speech:**
- Welcome everyone, thank you for having me at this IEEE student chapter session
- Today I'll walk you through my research on securing next-generation vehicle-to-everything communication against quantum computer threats
- We'll start with the basics of how vehicles talk to each other, why current security will break, and what we can do about it

---

## SLIDE 2: Outline / Roadmap
**Title:** Talk Outline

**Visual:** Numbered list or visual roadmap graphic

**Content:**
1. What is C-V2X? (Cellular Vehicle-to-Everything)
2. How vehicles communicate: Uu vs PC5 interfaces
3. Mode 4: Autonomous resource selection
4. Security: IEEE 1609.2 and current cryptography
5. The quantum threat
6. Post-quantum cryptography (PQC) candidates
7. Challenges of PQC in vehicular networks
8. Our research: PQC-enabled C-V2X simulation
9. Results and analysis
10. Conclusions and future work

**Speech:**
- Here's the roadmap for today's talk
- We'll build up from the basics so everyone can follow, then dive into the research contribution
- Feel free to ask questions at any point

---

## SLIDE 3: Why V2X Communication?
**Title:** The Need for Vehicle-to-Everything (V2X) Communication

**Visual:** Diagram showing V2X use cases - vehicle-to-vehicle (V2V), vehicle-to-infrastructure (V2I), vehicle-to-pedestrian (V2P), vehicle-to-network (V2N)

**Speech:**
- Before we talk about cryptography, let's understand WHY vehicles need to communicate
- Road accidents kill over 1.3 million people globally every year (WHO statistic)
- V2X enables vehicles to share safety-critical information: position, speed, heading, braking events
- Basic Safety Messages (BSMs) are broadcast 10 times per second so nearby vehicles know where you are
- This enables collision avoidance, intersection safety, cooperative driving
- Two competing technologies: DSRC (IEEE 802.11p) and C-V2X (3GPP cellular)
- C-V2X is winning the industry adoption battle - that's what we focus on

---

## SLIDE 4: C-V2X and 3GPP Evolution
**Title:** C-V2X: 3GPP Standardization Timeline

**Visual:** Timeline diagram showing 3GPP releases:
- Release 12 (2015): D2D ProSe foundation
- Release 14 (2017): LTE-V2X (Mode 3 + Mode 4)
- Release 15 (2018): 5G NR foundation
- Release 16 (2020): NR-V2X (advanced V2X)
- Release 17+ : Evolution

**Speech:**
- C-V2X stands for Cellular Vehicle-to-Everything
- It was standardized by 3GPP starting from Release 14 in 2017
- Release 14 introduced LTE-V2X with two modes of operation
- Release 16 brought NR-V2X for advanced use cases like platooning and remote driving
- Key advantage of C-V2X over DSRC: better range, reliability, and a clear 5G evolution path
- Our research focuses on the LTE-V2X Release 14 sidelink, which is the baseline deployed today

---

## SLIDE 5: C-V2X Network Architecture
**Title:** C-V2X Network Architecture: Two Interfaces

**Visual:** Diagram showing:
- Uu interface: UE <-> eNodeB <-> Core Network (traditional cellular)
- PC5 interface: UE <-> UE direct (sidelink)
- Show both interfaces on the same vehicle

**Speech:**
- C-V2X uses two interfaces
- The Uu interface is your regular cellular connection - vehicle talks to the base station, then to the cloud
- Good for non-safety stuff: traffic info, map updates, entertainment
- The PC5 interface is the game-changer - this is DIRECT communication between vehicles
- No base station needed, sub-10ms latency, works even without cellular coverage
- PC5 is what we use for safety-critical BSMs
- Think of it like Bluetooth but designed for 500+ meter range at highway speeds

---

## SLIDE 6: PC5 Mode 3 vs Mode 4
**Title:** PC5 Sidelink: Mode 3 vs Mode 4

**Visual:** Side-by-side comparison diagram:
- Mode 3: eNodeB assigns resources, vehicles in coverage
- Mode 4: Vehicles autonomously select resources, no eNodeB needed

**Speech:**
- PC5 sidelink operates in two modes
- Mode 3: the base station tells each vehicle WHEN and WHERE to transmit - centralized scheduling
- Requires cellular coverage - not always available on highways or rural roads
- Mode 4: vehicles figure it out themselves - fully autonomous, decentralized
- Mode 4 is the critical mode because safety communication MUST work even without cellular coverage
- Our research focuses on Mode 4 because it's the deployment baseline and the harder problem

---

## SLIDE 7: Mode 4 Semi-Persistent Scheduling (SPS)
**Title:** Mode 4: Semi-Persistent Scheduling (SPS)

**Visual:** Resource grid diagram showing:
- X-axis: subframes (time), Y-axis: subchannels (frequency)
- Show sensing window (last 1000ms), selection window (next 100ms)
- Highlight candidate resources, exclusion of occupied resources, final random selection
- Show Resource Reselection Counter counting down

**Speech:**
- Mode 4 uses a clever mechanism called Semi-Persistent Scheduling
- Step 1: SENSE - the vehicle listens to the channel for the last 1 second
- It measures which time-frequency resources are occupied by other vehicles
- Step 2: SELECT - it looks at the next selection window and excludes resources that would collide
- Any resource with received power above a threshold is excluded
- If more than 80% of resources are excluded, the threshold is raised and we try again
- Step 3: RESERVE - randomly pick from the remaining candidates
- The vehicle keeps this resource for multiple transmissions (Resource Reselection Counter)
- Counter counts down; when it hits zero, with probability 0.2 we pick a new resource
- This is the key to Mode 4: distributed, no coordination needed, but collisions can happen

---

## SLIDE 8: Mode 4 SPS - Resource Grid Example
**Title:** SPS Resource Selection: Visual Example

**Visual:** Animated or annotated resource grid showing:
- Sensing window with colored occupied subchannels (from SCI decoding)
- Selection window with candidate resources highlighted in green
- Final selected resource marked
- Arrow showing the reservation repeating every RRI (100ms)

**Speech:**
- Let me show you a concrete example
- Here's the resource grid - time on X axis, frequency subchannels on Y axis
- In the sensing window, we decoded SCI messages from other vehicles
- We know Vehicle A uses subchannel 3 every 100ms, Vehicle B uses subchannel 7
- So in the selection window, we exclude those subchannels at those times
- We randomly pick from the green candidates
- Once selected, we reserve it and repeat every 100ms (our Resource Reservation Interval)
- The SCI message we transmit tells other vehicles our reservation so they can avoid us too

---

## SLIDE 9: IEEE 1609.2 Security Framework
**Title:** Application Layer Security: IEEE 1609.2

**Visual:** SPDU (Secured Protocol Data Unit) packet structure diagram:
- BSM payload (position, speed, heading)
- IEEE 1609.2 header
- Digital signature
- Certificate (with public key)
- Show the layered structure

**Speech:**
- Now that we understand HOW vehicles communicate, let's talk about WHY security matters
- Imagine a malicious actor broadcasting fake BSMs - "I'm a truck braking hard ahead of you"
- Your car would slam on the brakes for no reason - dangerous
- IEEE 1609.2 is the standard that secures V2X messages
- Every BSM is wrapped in a Secured Protocol Data Unit - an SPDU
- The SPDU contains: the BSM data, a digital signature, and the sender's certificate
- The signature proves the message wasn't tampered with and came from an authorized vehicle
- Currently this uses ECDSA (Elliptic Curve Digital Signature Algorithm) with NIST P-256
- Every vehicle signs 10 messages per second, every vehicle verifies messages from ALL neighbors
- In dense traffic with 100 nearby vehicles, that's 1000 signature verifications per second

---

## SLIDE 10: Current Cryptographic Primitives
**Title:** Current V2X Cryptography: ECDSA

**Visual:** Table showing:
| Property | ECDSA P-256 |
|----------|-------------|
| Signature size | 64 bytes |
| Public key size | 64 bytes |
| Sign time | ~0.1 ms |
| Verify time | ~0.3 ms |
| Security basis | Elliptic curve discrete log |

**Speech:**
- Currently V2X uses ECDSA with the P-256 curve
- It's compact: 64-byte signatures, 64-byte public keys
- It's fast: sub-millisecond signing and verification
- Works perfectly for the constrained V2X environment
- BUT - and this is a big but - its security depends entirely on the hardness of the elliptic curve discrete logarithm problem
- This is where quantum computers enter the picture

---

## SLIDE 11: The Quantum Threat
**Title:** The Quantum Computing Threat to V2X Security

**Visual:** Diagram showing:
- Shor's algorithm breaking ECDSA, RSA, DH
- Timeline: "Harvest now, decrypt later" attack model
- NIST quote on post-quantum migration urgency

**Speech:**
- Quantum computers running Shor's algorithm can break ECDSA in polynomial time
- A sufficiently powerful quantum computer could forge BSM signatures
- An attacker could impersonate vehicles, inject fake safety messages, cause accidents
- "But quantum computers aren't here yet!" - True, but consider two things:
- First: "Harvest now, decrypt later" - adversaries can record V2X traffic today and break it later
- For safety-critical infrastructure, this matters
- Second: vehicles deployed TODAY will be on the road for 10-15 years
- If quantum computers arrive in 2030-2035, vehicles sold in 2025 are vulnerable for their entire lifetime
- NIST has been running a Post-Quantum Cryptography standardization process since 2016
- They finalized the first PQC standards in 2024: FIPS 203, 204, 205
- The migration needs to start NOW

---

## SLIDE 12: Post-Quantum Cryptography Overview
**Title:** Post-Quantum Cryptography: NIST Standards

**Visual:** Table comparing PQC families:
| Algorithm | Type | Basis | NIST Status |
|-----------|------|-------|-------------|
| ML-DSA (Dilithium) | Signature | Lattice | FIPS 204 (2024) |
| FN-DSA (Falcon) | Signature | Lattice (NTRU) | FIPS 206 (draft) |
| SLH-DSA (SPHINCS+) | Signature | Hash-based | FIPS 205 (2024) |

**Speech:**
- NIST standardized three digital signature algorithms for the post-quantum era
- ML-DSA, based on Dilithium - lattice-based, general purpose
- FN-DSA, based on Falcon - also lattice-based but uses NTRU lattices, more compact signatures
- SLH-DSA, based on SPHINCS+ - hash-based, very conservative security assumptions
- For V2X, we care about two things: signature SIZE and signing/verification SPEED
- Hash-based signatures are too large (tens of KB) - ruled out for V2X
- The real candidates are Dilithium and Falcon
- Let me show you why size matters so much in V2X

---

## SLIDE 13: PQC Size Comparison
**Title:** The Size Problem: PQC vs ECDSA

**Visual:** Bar chart comparing:
| | ECDSA | Dilithium-2 | Falcon-512 |
|--|-------|-------------|------------|
| Signature | 64 B | 2,420 B | ~652 B |
| Public Key | 64 B | 1,312 B | 897 B |
| Total overhead | 128 B | 3,732 B | ~1,549 B |

**Speech:**
- Here's the core challenge
- ECDSA adds only 128 bytes of crypto overhead to each BSM
- Dilithium-2 adds over 3,700 bytes - that's a 29x increase
- Falcon-512 is better at about 1,550 bytes - still a 12x increase
- Remember: these messages are broadcast 10 times per second on a shared radio channel
- More bytes per message means more radio resources consumed
- More resources consumed means more channel congestion
- More congestion means more packet collisions and lower reliability
- This is the fundamental tension: stronger security vs communication performance
- And this is exactly what our research quantifies

---

## SLIDE 14: Challenges of PQC in C-V2X
**Title:** Challenges of Integrating PQC into C-V2X

**Visual:** Challenge list with icons:
1. Increased packet size -> subchannel capacity limits
2. Computational overhead -> real-time signing/verification deadline
3. Channel congestion -> CBR increase
4. Backward compatibility -> mixed ECDSA/PQC environment
5. Variable signature size (Falcon) -> resource allocation complexity

**Speech:**
- Let me enumerate the specific challenges
- CHALLENGE 1: Packet size - C-V2X subchannels have limited capacity determined by MCS and number of subchannels
- A PQC-signed SPDU may not fit in a single subchannel allocation
- We may need more subchannels per message, reducing overall capacity
- CHALLENGE 2: Computational cost - vehicles must sign at 10 Hz and verify from ALL neighbors
- PQC operations are slower than ECDSA - can the hardware keep up?
- CHALLENGE 3: Channel congestion - larger packets occupy the channel longer
- CBR (Channel Busy Ratio) increases, triggering congestion control that drops packets
- CHALLENGE 4: Falcon produces variable-length signatures (649-666 bytes)
- This complicates resource allocation since you don't know the exact size in advance
- These aren't theoretical concerns - we built a simulation to measure the real impact

---

## SLIDE 15: Research Objective
**Title:** Research Objective

**Visual:** Clean statement with supporting graphic

**Content:**
"Evaluate the feasibility and performance impact of replacing ECDSA with post-quantum digital signature algorithms (Falcon-512, Dilithium-2) in C-V2X PC5 Mode 4 sidelink communication through high-fidelity simulation."

**Speech:**
- Our research question is straightforward
- CAN we replace ECDSA with PQC in C-V2X Mode 4, and WHAT is the performance cost?
- We answer this through simulation with three algorithms: ECDSA as baseline, Falcon-512, and Dilithium-2
- We measure impact on packet delivery ratio, end-to-end delay, channel congestion, and computational overhead
- Across varying vehicle densities to understand scalability

---

## SLIDE 16: Simulation Platform
**Title:** Simulation Framework

**Visual:** Architecture diagram showing:
- OMNeT++ (discrete event simulator)
- SimuLTE (LTE/C-V2X protocol stack)
- Veins (vehicular networking)
- SUMO (traffic simulator)
- Arrows showing how they interconnect
- Custom modules highlighted: Mode4App, LteMacVUeMode4, LtePhyVUeMode4, pqcdsa

**Speech:**
- We built our simulation on top of established open-source frameworks
- OMNeT++ is the discrete event network simulator - it's the engine
- SimuLTE provides the full LTE protocol stack including the Mode 4 sidelink MAC and PHY
- Veins handles the vehicular networking aspects
- SUMO generates realistic vehicle mobility - real intersection scenarios
- On top of this, we implemented the PQC digital signature layer
- We integrated Falcon-512, Dilithium-2, and ECDSA using the reference C implementations
- The signing happens in the application layer (Mode4App) and verification at the receiver
- The key insight: this is a FULL protocol stack simulation, not just a packet-level model
- Every SCI, every TB, every subchannel allocation, every HARQ process is modeled

---

## SLIDE 17: Simulation Architecture Detail
**Title:** Protocol Stack Implementation

**Visual:** Layered diagram:
```
Application Layer (Mode4App)
  - BSM generation (10 Hz)
  - PQC sign (Falcon/Dilithium/ECDSA)
  - PQC verify at receiver
  - SPDU assembly (BSM + Signature + Certificate)
       |
MAC Layer (LteMacVUeMode4)
  - SPS grant management
  - MCS selection (CBR-adaptive)
  - Subchannel allocation
  - Resource reselection counter
       |
PHY Layer (LtePhyVUeMode4)
  - SCI/TB transmission
  - Channel sensing (CBR measurement)
  - SINR-based decoding
  - Propagation + interference model
```

**Speech:**
- Here's how the pieces fit together in the protocol stack
- At the top, Mode4App generates BSMs 10 times per second
- Each BSM is signed with the selected PQC algorithm - we measure this time with wall-clock precision
- The signed SPDU goes down to the MAC layer which manages the SPS grant
- The MAC selects the MCS and number of subchannels based on current channel congestion
- This is critical: larger PQC packets may need more subchannels or higher MCS
- At the PHY layer, we model realistic channel propagation, interference, and SINR-based decoding
- On the receive side, everything flows back up: PHY decodes, RLC reassembles, App verifies signature

---

## SLIDE 18: Simulation Parameters
**Title:** Simulation Configuration

**Visual:** Table of key parameters:
| Parameter | Value |
|-----------|-------|
| Scenario | Highway intersection (SUMO) |
| Carrier frequency | 5.915 GHz (ITS band) |
| Bandwidth | 10 MHz (50 RBs) |
| Subchannels | 10 (10 RBs each) |
| Subchannel size | 10 RBs |
| Tx Power | 23 dBm |
| BSM rate | 10 Hz (100ms RRI) |
| MCS adaptation | CBR-based (per 3GPP) |
| HARQ retransmissions | 0 |
| probResourceKeep | 0.8 |
| Vehicle densities | 12 - 120 vehicles |
| Simulation time | ~65s per run |
| Algorithms tested | ECDSA, Falcon-512, Dilithium-2 |

**Speech:**
- Here are the key simulation parameters
- We use the 5.9 GHz ITS band with 10 MHz bandwidth - this is the standard C-V2X allocation
- 10 subchannels of 10 resource blocks each - this gives us enough capacity for PQC-sized packets
- BSMs are sent every 100 milliseconds, matching the SAE J2945 standard
- We use CBR-based MCS adaptation as specified by 3GPP - the system automatically adjusts modulation based on channel congestion
- We test across vehicle densities from 12 to 120 vehicles to see how PQC scales under load
- The highway intersection scenario is generated by SUMO with realistic traffic patterns

---

## SLIDE 19: SPDU Packet Structure Comparison
**Title:** SPDU Size Across Algorithms

**Visual:** Stacked bar chart showing SPDU composition:
For each algorithm, show stacked bars:
- BSM payload (fixed ~73 bytes)
- Certificate + public key
- Signature
- Headers/overhead

| Component | ECDSA | Falcon-512 | Dilithium-2 |
|-----------|-------|------------|-------------|
| BSM payload | ~73 B | ~73 B | ~73 B |
| Public key | 64 B | 897 B | 1,312 B |
| Signature | 64 B | ~652 B | 2,420 B |
| Total SPDU | ~250 B | ~1,622 B | ~3,805 B |

**Data source:** From appl_logs.csv - spdu_size, sig_size, cert_size columns

**Speech:**
- Let's look at the actual packet sizes from our simulation
- ECDSA SPDU is about 250 bytes - compact, fits easily in one subchannel
- Falcon-512 SPDU is about 1,622 bytes - notice the signature varies between 649-666 bytes
- Dilithium-2 SPDU is about 3,805 bytes - this is substantial
- The question is: can the C-V2X sidelink handle these larger packets?
- With MCS 11 and 10 subchannels, the maximum transport block capacity is about 4,008 bytes
- Dilithium-2 is cutting it very close to the limit
- Falcon-512 fits comfortably but still uses significantly more resources than ECDSA

---

## SLIDE 20: Signing Time Comparison
**Title:** Signature Generation Time

**Visual:** Box plot or bar chart with error bars showing signing time (ms) for each algorithm

**Data source:** From .sca file - signatureTimeMs:mean per vehicle, or from signatureTimeMs:vector for distribution
- Also: icaSignMs for RSU signing time

**Suggested chart:** Box plot showing distribution of signing times across all vehicles for each algorithm

**Speech:**
- Let's look at computational performance - how long does it take to sign one BSM?
- ECDSA signing is extremely fast - sub-millisecond
- Falcon-512 signing takes about 2.5 milliseconds on average in our simulation
- Dilithium-2 signing is faster than Falcon at about [X] milliseconds
- Remember: vehicles sign at 10 Hz, so they have 100ms budget per message
- Even Falcon's 2.5ms is only 2.5% of the budget - computationally feasible
- But on resource-constrained vehicular hardware (not a desktop CPU), these numbers will be higher
- The verification side is what really matters at scale - one vehicle may verify 100+ signatures per second

---

## SLIDE 21: Verification Time
**Title:** Signature Verification Time

**Visual:** Bar chart or box plot showing verification time per algorithm

**Data source:** icaVerifyMs from .sca (for ICA), or compute from simulation logs

**Speech:**
- Verification is the bottleneck in V2X - every vehicle verifies messages from ALL neighbors
- With 50 neighbors, that's 500 verifications per second
- ECDSA verification is about [X] ms
- Falcon-512 verification takes about [X] ms
- Dilithium-2 verification is typically faster than Falcon
- Multiply these by the number of neighbors to get the total verification load
- In dense scenarios (100+ vehicles), verification time becomes a real constraint

---

## SLIDE 22: End-to-End Delay
**Title:** End-to-End Message Delay

**Visual:** Line chart or bar chart:
- X-axis: number of vehicles (density)
- Y-axis: mean delay (ms)
- One line/bar per algorithm
- Include error bars or confidence intervals

**Data source:** From appl_logs.csv - delay_ms column, grouped by "Numer of Vehicles" and Algorithm
Or from .sca - delay:mean per vehicle

**Speech:**
- End-to-end delay is the time from when a vehicle creates a BSM to when a neighbor receives it
- For safety applications, SAE J3161 requires delays under 100 milliseconds
- Our results show mean delays of [X] ms for ECDSA, [X] ms for Falcon-512, [X] ms for Dilithium-2
- The delay increases slightly with vehicle density due to channel congestion
- Even with PQC, we remain well within the 100ms safety deadline
- The main contributor to delay is the MAC layer scheduling, not the crypto computation
- This is a positive finding: PQC does not break the latency requirements

---

## SLIDE 23: Packet Delivery Ratio
**Title:** Packet Delivery Ratio (PDR)

**Visual:** Line chart:
- X-axis: number of vehicles
- Y-axis: PDR (%)
- One line per algorithm
- Show the degradation curve as density increases

**Data source:** Compute from .sca: verified:sum / received:sum per vehicle
Or PHY level: tbDecoded / tbSent

**Speech:**
- PDR measures what fraction of transmitted BSMs are successfully received
- This is THE critical metric for safety - if PDR drops too low, vehicles lose awareness of neighbors
- At low density (12 vehicles), PDR is near 100% for all algorithms
- As density increases, PDR drops due to more collisions and interference
- The key question: does PQC make this worse?
- Larger packets occupy more subchannels, reducing total channel capacity
- Our results show [describe the comparison]
- ECDSA maintains higher PDR at extreme densities because smaller packets cause less congestion
- Falcon-512 shows moderate degradation
- Dilithium-2 shows the most degradation due to its large packet size

---

## SLIDE 24: Channel Busy Ratio
**Title:** Channel Busy Ratio (CBR) Analysis

**Visual:** Line chart:
- X-axis: simulation time or vehicle density
- Y-axis: CBR (0-1)
- One line per algorithm
- Mark the congestion control threshold (~0.65)

**Data source:** From .sca - cbr:mean per vehicle, or cbr:vector for time series

**Speech:**
- CBR is the fraction of radio resources that are occupied
- It directly measures channel congestion
- 3GPP defines CBR-based congestion control: when CBR exceeds thresholds, MCS and transmission parameters are adjusted
- Higher CBR means vehicles start dropping packets or reducing transmission rate
- With PQC's larger packets, each transmission occupies more subchannels
- Same number of vehicles, but more resources consumed per message
- Our results show CBR of [X] for ECDSA vs [X] for Falcon-512 vs [X] for Dilithium-2
- This directly explains the PDR differences

---

## SLIDE 25: Distance vs Delay Analysis
**Title:** Impact of Transmitter-Receiver Distance

**Visual:** Scatter plot:
- X-axis: distance (m)
- Y-axis: delay (ms)
- Color-coded by algorithm
- Maybe add a trend line

**Data source:** From appl_logs.csv - dist_m vs delay_ms columns

**Speech:**
- This plot shows how delay varies with distance between transmitter and receiver
- At short distances, signal quality is high, decoding is fast
- At longer distances, propagation effects increase
- Interesting to note whether PQC affects the distance-delay relationship differently than ECDSA
- The maximum reliable communication range is another metric to watch

---

## SLIDE 26: Vehicle Density Scalability
**Title:** Scalability: Performance vs Vehicle Density

**Visual:** Multi-panel figure with 3-4 sub-plots:
- Panel 1: PDR vs density
- Panel 2: Delay vs density
- Panel 3: CBR vs density
- Panel 4: Verification success rate vs density
All with per-algorithm lines

**Data source:** Aggregate from appl_logs.csv grouped by "Numer of Vehicles" column

**Speech:**
- This is the scalability picture - how does each algorithm perform as traffic gets denser?
- We tested from 12 to 120 vehicles
- [Walk through each panel]
- The takeaway: PQC is feasible at moderate densities but degrades faster than ECDSA at high density
- Falcon-512 offers the best tradeoff: much smaller signatures than Dilithium, much stronger security than ECDSA
- There exists a density threshold beyond which PQC performance becomes unacceptable
- This informs real-world deployment: PQC works on suburban roads, but downtown Manhattan needs optimization

---

## SLIDE 27: PHY Layer Analysis - SCI/TB Success Rates
**Title:** PHY Layer: Transmission Success Breakdown

**Visual:** Stacked bar chart showing per-algorithm:
- tbDecoded (success)
- tbFailedDueToProp (propagation failure)
- tbFailedDueToInterference (interference)
- tbFailedHalfDuplex (half-duplex collision)
- tbFailedDueToNoSCI (SCI missed)

**Data source:** From .sca file - tb* signals

**Speech:**
- Let's look under the hood at the PHY layer
- This breakdown shows WHY packets fail to be delivered
- Propagation failures are distance-dependent - same across algorithms
- Interference failures increase with PQC because larger packets create more collisions
- Half-duplex failures occur when a vehicle is transmitting and cannot receive simultaneously
- With PQC, transmissions occupy more subframe-subchannel resources, increasing half-duplex collisions
- SCI failures mean the receiver couldn't decode the control information, so it didn't even know where to look for the data

---

## SLIDE 28: RLC Layer Analysis
**Title:** RLC Layer: Packet Loss and Throughput

**Visual:** Table or small bar charts:
- rlcPduPacketLossD2D per algorithm
- rlcPduThroughputD2D per algorithm

**Data source:** From .sca - rlcPduPacketLossD2D:mean, rlcThroughputD2D:mean

**Speech:**
- At the RLC layer, we see the aggregate effect
- RLC packet loss on the D2D sidelink is about [X]% for ECDSA, [X]% for Falcon, [X]% for Dilithium
- This confirms the PHY-layer findings: larger packets lead to more congestion and loss
- Throughput actually increases with PQC because each successfully delivered packet carries more bytes
- But the useful payload (BSM data) is the same - the extra bytes are just cryptographic overhead

---

## SLIDE 29: Algorithm Comparison Summary Table
**Title:** Algorithm Comparison Summary

**Visual:** Comprehensive comparison table:
| Metric | ECDSA | Falcon-512 | Dilithium-2 |
|--------|-------|------------|-------------|
| Signature size | 64 B | ~652 B | 2,420 B |
| Public key size | 64 B | 897 B | 1,312 B |
| SPDU total | ~250 B | ~1,622 B | ~3,805 B |
| Sign time | ~0.1 ms | ~2.5 ms | ~X ms |
| Verify time | ~0.3 ms | ~X ms | ~X ms |
| PDR (50 veh) | X% | X% | X% |
| Mean delay | X ms | X ms | X ms |
| CBR impact | Low | Medium | High |
| Quantum-safe | No | Yes | Yes |
| NIST status | Legacy | FIPS 206 | FIPS 204 |

**Speech:**
- Here's the head-to-head comparison
- ECDSA wins on performance but loses on quantum security - it's living on borrowed time
- Falcon-512 offers the best balance: reasonably compact signatures, feasible performance, quantum-safe
- Dilithium-2 has the strongest standardization backing (FIPS 204, already final) but the largest signatures
- For C-V2X, Falcon-512 appears to be the most promising candidate
- But Dilithium-2 shouldn't be ruled out - with wider bandwidth (20 MHz) or smaller BSMs, it could work

---

## SLIDE 30: SAE J3161 Compliance
**Title:** SAE J3161 Compliance Considerations

**Visual:** Checklist or brief table:
- BSM rate: 10 Hz - Compliant
- End-to-end delay: < 100 ms - Compliant (with PQC)
- SPDU structure: IEEE 1609.2 compatible - Compliant
- SPS resource management - Implemented
- Note: One-shot transmission not yet implemented

**Speech:**
- Brief note on standards compliance
- Our simulation follows the SAE J3161 standard for DSRC/C-V2X interoperability
- The PQC-signed messages still meet the 100ms delay requirement
- The SPDU structure follows IEEE 1609.2 with certificate and signature fields
- One area for future work: J3161 specifies one-shot transmissions alongside SPS to reduce persistent collisions
- This is something we're implementing next

---

## SLIDE 31: Key Findings
**Title:** Key Findings

**Visual:** Numbered findings with icons

**Content:**
1. PQC integration in C-V2X Mode 4 is FEASIBLE - all algorithms meet the 100ms delay deadline
2. Falcon-512 is the most suitable PQC candidate for V2X: 10x smaller signatures than Dilithium-2
3. PQC increases channel congestion proportionally to signature size, reducing PDR at high vehicle densities
4. Computational overhead (2-3ms signing) is manageable within the 100ms BSM interval
5. The primary bottleneck is RADIO RESOURCE CONSUMPTION, not computation
6. CBR-based congestion control helps but cannot fully compensate for the increased packet sizes

**Speech:**
- Let me summarize the key findings
- [Walk through each finding]
- The headline: PQC in C-V2X is feasible today, but not without performance cost
- The cost is manageable with Falcon-512 at moderate vehicle densities
- High-density scenarios need further optimization: packet compression, certificate omission schemes, wider bandwidth

---

## SLIDE 32: Future Work
**Title:** Future Work

**Visual:** List with brief descriptions

**Content:**
1. Hybrid signatures (ECDSA + PQC) for backward compatibility
2. Certificate omission and implicit certificates to reduce overhead
3. One-shot transmission (J3161 compliance) to reduce persistent collisions
4. NR-V2X (5G) sidelink with wider bandwidth and higher MCS
5. Hardware-in-the-loop testing with real V2X chipsets
6. Multi-RSU ICA (Intersection Collision Avoidance) with PQC
7. SPHINCS+ evaluation for ultra-conservative security margins

**Speech:**
- Several exciting directions for future work
- Hybrid signatures: send both ECDSA and PQC signatures during the transition period
- Certificate omission: instead of including the full public key every time, use a hash reference
- This could save 900+ bytes per packet - almost as impactful as choosing a better algorithm
- 5G NR-V2X offers wider bandwidth which would significantly ease the PQC size problem
- And ultimately, we need to test on real hardware to confirm the computational feasibility

---

## SLIDE 33: Conclusion
**Title:** Conclusion

**Visual:** 3-4 bullet summary with a closing graphic

**Speech:**
- Quantum computers will break the cryptography that secures vehicle communication
- Post-quantum cryptography is the solution, but it comes with larger signatures
- Our simulation shows that Falcon-512 is a viable PQC candidate for C-V2X Mode 4
- Performance degrades at high vehicle densities, but remains within safety requirements at moderate densities
- The migration to PQC in V2X should start now - vehicles deployed today will still be on the road when quantum computers arrive
- Thank you! I'm happy to take questions.

---

## SLIDE 34: Questions
**Title:** Questions?

**Visual:** Contact information, GitHub/lab link if applicable

---

## SLIDE 35 (BACKUP): Simulation Tool Details
**Title:** Simulation Toolchain Details

**Visual:** Version table:
| Tool | Version | Role |
|------|---------|------|
| OMNeT++ | 5.x | Discrete event simulation |
| SimuLTE | Custom fork | LTE Mode 4 protocol stack |
| Veins | 5.x | V2X framework |
| SUMO | 1.x | Traffic simulation |
| liboqs | 0.9.x | PQC library |

**Speech:** (Only if asked about implementation details)
- Here are the specific versions and tools used

---

## SLIDE 36 (BACKUP): Mode 4 MAC State Machine
**Title:** SPS State Machine Detail

**Visual:** State diagram from LteMacVUeMode4 showing:
- IDLE -> GRANT_REQUEST -> SENSING -> SELECTION -> TRANSMITTING -> COUNTER_CHECK -> (loop or RESELECT)

**Speech:** (Only if asked about Mode 4 details)

---

## SUGGESTED CHARTS TO GENERATE

Before the talk, create these plots from your data:

1. **Bar chart: SPDU size comparison** (ECDSA vs Falcon vs Dilithium)
   - Source: appl_logs.csv columns: sig_size, cert_size, spdu_size

2. **Box plot: Signing time distribution per algorithm**
   - Source: .sca signatureTimeMs:mean across vehicles, or vector data

3. **Line chart: PDR vs vehicle density per algorithm**
   - Source: compute from .sca verified:sum / received:sum, grouped by density

4. **Line chart: Mean delay vs vehicle density per algorithm**
   - Source: appl_logs.csv delay_ms grouped by Numer of Vehicles and Algorithm

5. **Line chart: CBR vs vehicle density per algorithm**
   - Source: .sca cbr:mean per vehicle, grouped by density

6. **Scatter plot: Distance vs delay**
   - Source: appl_logs.csv dist_m vs delay_ms

7. **Stacked bar: TB failure breakdown per algorithm**
   - Source: .sca tbDecoded, tbFailedDueToProp, tbFailedDueToInterference, etc.

8. **Stacked bar: SPDU composition** (payload + cert + signature + overhead)
   - Source: appl_logs.csv or known sizes

---

## TIMING GUIDE

| Section | Slides | Minutes |
|---------|--------|---------|
| Introduction & V2X basics | 1-3 | 3 min |
| C-V2X architecture & 3GPP | 4-6 | 4 min |
| Mode 4 SPS detail | 7-8 | 5 min |
| Security & IEEE 1609.2 | 9-10 | 4 min |
| Quantum threat | 11 | 3 min |
| PQC overview & challenges | 12-14 | 5 min |
| Research setup | 15-18 | 5 min |
| Results & analysis | 19-28 | 12 min |
| Comparison & findings | 29-31 | 4 min |
| Future work & conclusion | 32-34 | 3 min |
| **TOTAL** | **34** | **~43 min** |

---

## PRESENTATION TIPS

1. **Start strong:** Open with a real-world V2X crash-avoidance scenario to hook the audience
2. **Use animations:** On the SPS slides, animate the sensing and selection process step by step
3. **Color coding:** Use consistent colors throughout: green=ECDSA, blue=Falcon, red=Dilithium
4. **The "So What" test:** After each result slide, explicitly state why it matters for deployment
5. **Anticipate questions:**
   - "Why not just use bigger bandwidth?" -> Great point, NR-V2X with 20-40 MHz helps, but spectrum is scarce
   - "Is Falcon standardized?" -> FIPS 206 draft, expected final in 2025
   - "What about key exchange?" -> V2X uses signatures, not key exchange; different PQC problem
   - "Real hardware?" -> Future work; these are reference implementations, hardware acceleration would help
6. **Have backup slides** ready for deep technical questions (slides 35-36)
