import BSM;
import Certificate;

packet SPDU extends cPacket
{
    BSM          bsm;              // nested message
    uint8_t      signature[];   // raw bytes (666 for Falcon-512)
    Certificate  cert;             // sender’s certificate
}