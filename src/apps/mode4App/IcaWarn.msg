packet IcaWarn extends cPacket
{
int32     msgCnt = 0;
    string    tempId;            // hex string for TemporaryID (4 bytes -> 8 hex chars)
    int32     intersectionId = 0;
    int32     approach     = -1; // exclusive with lane; -1 if not used
    int32     lane         = -1; // exclusive with approach; -1 if not used
    int32     eventFlag    = 0;  // VehicleEventFlags bit mask

    // Optional/minimal Part I (if you want quick visualization)
    int64     lat = 0;           // raw integer like J2735 (e.g., 1e-7 deg scaling upstream)
    int64     lon = 0;

    // Timing (who cares about exact UPER here; we just keep a timestamp)
    simtime_t genTime;           // when RSU generated/bridged it
}

