packet IcaWarn extends cPacket
{
    int32     msgCnt = 0;        // use as monotonically increasing sequence number
    string    tempId;            // hex string for TemporaryID (4 bytes -> 8 hex chars)
    int32     intersectionId = 0;
    int32     approach     = -1; // exclusive with lane; -1 if not used
    int32     lane         = -1; // exclusive with approach; -1 if not used
    int32     eventFlag    = 0;  // VehicleEventFlags bit mask

    // RSU transmitter position (OMNeT++ world coords)
    double    srcX = 0;
    double    srcY = 0;

    // Optional/minimal Part I (kept as-is)
    int64     lat = 0;
    int64     lon = 0;

    // Timing (when RSU generated/bridged it)
    simtime_t genTime;
}
