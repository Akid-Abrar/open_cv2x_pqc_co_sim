import BSM;
import Certificate;

packet SPDU extends cPacket
{
    // --- Ieee1609Dot2Data ---
    uint8_t   protocolVersion = 3;

    // --- HeaderInfo ---
    uint32_t  psid = 0x20;             // BSM PSID
    int64_t   generationTime = 0;      // microsec since 2004-01-01 UTC
    int32_t   genLocation_lat = 0;     // optional
    int32_t   genLocation_lon = 0;
    int16_t   genLocation_elev = 0;

    // --- ToBeSignedData.payload ---
    BSM       bsm;

    // --- SignerIdentifier ---
    uint8_t   signerType = 1;          // 0=digest, 1=certificate, 2=self
    uint8_t   signerDigest[8];         // HashedId8 (when signerType=0)
    Certificate cert;                   // included when signerType=1

    // --- Signature ---
    uint8_t   signature[];
}
