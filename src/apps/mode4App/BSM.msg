message BSM
{
    // --- J2735 BSMcoreData (Part I) — fully compliant ---
    uint8_t   msgCnt = 0;              // MsgCount (0-127)                         1B
    int32_t   msgId;                   // simulation-only PDR tracking             4B (not in J2735)
    string    tempId = "";             // TemporaryID (4-octet hex)                4B wire
    uint16_t  secMark = 0;            // DSecond (ms within minute, 0-65535)      2B
    int32_t   lat = 0;                // Latitude  (OMNeT coord in mm: m * 1000)  4B
    int32_t   lon = 0;                // Longitude (OMNeT coord in mm: m * 1000)  4B
    int16_t   elev = 0;               // Elevation (0.1m, -4096..61439)           2B

    // PositionalAccuracy (J2735: 4 bytes total)
    uint8_t   semiMajor = 255;        // 0.05m units (255=unavailable)            1B
    uint8_t   semiMinor = 255;        // 0.05m units (255=unavailable)            1B
    uint16_t  semiMajorOrient = 65535; // 360/65535 deg (65535=unavailable)        2B

    uint8_t   transmission = 7;       // TransmissionState (7=unavailable)        1B
    uint16_t  speed_j = 8191;         // Speed (0.02 m/s, 8191=unavailable)       2B
    uint16_t  heading_j = 28800;      // Heading (0.0125 deg, 28800=unavailable)  2B
    int8_t    angle = 127;            // SteeringWheelAngle (127=unavailable)     1B

    // AccelerationSet4Way (J2735: 7 bytes total)
    int16_t   accelLong = 2001;       // 0.01 m/s^2 (-2000..2001, 2001=unavail)  2B
    int16_t   accelLat = 2001;        // 0.01 m/s^2 (-2000..2001, 2001=unavail)  2B
    int8_t    accelVert = -127;       // 0.02 G (-127..127, -127=unavailable)     1B
    int16_t   yawRate = 0;            // 0.01 deg/s (-32767..32767)               2B

    // BrakeSystemStatus (J2735: 2 bytes total)
    uint16_t  brakes = 0;            // packed bitfield (all unavailable)         2B

    // VehicleSize (J2735: 3 bytes total)
    uint16_t  vehWidth = 0;          // 0.01m (0-1023, 0=unavailable)            2B
    uint8_t   vehLength = 0;         // 0.1m (0-255, mapped from 0-4095)         1B
}
