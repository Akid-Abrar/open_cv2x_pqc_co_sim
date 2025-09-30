//#ifndef PQCDSA_H
//#define PQCDSA_H
//
//#include <string>
//#include <vector>
//
//namespace pqcdsa {
//
//struct KeyPair {
//    std::string pubHex;   // hex‑encoded public key
//    std::string privHex;  // hex‑encoded secret key
//    size_t pubKeyLength;
//    size_t privKeyLength;
//};
//
///*  Generate a Falcon‑512 key pair */
//KeyPair generateKeyPair();
//
///*  Sign an arbitrary byte‑sequence (provided as hex) */
//std::string sign(const std::string& dataHex,
//                 const std::string& privHex);
//
///*  Verify a Falcon signature; returns true if valid */
//bool verify(const std::string& dataHex,
//            const std::string& sigHex,
//            const std::string& pubHex);
//
///*  Utility helpers (public in case you need them elsewhere) */
//std::string toHex(const uint8_t* buf, size_t len);
//std::vector<uint8_t> fromHex(const std::string& hex);
//
//} // namespace pqcdsa
//#endif

#ifndef PQCDSA_H
#define PQCDSA_H

#include <string>
#include <vector>

namespace pqcdsa {

struct KeyPair {
    std::string pubHex;   // may be prefixed: "ALG:<name>:<hex>"
    std::string privHex;  // may be prefixed: "ALG:<name>:<hex>"
    size_t pubKeyLength = 0;
    size_t privKeyLength = 0;
};

/* Generate a key pair (algorithm is chosen by env var PQCDSA_ALGO:
   "ecdsa", "falcon-512", "dilithium-2"; default "dilithium-2") */
KeyPair generateKeyPair();

/* Sign arbitrary data (dataHex = hex of message bytes). The algorithm
   is inferred from the prefix inside privHex. */
std::string sign(const std::string& dataHex, const std::string& privHex);

/* Verify signature (sigHex) over dataHex. Algorithm inferred from pubHex. */
bool verify(const std::string& dataHex, const std::string& sigHex, const std::string& pubHex);

/* Hex helpers (now robust to prefixed strings in fromHex) */
std::string toHex(const uint8_t* buf, size_t len);
std::vector<uint8_t> fromHex(const std::string& maybePrefixedHex);

} // namespace pqcdsa
#endif


