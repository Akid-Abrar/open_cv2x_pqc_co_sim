#!/usr/bin/env python3
"""
Generate a PDF document listing all OMNeT++ result parameters
(scalars and vectors) from the SimuLTE Mode4 simulation.
"""

from fpdf import FPDF

class ResultParamsPDF(FPDF):
    def header(self):
        self.set_font("Helvetica", "B", 14)
        self.cell(0, 10, "SimuLTE Mode4 - Simulation Result Parameters Reference", new_x="LMARGIN", new_y="NEXT", align="C")
        self.ln(2)

    def footer(self):
        self.set_y(-15)
        self.set_font("Helvetica", "I", 8)
        self.cell(0, 10, f"Page {self.page_no()}/{{nb}}", align="C")

    def section_title(self, title):
        self.set_font("Helvetica", "B", 11)
        self.set_fill_color(220, 230, 241)
        self.cell(0, 8, title, new_x="LMARGIN", new_y="NEXT", fill=True)
        self.ln(2)

    def section_body(self, text):
        self.set_font("Helvetica", "", 9)
        self.multi_cell(0, 5, text)
        self.ln(2)

    def add_table(self, headers, data, col_widths):
        # Header row
        self.set_font("Helvetica", "B", 7)
        self.set_fill_color(41, 65, 122)
        self.set_text_color(255, 255, 255)
        for i, h in enumerate(headers):
            self.cell(col_widths[i], 7, h, border=1, fill=True, align="C")
        self.ln()
        self.set_text_color(0, 0, 0)

        # Data rows
        self.set_font("Helvetica", "", 6.5)
        row_alt = False
        for row in data:
            if row_alt:
                self.set_fill_color(240, 245, 250)
            else:
                self.set_fill_color(255, 255, 255)

            # Calculate max height needed for this row
            max_lines = 1
            for i, cell_text in enumerate(row):
                lines = self.multi_cell(col_widths[i], 4, cell_text, dry_run=True, output="LINES")
                max_lines = max(max_lines, len(lines))
            row_h = max(max_lines * 4, 5)

            x_start = self.get_x()
            y_start = self.get_y()

            # Check page break
            if y_start + row_h > self.h - 20:
                self.add_page()
                # Re-draw header
                self.set_font("Helvetica", "B", 7)
                self.set_fill_color(41, 65, 122)
                self.set_text_color(255, 255, 255)
                for i, h in enumerate(headers):
                    self.cell(col_widths[i], 7, h, border=1, fill=True, align="C")
                self.ln()
                self.set_text_color(0, 0, 0)
                self.set_font("Helvetica", "", 6.5)
                if row_alt:
                    self.set_fill_color(240, 245, 250)
                else:
                    self.set_fill_color(255, 255, 255)
                x_start = self.get_x()
                y_start = self.get_y()

            for i, cell_text in enumerate(row):
                self.set_xy(x_start + sum(col_widths[:i]), y_start)
                self.rect(x_start + sum(col_widths[:i]), y_start, col_widths[i], row_h, style="DF")
                self.set_xy(x_start + sum(col_widths[:i]) + 0.5, y_start + 0.5)
                self.multi_cell(col_widths[i] - 1, 4, cell_text)

            self.set_xy(x_start, y_start + row_h)
            row_alt = not row_alt


def main():
    pdf = ResultParamsPDF(orientation="L", format="A4")
    pdf.alias_nb_pages()
    pdf.set_auto_page_break(auto=True, margin=15)
    pdf.add_page()

    # --- Introduction ---
    pdf.section_title("1. Overview")
    pdf.section_body(
        "This document catalogues every scalar and vector result parameter recorded during "
        "SimuLTE Mode 4 (C-V2X) simulations. For each parameter the table lists:\n"
        "  - The signal name as it appears in the .sca/.vec result files\n"
        "  - The recording type (scalar aggregation or time-series vector)\n"
        "  - The OMNeT++ module that emits the signal\n"
        "  - The source file and approximate line number where the signal is emitted\n"
        "  - A description of what the parameter represents and how the value is computed\n\n"
        "Result files analysed:\n"
        "  simulations/Mode4/results/Base-#0.sca  (scalars)\n"
        "  simulations/Mode4/results/Base-#0.vec  (vectors)\n\n"
        "Simulation configuration: Highway network, Falcon-512 PQC signatures, "
        "10 subchannels x 10 RBs each, RRI=100 ms, CBR-based MCS/subchannel adaptation."
    )

    # --- Architecture note ---
    pdf.section_title("2. Protocol Stack Layers")
    pdf.section_body(
        "The parameters span four layers of the C-V2X Mode 4 protocol stack:\n\n"
        "  Application Layer (Mode4App / Mode4RSUApp)\n"
        "    - BSM/SPDU generation, PQC signature & verification, PDR tracking, ICA warnings.\n\n"
        "  MAC Layer (LteMacVUeMode4)\n"
        "    - Semi-Persistent Scheduling (SPS), grant management, resource (re)selection,\n"
        "      MCS selection, subchannel allocation, DCC packet dropping.\n\n"
        "  RLC Layer (UmRxEntity / UmRxQueue)\n"
        "    - Unacknowledged-mode segmentation/reassembly, SDU/PDU packet loss,\n"
        "      delay, and throughput measurement per direction (UL/DL/D2D).\n\n"
        "  PHY Layer (LtePhyVUeMode4)\n"
        "    - SCI/TB transmission & reception, channel sensing (CBR),\n"
        "      propagation/interference/half-duplex failure tracking,\n"
        "      awareness ratio, inter-packet delay, position reporting.\n\n"
        "  Veins Mobility (TraCIMobility)\n"
        "    - Vehicle position, speed, CO2, lifetime  (from SUMO via TraCI)."
    )

    # -----------------------------------------------------------------
    # TABLE 1 : APPLICATION LAYER
    # -----------------------------------------------------------------
    pdf.section_title("3. Application Layer Parameters (Mode4App / Mode4RSUApp)")
    headers = ["Parameter Name", "Recording", "Unit", "Module", "Source File : Line", "Description"]
    col_widths = [38, 22, 12, 32, 55, 118]

    app_data = [
        ["sentMsg", "sum, vector", "", "Mode4App", "Mode4App.cc : 564", "Incremented by 1 each time a BSM/SPDU packet is broadcast. Total count of V2X messages sent by this vehicle."],
        ["received", "sum, vector", "", "Mode4App", "Mode4App.cc : 390", "Incremented by 1 for every SPDU received from another vehicle. Counts all receptions regardless of verification outcome."],
        ["verified", "sum, vector", "s", "Mode4App", "Mode4App.cc : 411", "Incremented by 1 when the PQC signature on a received SPDU passes verification. Used to compute verification success rate."],
        ["delay", "mean, vector", "s", "Mode4App", "Mode4App.cc : 389", "One-way end-to-end latency: simTime() minus the SPDU timestamp set by the sender. Measures total application-to-application delay."],
        ["cbr (app)", "vector", "", "Mode4App", "Mode4App.cc : 144", "Channel Busy Ratio forwarded from the PHY-layer CBR measurement packet. Fraction of subchannels sensed as occupied (0.0-1.0)."],
        ["lifetime", "mean, vector", "s", "Mode4App", "Mode4App.cc : 570", "Duration the vehicle existed in the simulation: simTime() - entryTime. Recorded at vehicle departure (finish())."],
        ["signatureTimeMs", "mean, vector", "ms", "Mode4App", "Mode4App.cc : 507", "Wall-clock time to generate one PQC digital signature (Falcon-512/Dilithium-2). Measured with chrono::high_resolution_clock."],
        ["warnReceived", "sum, vector", "", "Mode4App", "Mode4App.cc : 233", "Count of ICA (Intersection Collision Avoidance) warning SPDUs received from the RSU."],
        ["warnVerified", "sum, vector", "", "Mode4App", "Mode4App.cc : 315", "Count of ICA warnings whose PQC signature was successfully verified."],
        ["warnExpected", "sum, vector", "", "Mode4App", "Mode4App.cc : 327", "Expected number of ICA warnings based on sequence-number gap detection. Used to compute ICA PDR."],
        ["icaVerifyMs", "mean, vector", "ms", "Mode4App", "Mode4App.cc : 314", "Wall-clock time to verify one ICA warning signature. Measured with chrono in microseconds, emitted as ms."],
        ["icaDelayMs", "mean, vector", "ms", "Mode4App", "Mode4App.cc (scalar)", "One-way delay for ICA warning messages: simTime() - warn timestamp. Recorded as scalar at finish."],
        ["icaReceived", "scalar", "", "Mode4App", "Mode4App.cc : finish()", "Total ICA warnings received during vehicle lifetime. Recorded as scalar in finish()."],
        ["icaExpected", "scalar", "", "Mode4App", "Mode4App.cc : finish()", "Total ICA warnings expected during vehicle lifetime. Recorded as scalar in finish()."],
        ["icaPDR", "scalar", "", "Mode4App", "Mode4App.cc : finish()", "ICA Packet Delivery Ratio = icaReceived / icaExpected. Recorded as scalar in finish()."],
        ["rsuReceivedMsg", "sum, vector", "", "Mode4RSUApp", "Mode4RSUApp.cc : 318", "Incremented by 1 for every SPDU received at the RSU node."],
        ["rsuVerifiedMsg", "sum, vector", "", "Mode4RSUApp", "Mode4RSUApp.cc : 358", "Incremented by 1 when the RSU successfully verifies a received SPDU's PQC signature."],
        ["numBroadcasted", "sum", "", "Mode4RSUApp", "Mode4RSUApp.cc : 266", "Count of ICA warning SPDUs broadcast by the RSU over the sidelink."],
        ["icaSignMs", "mean", "ms", "Mode4RSUApp", "Mode4RSUApp.cc : 239", "Wall-clock time for the RSU to sign one ICA warning with its PQC private key."],
        ["cbr (rsu)", "mean", "", "Mode4RSUApp", "Mode4RSUApp.cc : 313", "Channel Busy Ratio as observed at the RSU node. Forwarded from PHY CBR packet."],
    ]
    pdf.add_table(headers, app_data, col_widths)
    pdf.ln(4)

    # -----------------------------------------------------------------
    # TABLE 2 : MAC LAYER
    # -----------------------------------------------------------------
    pdf.add_page()
    pdf.section_title("4. MAC Layer Parameters (LteMacVUeMode4)")
    mac_data = [
        ["macNodeID", "vector", "", "LteMacVUeMode4", "LteMacVUeMode4.cc : 114", "The MAC-layer node identifier assigned to this UE. Emitted once at initialization; allows mapping vector indices to node IDs."],
        ["grantRequests", "sum, vector", "", "LteMacVUeMode4", "LteMacVUeMode4.cc : 1140", "Incremented each time the MAC requests a new SPS grant from the resource selection procedure (Section 14.1.1.6 of 3GPP 36.321)."],
        ["grantStartTime", "sum, vector", "", "LteMacVUeMode4", "LteMacVUeMode4.cc : 885", "The absolute simulation time at which the selected CSR (Candidate Single-subframe Resource) grant begins."],
        ["grantBreak", "sum, vector", "", "LteMacVUeMode4", "LteMacVUeMode4.cc : 749", "Emitted when the SPS resource reservation counter reaches zero and the grant expires. Triggers resource reselection."],
        ["grantBreakTiming", "sum, vector", "", "LteMacVUeMode4", "LteMacVUeMode4.cc : 644", "Emitted when a grant breaks because the timing constraint could not be met (grant start time already passed)."],
        ["grantBreakSize", "count, sum", "bytes", "LteMacVUeMode4", "LteMacVUeMode4.cc : 1295", "Emitted with the PDU payload length (bytes) when the transport block exceeds the capacity of the selected MCS/subchannel combination."],
        ["grantBreakMissedTrans", "sum", "", "LteMacVUeMode4", "LteMacVUeMode4.cc : 1329", "Emitted when a scheduled transmission opportunity is missed (e.g., no data ready in the buffer at grant time)."],
        ["missedTransmission", "sum", "", "LteMacVUeMode4", "LteMacVUeMode4.cc : 1320", "Counter of all missed transmission slots regardless of cause."],
        ["resourceReselectionCounter", "sum, vector", "", "LteMacVUeMode4", "LteMacVUeMode4.cc : 733", "Current value of the SPS Resource Reselection Counter (C_resel). Counts down each RRI; at zero, resource reselection is triggered with probability (1 - probResourceKeep)."],
        ["retainGrant", "sum, vector", "", "LteMacVUeMode4", "LteMacVUeMode4.cc : 734", "Emitted when the UE retains its current grant (counter did not expire, or random draw kept the grant at counter=0)."],
        ["selectedMCS", "mean, vector", "", "LteMacVUeMode4", "LteMacVUeMode4.cc : 1282", "The Modulation and Coding Scheme index (0-28) chosen for the current transmission, based on CBR-to-MCS lookup table from sidelink_configuration.xml."],
        ["selectedSubchannelIndex", "mean, vector", "", "LteMacVUeMode4", "LteMacVUeMode4.cc : 892", "Starting subchannel index (0 to numSubchannels-1) of the allocated resource within the subframe."],
        ["selectedNumSubchannels", "mean, vector", "", "LteMacVUeMode4", "LteMacVUeMode4.cc : 893", "Number of contiguous subchannels allocated to this transmission (1 to numSubchannels)."],
        ["maximumCapacity", "mean", "bytes", "LteMacVUeMode4", "LteMacVUeMode4.cc : 1296", "Maximum transport block payload capacity (bytes) for the selected MCS and number of subchannels. If the PDU exceeds this, the grant breaks."],
        ["takingReservedGrant", "mean, vector", "", "LteMacVUeMode4", "LteMacVUeMode4.cc : 894", "Boolean flag (0 or 1): whether the selected CSR came from the reserved (previously-used) pool rather than a fresh random selection."],
        ["packetDropDCC", "sum", "", "LteMacVUeMode4", "LteMacVUeMode4.cc : 1179", "Incremented when the DCC (Decentralized Congestion Control) mechanism drops a packet to reduce channel load."],
        ["droppedTimeout", "sum", "", "LteMacVUeMode4", "LteMacVUeMode4.cc : ~1300", "Packets dropped because they exceeded the MAC buffer lifetime. Currently commented out in code."],
    ]
    pdf.add_table(headers, mac_data, col_widths)
    pdf.ln(4)

    # -----------------------------------------------------------------
    # TABLE 3 : PHY LAYER
    # -----------------------------------------------------------------
    pdf.add_page()
    pdf.section_title("5. PHY Layer Parameters (LtePhyVUeMode4)")

    phy_data = [
        ["servingCell", "vector", "", "LtePhyUe", "LtePhyUe.cc : 153", "MAC node ID of the serving eNodeB. In Mode 4 (out-of-coverage) this is a virtual cell ID."],
        ["cbr (phy)", "mean, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 1872", "Channel Busy Ratio: ratio of subchannels whose RSSI exceeded the sensing threshold over the last 100 ms sensing window."],
        ["cbrPscch", "mean, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 1873", "CBR measured only on PSCCH (Physical Sidelink Control Channel) resources. Indicates control-channel congestion."],
        ["threshold", "mean, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 179", "RSRP sensing threshold used to exclude candidate resources during SPS selection (3GPP 36.213 Section 14.1.1.6). Increased iteratively if fewer than 20% of candidates remain."],
        ["sciSent", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 629", "Count of Sidelink Control Information (SCI) messages transmitted. One SCI is sent per subframe when the UE has an active grant."],
        ["sciReceived", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 267", "Cumulative count of SCI messages received from all neighboring UEs."],
        ["sciDecoded", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 269", "Cumulative count of SCI messages successfully decoded (SINR above SCI decoding threshold)."],
        ["sciUnsensed", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 268", "SCI messages not sensed because the received power was below the RSRP threshold (pThresh)."],
        ["sciFailedDueToProp", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 270", "SCI decoding failures caused by propagation loss (signal too weak due to path loss, fading)."],
        ["sciFailedDueToInterference", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 271", "SCI decoding failures caused by co-channel interference from other simultaneous transmissions."],
        ["sciFailedHalfDuplex", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 272", "SCI messages missed because the UE was transmitting at the same time (half-duplex constraint: cannot TX and RX simultaneously)."],
        ["txRxDistanceSCI", "mean, vector", "m", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 1504", "Euclidean distance (meters) between the SCI transmitter and this receiver at the time of reception."],
        ["tbSent", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 526", "Count of Transport Blocks (data payload) transmitted by this UE."],
        ["tbReceived", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 293", "Cumulative count of Transport Blocks received from all neighbors."],
        ["tbDecoded", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 294", "Transport Blocks successfully decoded (SINR above TB decoding threshold)."],
        ["tbFailedDueToNoSCI", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 295", "TB decoding failures because the corresponding SCI was not received (cannot determine TB resource allocation without SCI)."],
        ["tbFailedButSCIReceived", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 298", "TB failed to decode even though its SCI was successfully received. Indicates data-channel SINR was insufficient."],
        ["tbAndSCINotReceived", "sum", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 151", "Both TB and SCI not received (complete miss). Neither control nor data was decoded."],
        ["tbFailedHalfDuplex", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 299", "TB missed due to half-duplex: UE was transmitting when the TB arrived."],
        ["tbFailedDueToProp", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 296", "TB decoding failures due to propagation loss (path loss + fading)."],
        ["tbFailedDueToInterference", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 297", "TB decoding failures due to co-channel interference."],
        ["tbFailedDueToPropIgnoreSCI", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 302", "TB propagation failures counted regardless of SCI status. Used for analysis that decouples control- and data-channel."],
        ["tbFailedDueToInterferenceIgnoreSCI", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 303", "TB interference failures counted regardless of SCI status."],
        ["tbDecodedIgnoreSCI", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 304", "TB successfully decoded regardless of SCI status. Measures raw data-channel reliability."],
        ["txRxDistanceTB", "mean, vector", "m", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 292", "Euclidean distance (meters) between the TB transmitter and this receiver."],
        ["periodic", "vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 300", "Flag from SCI indicating traffic type: 1 = periodic (SPS), 0 = aperiodic (event-triggered)."],
        ["senderID", "vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 1513", "MAC node ID of the transmitting UE, extracted from the SCI metadata."],
        ["subchannelReceived", "mean, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 273", "Starting subchannel index of the received packet's resource allocation."],
        ["subchannelsUsed", "mean, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 274", "Number of subchannels occupied by the received packet."],
        ["subchannelSent", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 630", "Starting subchannel index used when transmitting."],
        ["subchannelsUsedToSend", "sum, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 631", "Number of subchannels allocated for the transmitted packet."],
        ["interPacketDelay", "mean, vector", "s", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 1669", "Time elapsed between consecutive packet receptions from the same sender. Used to detect packet loss gaps."],
        ["awareness1sStat", "mean, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 1917", "Neighbor awareness ratio over a 1-second window: fraction of nearby vehicles from which at least one packet was received within the last 1 s."],
        ["awareness500msStat", "mean, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 1918", "Neighbor awareness ratio over a 500 ms window."],
        ["awareness200msStat", "mean, vector", "", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 1919", "Neighbor awareness ratio over a 200 ms window."],
        ["posX", "mean, vector", "m", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 1953", "X-coordinate (meters) of this node's position in the OMNeT++ playground at reception time."],
        ["posY", "mean, vector", "m", "LtePhyVUeMode4", "LtePhyVUeMode4.cc : 1954", "Y-coordinate (meters) of this node's position in the OMNeT++ playground at reception time."],
        ["averageCqiD2D", "mean", "", "LtePhyUeD2D", "LtePhyUeD2D.cc : 266", "Average Channel Quality Indicator for the D2D sidelink. CQI ranges 0-15; higher = better channel conditions."],
        ["averageCqiDl", "mean", "", "LtePhyUe", "LtePhyUe.cc : 413", "Average CQI for the downlink direction. Typically -nan in Mode 4 (no eNodeB)."],
        ["averageCqiUl", "mean", "", "LtePhyUe", "LtePhyUe.cc : 511", "Average CQI for the uplink direction. Typically -nan in Mode 4."],
    ]
    pdf.add_table(headers, phy_data, col_widths)
    pdf.ln(4)

    # -----------------------------------------------------------------
    # TABLE 4 : RLC LAYER
    # -----------------------------------------------------------------
    pdf.add_page()
    pdf.section_title("6. RLC Layer Parameters (UmRxEntity / UmRxQueue)")
    pdf.section_body(
        "RLC (Radio Link Control) parameters are recorded per direction: D2D (sidelink), DL (downlink), UL (uplink). "
        "In Mode 4 out-of-coverage operation, only D2D values are meaningful; DL and UL will report NaN.\n"
        "Two granularity levels exist: PDU-level (per RLC protocol data unit) and SDU-level (per service data unit / IP packet). "
        "Cell-level variants aggregate across all UEs."
    )

    rlc_data = [
        ["rlcPduPacketLossD2D", "mean", "", "UmRxEntity", "UmRxEntity.cc : 725", "Per-PDU packet loss on D2D: emits 0.0 (received) or 1.0 (lost) for each expected RLC PDU. Mean gives loss rate."],
        ["rlcPduPacketLossDl", "mean", "", "UmRxEntity", "UmRxEntity.cc : 714", "Per-PDU packet loss on downlink. NaN in Mode 4."],
        ["rlcPduPacketLossUl", "mean", "", "UmRxEntity", "UmRxEntity.cc : 703", "Per-PDU packet loss on uplink. NaN in Mode 4."],
        ["rlcPduDelayD2D", "mean", "", "UmRxEntity", "UmRxEntity.cc : 726", "RLC PDU delay on D2D: (NOW - creationTime) in seconds for each successfully received PDU."],
        ["rlcPduDelayDl", "mean", "", "UmRxEntity", "UmRxEntity.cc : 715", "RLC PDU delay on downlink. NaN in Mode 4."],
        ["rlcPduDelayUl", "mean", "", "UmRxEntity", "UmRxEntity.cc : 704", "RLC PDU delay on uplink. NaN in Mode 4."],
        ["rlcPduThroughputD2D", "mean", "B/s", "UmRxEntity", "UmRxEntity.cc : 727", "RLC PDU throughput on D2D: PDU size / elapsed time since last PDU."],
        ["rlcPduThroughputDl", "mean", "B/s", "UmRxEntity", "UmRxEntity.cc : 716", "RLC PDU throughput on downlink. NaN in Mode 4."],
        ["rlcPduThroughputUl", "mean", "B/s", "UmRxEntity", "UmRxEntity.cc : 705", "RLC PDU throughput on uplink. NaN in Mode 4."],
        ["rlcPacketLossD2D", "mean", "", "UmRxEntity", "UmRxEntity.cc", "Per-SDU (higher-layer packet) loss rate on D2D."],
        ["rlcPacketLossDl", "mean", "", "UmRxEntity", "UmRxEntity.cc", "Per-SDU loss rate on downlink."],
        ["rlcPacketLossUl", "mean", "", "UmRxEntity", "UmRxEntity.cc", "Per-SDU loss rate on uplink."],
        ["rlcPacketLossTotal", "mean", "", "UmRxEntity", "UmRxEntity.cc", "Overall SDU loss rate across all directions."],
        ["rlcDelayD2D", "mean", "s", "UmRxEntity", "UmRxEntity.cc : 771", "SDU-level delay on D2D: (NOW - original timestamp) in seconds."],
        ["rlcDelayDl", "mean", "s", "UmRxEntity", "UmRxEntity.cc : 757", "SDU-level delay on downlink."],
        ["rlcDelayUl", "mean", "s", "UmRxEntity", "UmRxEntity.cc : 766", "SDU-level delay on uplink."],
        ["rlcThroughputD2D", "mean", "B/s", "UmRxEntity", "UmRxEntity.cc", "SDU-level throughput on D2D."],
        ["rlcThroughputDl", "mean", "B/s", "UmRxEntity", "UmRxEntity.cc", "SDU-level throughput on downlink."],
        ["rlcThroughputUl", "mean", "B/s", "UmRxEntity", "UmRxEntity.cc", "SDU-level throughput on uplink."],
        ["rlcCellPacketLossD2D", "mean", "", "UmRxEntity", "UmRxEntity.cc", "Cell-wide aggregate SDU loss on D2D."],
        ["rlcCellPacketLossDl", "mean", "", "UmRxEntity", "UmRxEntity.cc", "Cell-wide aggregate SDU loss on downlink."],
        ["rlcCellPacketLossUl", "mean", "", "UmRxEntity", "UmRxEntity.cc", "Cell-wide aggregate SDU loss on uplink."],
        ["rlcCellThroughputD2D", "mean", "B/s", "UmRxEntity", "UmRxEntity.cc", "Cell-wide aggregate throughput on D2D."],
        ["rlcCellThroughputDl", "mean", "B/s", "UmRxEntity", "UmRxEntity.cc", "Cell-wide aggregate throughput on downlink."],
        ["rlcCellThroughputUl", "mean", "B/s", "UmRxEntity", "UmRxEntity.cc", "Cell-wide aggregate throughput on uplink."],
    ]
    pdf.add_table(headers, rlc_data, col_widths)
    pdf.ln(4)

    # -----------------------------------------------------------------
    # TABLE 5 : HARQ
    # -----------------------------------------------------------------
    pdf.add_page()
    pdf.section_title("7. HARQ Parameters (LteMacBase)")
    pdf.section_body(
        "HARQ (Hybrid Automatic Repeat Request) error-rate statistics. Recorded per retransmission attempt (1st through 4th) "
        "and per direction. In the current Mode 4 config maxHarqRtx=0, so only 1st-attempt and 2nd-attempt (harqErrorRate_2nd_Dl) "
        "appear with non-NaN values. Most are disabled in omnetpp.ini."
    )
    harq_data = [
        ["harqErrorRate_2nd_Dl", "mean", "", "LteMacBase", "LteMacBase.cc", "HARQ block error rate on the 2nd transmission attempt (downlink). Fraction of HARQ processes that still fail after first retransmission."],
    ]
    pdf.add_table(headers, harq_data, col_widths)
    pdf.ln(4)

    # -----------------------------------------------------------------
    # TABLE 6 : VEINS MOBILITY
    # -----------------------------------------------------------------
    pdf.section_title("8. Veins Mobility Parameters (TraCIMobility)")
    pdf.section_body(
        "These scalars are produced by the Veins TraCIMobility module which interfaces with SUMO. "
        "They are recorded per-vehicle as scalars (no vector variant)."
    )
    mob_data = [
        ["startTime", "scalar", "s", "TraCIMobility", "TraCIMobility.cc", "Simulation time when the vehicle entered the network (SUMO departure time)."],
        ["stopTime", "scalar", "s", "TraCIMobility", "TraCIMobility.cc", "Simulation time when the vehicle left the network."],
        ["totalTime", "scalar", "s", "TraCIMobility", "TraCIMobility.cc", "Total time the vehicle was active: stopTime - startTime."],
        ["minSpeed", "scalar", "m/s", "TraCIMobility", "TraCIMobility.cc", "Minimum instantaneous speed observed during the vehicle's trip."],
        ["maxSpeed", "scalar", "m/s", "TraCIMobility", "TraCIMobility.cc", "Maximum instantaneous speed observed during the vehicle's trip."],
        ["totalDistance", "scalar", "m", "TraCIMobility", "TraCIMobility.cc", "Total distance traveled by the vehicle (odometer)."],
        ["totalCO2Emission", "scalar", "g", "TraCIMobility", "TraCIMobility.cc", "Cumulative CO2 emissions reported by SUMO's emission model."],
        ["posx", "vector", "m", "TraCIMobility", "TraCIMobility.cc", "X-coordinate of vehicle position over time (from SUMO via TraCI)."],
        ["posy", "vector", "m", "TraCIMobility", "TraCIMobility.cc", "Y-coordinate of vehicle position over time."],
        ["speed", "vector", "m/s", "TraCIMobility", "TraCIMobility.cc", "Instantaneous vehicle speed over time."],
        ["acceleration", "vector", "m/s2", "TraCIMobility", "TraCIMobility.cc", "Instantaneous vehicle acceleration over time."],
        ["co2emission", "vector", "g/s", "TraCIMobility", "TraCIMobility.cc", "Instantaneous CO2 emission rate over time."],
        ["roiArea", "scalar", "", "TraCIMobility", "TraCIMobility.cc", "Flag indicating whether the vehicle was within the Region of Interest."],
    ]
    pdf.add_table(headers, mob_data, col_widths)
    pdf.ln(4)

    # -----------------------------------------------------------------
    # NOTES
    # -----------------------------------------------------------------
    pdf.add_page()
    pdf.section_title("9. Notes on Disabled Statistics")
    pdf.section_body(
        "The following statistics are defined in .ned files but explicitly disabled in omnetpp.ini "
        "(statistic-recording = false) to reduce result file size and simulation overhead:\n\n"
        "  MAC layer (all directions): macDelay, macThroughput, macCellThroughput, macCellPacketLoss,\n"
        "    macPacketLoss, macBufferOverFlow, harqErrorRate (1st/2nd/3rd/4th attempts UL/DL/D2D),\n"
        "    receivedPacketFromUpperLayer, receivedPacketFromLowerLayer,\n"
        "    sentPacketToUpperLayer, sentPacketToLowerLayer, measuredItbs, pdcpdrop0-3.\n\n"
        "These can be re-enabled by setting the corresponding statistic-recording to true in omnetpp.ini."
    )

    pdf.section_title("10. Key Formulas")
    pdf.section_body(
        "Packet Delivery Ratio (PDR):\n"
        "  PDR = tbDecoded / tbSent  (PHY-layer, per link)\n"
        "  PDR = received / (sum of all sentMsg by neighbors)  (application-layer)\n"
        "  ICA PDR = icaReceived / icaExpected  (ICA-specific)\n\n"
        "Channel Busy Ratio (CBR):\n"
        "  CBR = (number of subchannels with RSSI > threshold) / (total subchannels in 100ms window)\n\n"
        "Verification Success Rate:\n"
        "  VSR = verified / received\n\n"
        "SCI Decode Rate:\n"
        "  SDR = sciDecoded / sciReceived\n\n"
        "TB Decode Rate:\n"
        "  TDR = tbDecoded / tbReceived\n\n"
        "RLC Packet Loss Rate:\n"
        "  PLR = mean(rlcPduPacketLossD2D)  (each sample is 0.0 or 1.0)"
    )

    pdf.section_title("11. Simulation Configuration Summary")
    pdf.section_body(
        "Network: Highway (Veins + SUMO intersection scenario)\n"
        "Crypto: PQC digital signatures (default Falcon-512)\n"
        "Subchannels: 10 subchannels x 10 RBs each\n"
        "RRI: 100 ms (pStep=100)\n"
        "Tx Power: 23 dBm\n"
        "RSSI Threshold: 9\n"
        "Max HARQ Retx: 0\n"
        "Prob Resource Keep: 0.8\n"
        "CBR-based adaptation: enabled\n"
        "CR Limit: enabled\n"
        "Packet dropping: enabled\n"
        "Packet size: 2300 bytes\n"
        "Send interval: 100 ms\n"
        "Carrier frequency: 5.915 GHz"
    )

    out_path = "/home/veins/src/simulte/analysis_notes/result_parameters_reference.pdf"
    pdf.output(out_path)
    print(f"PDF saved to: {out_path}")


if __name__ == "__main__":
    main()
