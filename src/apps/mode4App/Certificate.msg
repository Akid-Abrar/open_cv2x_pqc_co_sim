message Certificate
{
    string  subjectId;
    string  algoName;
    uint8_t publicKey[]; // Use a dynamic array for raw binary data
    int64_t notBefore = 0;
    int64_t notAfter  = 0;
}