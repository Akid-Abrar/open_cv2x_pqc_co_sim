// IcaSpdu.msg
import IcaWarn;
import Certificate;

packet IcaSpdu extends cPacket
{
    IcaWarn   warn;        // the ICA payload
    uint8_t   signature[]; // PQC signature bytes
    Certificate cert;      // RSU certificate
}
