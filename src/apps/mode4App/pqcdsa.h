#ifndef PQCDSA_H
#define PQCDSA_H

#include <string>
#include <vector>

namespace pqcdsa {

struct KeyPair {
    std::string pubHex;   // prefixed: "ALG:<name>:<hex>"
    std::string privHex;  // prefixed: "ALG:<name>:<hex>"
    size_t pubKeyLength = 0;
    size_t privKeyLength = 0;
};

KeyPair generateKeyPair();

std::string sign(const std::string& dataHex, const std::string& privHex);

bool verify(const std::string& dataHex, const std::string& sigHex, const std::string& pubHex);

std::string toHex(const uint8_t* buf, size_t len);

std::vector<uint8_t> fromHex(const std::string& maybePrefixedHex);

std::string algoTagFromKey(const std::string& prefixedHex);

std::string prettyNameFromTag(const std::string& tag);

std::string prefixKeyWithCertAlgo(const std::string& rawHex, const std::string& certAlgoName);

} // namespace pqcdsa
#endif
