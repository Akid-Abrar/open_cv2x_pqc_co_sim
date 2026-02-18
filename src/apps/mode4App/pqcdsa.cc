// pqcdsa.cc
#include "pqcdsa.h"

#include <stdexcept>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <memory>

// OpenSSL (ECDSA P-256)
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/x509.h>

// liboqs (PQC)
#include <oqs/oqs.h>

namespace {

// ---- Algorithm enum & lookup ----

enum class Alg { ECDSA_P256, FALCON_512, DILITHIUM_2 };

static std::string toLower(std::string s) {
    for (auto& ch : s) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return s;
}

static Alg algFromName(const std::string& nameIn) {
    const std::string s = toLower(nameIn);
    if (s == "ecdsa" || s == "ecdsa-p256" || s == "p256" || s.find("ecdsa") != std::string::npos)
        return Alg::ECDSA_P256;
    if (s == "falcon-512" || s == "falcon" || s.find("falcon") != std::string::npos)
        return Alg::FALCON_512;
    return Alg::DILITHIUM_2;
}

static Alg getDefaultAlg() {
    const char* env = std::getenv("PQCDSA_ALGO");
    return env ? algFromName(env) : Alg::FALCON_512; //Change Here
}

static const char* algTag(Alg a) {
    switch (a) {
        case Alg::ECDSA_P256:   return "ecdsa";
        case Alg::FALCON_512:   return "falcon-512";
        case Alg::DILITHIUM_2:  return "dilithium-2";
    }
    return "dilithium-2";
}

static const char* oqsAlgId(Alg a) {
    return (a == Alg::FALCON_512) ? OQS_SIG_alg_falcon_512
                                  : OQS_SIG_alg_dilithium_2;
}

// ---- Hex helpers ----

static std::string bytesToHex(const uint8_t* buf, size_t len) {
    static const char hex[] = "0123456789abcdef";
    std::string out(len * 2, '\0');
    for (size_t i = 0; i < len; ++i) {
        out[2*i]   = hex[buf[i] >> 4];
        out[2*i+1] = hex[buf[i] & 0x0f];
    }
    return out;
}

static uint8_t hexNibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    throw std::runtime_error("invalid hex character");
}

static std::vector<uint8_t> hexToBytes(const std::string& hex) {
    if (hex.size() % 2) throw std::runtime_error("odd-length hex string");
    std::vector<uint8_t> out(hex.size() / 2);
    for (size_t i = 0; i < out.size(); ++i)
        out[i] = (hexNibble(hex[2*i]) << 4) | hexNibble(hex[2*i+1]);
    return out;
}

// ---- Prefix helpers ----

static std::string stripPrefix(const std::string& s) {
    if (s.size() >= 4 && s.compare(0, 4, "ALG:") == 0) {
        auto pos = s.find_last_of(':');
        if (pos == std::string::npos || pos + 1 >= s.size())
            throw std::runtime_error("invalid prefixed key");
        return s.substr(pos + 1);
    }
    if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        return s.substr(2);
    return s;
}

static Alg algFromPrefixed(const std::string& s) {
    if (s.size() >= 4 && s.compare(0, 4, "ALG:") == 0) {
        auto second = s.find(':', 4);
        if (second == std::string::npos) throw std::runtime_error("invalid key prefix");
        return algFromName(s.substr(4, second - 4));
    }
    return getDefaultAlg();
}

static std::vector<uint8_t> decodeHex(const std::string& maybePrefixed) {
    return hexToBytes(stripPrefix(maybePrefixed));
}

// ---- OpenSSL RAII ----

struct EvpPkeyDel { void operator()(EVP_PKEY* p)     const { EVP_PKEY_free(p); } };
struct PkeyCtxDel { void operator()(EVP_PKEY_CTX* c)  const { EVP_PKEY_CTX_free(c); } };
struct MdCtxDel   { void operator()(EVP_MD_CTX* c)    const { EVP_MD_CTX_free(c); } };

// ---- ECDSA helpers ----

static void genECDSAp256(std::vector<uint8_t>& pubDer, std::vector<uint8_t>& privDer) {
    std::unique_ptr<EVP_PKEY_CTX, PkeyCtxDel> ctx(EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr));
    if (!ctx || EVP_PKEY_keygen_init(ctx.get()) <= 0)
        throw std::runtime_error("ECDSA keygen init failed");
    if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx.get(), NID_X9_62_prime256v1) <= 0)
        throw std::runtime_error("ECDSA param set failed");

    EVP_PKEY* raw = nullptr;
    if (EVP_PKEY_keygen(ctx.get(), &raw) <= 0)
        throw std::runtime_error("ECDSA keygen failed");
    std::unique_ptr<EVP_PKEY, EvpPkeyDel> pkey(raw);

    int lenPub = i2d_PUBKEY(pkey.get(), nullptr);
    if (lenPub <= 0) throw std::runtime_error("ECDSA pub DER failed");
    pubDer.resize(lenPub);
    unsigned char* pp = pubDer.data();
    i2d_PUBKEY(pkey.get(), &pp);

    int lenPriv = i2d_PrivateKey(pkey.get(), nullptr);
    if (lenPriv <= 0) throw std::runtime_error("ECDSA priv DER failed");
    privDer.resize(lenPriv);
    pp = privDer.data();
    i2d_PrivateKey(pkey.get(), &pp);
}

static std::string ecdsaSign(const std::vector<uint8_t>& msg, const std::vector<uint8_t>& privDer) {
    const unsigned char* p = privDer.data();
    std::unique_ptr<EVP_PKEY, EvpPkeyDel> pkey(d2i_AutoPrivateKey(nullptr, &p, (long)privDer.size()));
    if (!pkey) throw std::runtime_error("ECDSA load priv failed");

    std::unique_ptr<EVP_MD_CTX, MdCtxDel> md(EVP_MD_CTX_new());
    if (EVP_DigestSignInit(md.get(), nullptr, EVP_sha256(), nullptr, pkey.get()) <= 0)
        throw std::runtime_error("ECDSA DigestSignInit failed");
    if (EVP_DigestSignUpdate(md.get(), msg.data(), msg.size()) <= 0)
        throw std::runtime_error("ECDSA DigestSignUpdate failed");

    size_t derLen = 0;
    EVP_DigestSignFinal(md.get(), nullptr, &derLen);
    std::vector<uint8_t> derSig(derLen);
    if (EVP_DigestSignFinal(md.get(), derSig.data(), &derLen) <= 0)
        throw std::runtime_error("ECDSA sign failed");
    derSig.resize(derLen);

    // Convert DER signature to fixed-size raw (r||s) = 64 bytes for P-256
    const unsigned char* dp = derSig.data();
    ECDSA_SIG* esig = d2i_ECDSA_SIG(nullptr, &dp, (long)derSig.size());
    if (!esig) throw std::runtime_error("ECDSA DER decode failed");

    const BIGNUM* r = nullptr;
    const BIGNUM* s = nullptr;
    ECDSA_SIG_get0(esig, &r, &s);

    std::vector<uint8_t> raw(64, 0);
    BN_bn2binpad(r, raw.data(),      32);
    BN_bn2binpad(s, raw.data() + 32, 32);
    ECDSA_SIG_free(esig);

    return bytesToHex(raw.data(), raw.size());
}

static bool ecdsaVerify(const std::vector<uint8_t>& msg,
                        const std::vector<uint8_t>& rawSig,
                        const std::vector<uint8_t>& pubDer) {
    if (rawSig.size() != 64) return false;

    // Convert fixed-size raw (r||s) back to DER for OpenSSL
    ECDSA_SIG* esig = ECDSA_SIG_new();
    if (!esig) return false;

    BIGNUM* r = BN_bin2bn(rawSig.data(),      32, nullptr);
    BIGNUM* s = BN_bin2bn(rawSig.data() + 32, 32, nullptr);
    if (!r || !s || !ECDSA_SIG_set0(esig, r, s)) {
        // set0 takes ownership on success; free manually on failure
        if (r) BN_free(r);
        if (s) BN_free(s);
        ECDSA_SIG_free(esig);
        return false;
    }

    int derLen = i2d_ECDSA_SIG(esig, nullptr);
    if (derLen <= 0) { ECDSA_SIG_free(esig); return false; }
    std::vector<uint8_t> derSig(derLen);
    unsigned char* dp = derSig.data();
    i2d_ECDSA_SIG(esig, &dp);
    ECDSA_SIG_free(esig);

    const unsigned char* pp = pubDer.data();
    std::unique_ptr<EVP_PKEY, EvpPkeyDel> pkey(d2i_PUBKEY(nullptr, &pp, (long)pubDer.size()));
    if (!pkey) return false;

    std::unique_ptr<EVP_MD_CTX, MdCtxDel> md(EVP_MD_CTX_new());
    if (EVP_DigestVerifyInit(md.get(), nullptr, EVP_sha256(), nullptr, pkey.get()) <= 0)
        return false;
    if (EVP_DigestVerifyUpdate(md.get(), msg.data(), msg.size()) <= 0)
        return false;
    return EVP_DigestVerifyFinal(md.get(), derSig.data(), derSig.size()) == 1;
}

// ---- liboqs helpers ----

static std::string oqsSign(const std::vector<uint8_t>& msg, Alg alg,
                            const std::vector<uint8_t>& sk) {
    OQS_SIG* sig = OQS_SIG_new(oqsAlgId(alg));
    if (!sig) throw std::runtime_error("OQS alg unavailable");

    std::vector<uint8_t> signature(sig->length_signature);
    size_t sigLen = 0;
    if (OQS_SIG_sign(sig, signature.data(), &sigLen, msg.data(), msg.size(), sk.data()) != OQS_SUCCESS) {
        OQS_SIG_free(sig);
        throw std::runtime_error("OQS sign failed");
    }
    OQS_SIG_free(sig);
    return bytesToHex(signature.data(), sigLen);
}

static bool oqsVerify(const std::vector<uint8_t>& msg, Alg alg,
                       const std::vector<uint8_t>& sigBytes,
                       const std::vector<uint8_t>& pk) {
    OQS_SIG* sig = OQS_SIG_new(oqsAlgId(alg));
    if (!sig) return false;
    bool ok = OQS_SIG_verify(sig, msg.data(), msg.size(),
                              sigBytes.data(), sigBytes.size(), pk.data()) == OQS_SUCCESS;
    OQS_SIG_free(sig);
    return ok;
}

} // unnamed namespace


// ---- Public API ----
namespace pqcdsa {

std::string toHex(const uint8_t* buf, size_t len) {
    return bytesToHex(buf, len);
}

std::vector<uint8_t> fromHex(const std::string& maybePrefixedHex) {
    return decodeHex(maybePrefixedHex);
}

KeyPair generateKeyPair() {
    KeyPair kp;
    Alg alg = getDefaultAlg();
    const char* tag = algTag(alg);

    if (alg == Alg::ECDSA_P256) {
        std::vector<uint8_t> pubDer, privDer;
        genECDSAp256(pubDer, privDer);
        kp.pubHex       = std::string("ALG:") + tag + ":" + bytesToHex(pubDer.data(), pubDer.size());
        kp.privHex      = std::string("ALG:") + tag + ":" + bytesToHex(privDer.data(), privDer.size());
        kp.pubKeyLength  = pubDer.size();
        kp.privKeyLength = privDer.size();
        return kp;
    }

    OQS_SIG* sig = OQS_SIG_new(oqsAlgId(alg));
    if (!sig) throw std::runtime_error("OQS alg unavailable");

    std::vector<uint8_t> pk(sig->length_public_key);
    std::vector<uint8_t> sk(sig->length_secret_key);
    if (OQS_SIG_keypair(sig, pk.data(), sk.data()) != OQS_SUCCESS) {
        OQS_SIG_free(sig);
        throw std::runtime_error("OQS keypair gen failed");
    }
    kp.pubHex       = std::string("ALG:") + tag + ":" + bytesToHex(pk.data(), pk.size());
    kp.privHex      = std::string("ALG:") + tag + ":" + bytesToHex(sk.data(), sk.size());
    kp.pubKeyLength  = pk.size();
    kp.privKeyLength = sk.size();
    OQS_SIG_free(sig);
    return kp;
}

std::string sign(const std::string& dataHex, const std::string& privHex) {
    Alg alg = algFromPrefixed(privHex);
    std::vector<uint8_t> msg = decodeHex(dataHex);
    std::vector<uint8_t> sk  = decodeHex(privHex);

    if (alg == Alg::ECDSA_P256)
        return ecdsaSign(msg, sk);
    return oqsSign(msg, alg, sk);
}

bool verify(const std::string& dataHex, const std::string& sigHex, const std::string& pubHex) {
    Alg alg = algFromPrefixed(pubHex);
    std::vector<uint8_t> msg = decodeHex(dataHex);
    std::vector<uint8_t> sig = decodeHex(sigHex);
    std::vector<uint8_t> pk  = decodeHex(pubHex);

    if (alg == Alg::ECDSA_P256)
        return ecdsaVerify(msg, sig, pk);
    return oqsVerify(msg, alg, sig, pk);
}

std::string algoTagFromKey(const std::string& prefixedHex) {
    try {
        return algTag(algFromPrefixed(prefixedHex));
    } catch (...) {
        return algTag(getDefaultAlg());
    }
}

std::string prettyNameFromTag(const std::string& tag) {
    Alg a = algFromName(tag);
    switch (a) {
        case Alg::ECDSA_P256:   return "ECDSA P-256";
        case Alg::FALCON_512:   return "Falcon-512";
        case Alg::DILITHIUM_2:  return "Dilithium 2";
    }
    return "Dilithium 2";
}

std::string prefixKeyWithCertAlgo(const std::string& rawHex, const std::string& certAlgoName) {
    return std::string("ALG:") + algTag(algFromName(certAlgoName)) + ":" + rawHex;
}

} // namespace pqcdsa
