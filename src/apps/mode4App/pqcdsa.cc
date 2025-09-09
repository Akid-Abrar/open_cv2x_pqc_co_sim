#include "pqcdsa.h"
#include <oqs/oqs.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace {

/* ----------  small hex helpers  ---------- */
std::string bytesToHex(const uint8_t* buf, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(buf[i]);
    return oss.str();
}

std::vector<uint8_t> hexToBytes(const std::string& hex) {
    if (hex.size() % 2) throw std::runtime_error("hex length odd");
    std::vector<uint8_t> out(hex.size() / 2);
    for (size_t i = 0; i < out.size(); ++i)
        out[i] = static_cast<uint8_t>(
            std::stoi(hex.substr(2 * i, 2), nullptr, 16));
    return out;
}

} // unnamed namespace

/* ----------  public wrappers  ---------- */
namespace pqcdsa {

const char* OQS_ALGORITHM_NAME = OQS_SIG_alg_dilithium_2;

std::string toHex(const uint8_t* buf, size_t len) { return bytesToHex(buf, len); }
std::vector<uint8_t> fromHex(const std::string& h) { return hexToBytes(h); }

KeyPair generateKeyPair() {
    OQS_SIG* sig = OQS_SIG_new(OQS_ALGORITHM_NAME);
//    OQS_SIG* sig = OQS_SIG_new(OQS_SIG_alg_sphincs_shake_128f_simple);
    if (!sig) throw std::runtime_error("Falcon‑512 unavailable");

    std::vector<uint8_t> pk(sig->length_public_key);
    std::vector<uint8_t> sk(sig->length_secret_key);

    if (OQS_SIG_keypair(sig, pk.data(), sk.data()) != OQS_SUCCESS)
        throw std::runtime_error("Falcon keypair gen failed");

//    KeyPair kp{ bytesToHex(pk.data(), pk.size()),
//                bytesToHex(sk.data(), sk.size()),  };
    KeyPair kp;
    kp.pubHex = bytesToHex(pk.data(), pk.size());
    kp.privHex = bytesToHex(sk.data(), sk.size());
    kp.pubKeyLength = sig->length_public_key;   // Store the length
    kp.privKeyLength = sig->length_secret_key; // Store the length
    OQS_SIG_free(sig);
    return kp;
}

std::string sign(const std::string& dataHex,
                 const std::string& privHex) {

    OQS_SIG* sig = OQS_SIG_new(OQS_ALGORITHM_NAME);
//    OQS_SIG* sig = OQS_SIG_new(OQS_SIG_alg_sphincs_shake_128f_simple);
    if (!sig) throw std::runtime_error("Falcon‑512 unavailable");

    auto msg     = hexToBytes(dataHex);
    auto skBytes = hexToBytes(privHex);

    std::vector<uint8_t> signature(sig->length_signature);
    size_t sigLen = 0;

    if (OQS_SIG_sign(sig,
                     signature.data(), &sigLen,
                     msg.data(), msg.size(),
                     skBytes.data()) != OQS_SUCCESS)
        throw std::runtime_error("Falcon sign failed");

    OQS_SIG_free(sig);
    return bytesToHex(signature.data(), sigLen);
}

bool verify(const std::string& dataHex,
            const std::string& sigHex,
            const std::string& pubHex) {

    OQS_SIG* sig = OQS_SIG_new(OQS_ALGORITHM_NAME);
//    OQS_SIG* sig = OQS_SIG_new(OQS_SIG_alg_sphincs_shake_128f_simple);
    if (!sig) throw std::runtime_error("Falcon‑512 unavailable");

    auto msg  = hexToBytes(dataHex);
    auto sigB = hexToBytes(sigHex);
    auto pkB  = hexToBytes(pubHex);

    bool ok = (OQS_SIG_verify(sig,
                              msg.data(), msg.size(),
                              sigB.data(), sigB.size(),
                              pkB.data()) == OQS_SUCCESS);

    OQS_SIG_free(sig);
    return ok;
}

} // namespace pqcdsa
