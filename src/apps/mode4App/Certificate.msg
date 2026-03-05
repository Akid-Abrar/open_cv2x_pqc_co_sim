message Certificate
{
    // --- CertificateBase ---
    uint8_t   version = 3;
    uint8_t   certType = 0;           // 0=explicit
    uint8_t   issuerType = 0;         // 0=sha256AndDigest, 1=self
    uint8_t   issuerDigest[8];        // HashedId8 of issuing CA

    // --- ToBeSignedCertificate ---
    string    subjectId = "";          // CertificateId (kept for debug)
    uint8_t   cracaId[3];             // HashedId3
    uint16_t  crlSeries = 0;
    int64_t   validityStart = 0;       // Time32 (seconds since epoch)
    int64_t   validityDuration = 0;    // Duration (seconds)
    uint32_t  appPermPsid = 0x20;      // appPermissions PSID

    // --- VerifyKeyIndicator ---
    string    algoName = "";
    uint8_t   publicKey[];
}
