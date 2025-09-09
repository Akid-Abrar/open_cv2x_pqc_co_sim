#ifndef PQCDSA_H
#define PQCDSA_H

#include <string>
#include <vector>

namespace pqcdsa {

struct KeyPair {
    std::string pubHex;   // hex‑encoded public key
    std::string privHex;  // hex‑encoded secret key
    size_t pubKeyLength;
    size_t privKeyLength;
};

/*  Generate a Falcon‑512 key pair */
KeyPair generateKeyPair();

/*  Sign an arbitrary byte‑sequence (provided as hex) */
std::string sign(const std::string& dataHex,
                 const std::string& privHex);

/*  Verify a Falcon signature; returns true if valid */
bool verify(const std::string& dataHex,
            const std::string& sigHex,
            const std::string& pubHex);

/*  Utility helpers (public in case you need them elsewhere) */
std::string toHex(const uint8_t* buf, size_t len);
std::vector<uint8_t> fromHex(const std::string& hex);

} // namespace pqcdsa
#endif
