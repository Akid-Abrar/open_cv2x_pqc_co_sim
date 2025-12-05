// pqcdsa.cc
#include "pqcdsa.h"

#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

// OpenSSL (ECDSA P-256)
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/sha.h>
#include <openssl/obj_mac.h>
#include <openssl/x509.h>   // i2d_PUBKEY / d2i_PUBKEY

// liboqs (PQC)
#include <oqs/oqs.h>

namespace {

// ---------------- small hex helpers ----------------
static std::string bytesToHex(const uint8_t* buf, size_t len) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i)
        oss << std::setw(2) << static_cast<unsigned>(buf[i]);
    return oss.str();
}

static bool isHexChar(char c) {
    return std::isdigit(static_cast<unsigned char>(c))
        || (c>='a' && c<='f') || (c>='A' && c<='F');
}

static std::string stripAlgPrefixIfAny(const std::string& s) {
    // Accept forms:
    //   "ALG:<name>:<hex>"
    //   "0x<hex>"
    //   "<hex>"
    // Return "<hex>" portion.
    if (s.size() >= 4 && s.compare(0, 4, "ALG:") == 0) {
        auto pos = s.find_last_of(':');
        if (pos == std::string::npos || pos + 1 >= s.size())
            throw std::runtime_error("invalid prefixed key");
        return s.substr(pos + 1);
    }
    if (s.size() >= 2 && (s[0] == '0') && (s[1] == 'x' || s[1] == 'X')) {
        return s.substr(2);
    }
    return s;
}

static std::vector<uint8_t> hexToBytes_strict(const std::string& hex) {
    if (hex.empty() || (hex.size() % 2))
        throw std::runtime_error("invalid hex");
    std::vector<uint8_t> out(hex.size() / 2);
    for (size_t i = 0; i < out.size(); ++i) {
        char c1 = hex[2*i], c2 = hex[2*i+1];
        if (!isHexChar(c1) || !isHexChar(c2))
            throw std::runtime_error("invalid hex");
        out[i] = static_cast<uint8_t>(std::stoi(hex.substr(2*i, 2), nullptr, 16));
    }
    return out;
}

// ---------------- algorithm selection ----------------
enum class Alg { ECDSA_P256, FALCON_512, DILITHIUM_2 };

// You can change default here if desired
static Alg kDefaultAlgo = Alg::ECDSA_P256;

static Alg defaultAlgFromEnv() {
    const char* env = std::getenv("PQCDSA_ALGO");
    if (!env) return Alg::DILITHIUM_2; // current default used when no prefix known
    std::string s(env);
    for (auto& ch : s) ch = std::tolower(static_cast<unsigned char>(ch));
    if (s == "ecdsa" || s == "ecdsa-p256" || s == "p256") return Alg::ECDSA_P256;
    if (s == "falcon-512" || s == "falcon") return Alg::FALCON_512;
    if (s == "dilithium-2" || s == "dilithium") return Alg::DILITHIUM_2;
    return Alg::DILITHIUM_2;
}

static std::string algName(Alg a) {
    switch (a) {
        case Alg::ECDSA_P256:   return "ecdsa";
        case Alg::FALCON_512:   return "falcon-512";
        case Alg::DILITHIUM_2:  return "dilithium-2";
    }
    return "dilithium-2";
}

static Alg algFromPrefixed(const std::string& s) {
    if (s.size() >= 4 && s.compare(0,4,"ALG:")==0) {
        // form: ALG:<name>:<hex>
        auto second = s.find(':', 4);
        if (second == std::string::npos) throw std::runtime_error("invalid key prefix");
        const std::string name = s.substr(4, second-4);
        std::string low = name;
        for (auto& ch : low) ch = std::tolower(static_cast<unsigned char>(ch));
        if (low == "ecdsa" || low == "ecdsa-p256" || low == "p256") return Alg::ECDSA_P256;
        if (low == "falcon-512" || low == "falcon") return Alg::FALCON_512;
        if (low == "dilithium-2" || low == "dilithium") return Alg::DILITHIUM_2;
        throw std::runtime_error("unknown algorithm in key prefix");
    }
    // no prefix => assume current default (for backward compat you can force PQC via env)
    return defaultAlgFromEnv();
}

// --------------- OpenSSL ECDSA helpers ---------------
struct EvpPkeyDeleter { void operator()(EVP_PKEY* p) const { EVP_PKEY_free(p); } };
struct CtxDeleter     { void operator()(EVP_PKEY_CTX* c) const { EVP_PKEY_CTX_free(c); } };
struct MdCtxDel       { void operator()(EVP_MD_CTX* c) const { EVP_MD_CTX_free(c); } };

static void genECDSAp256(std::vector<uint8_t>& pubDer, std::vector<uint8_t>& privDer) {
    std::unique_ptr<EVP_PKEY_CTX, CtxDeleter> ctx(EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr));
    if (!ctx || EVP_PKEY_keygen_init(ctx.get()) <= 0)
        throw std::runtime_error("ECDSA keygen init failed");
    if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx.get(), NID_X9_62_prime256v1) <= 0)
        throw std::runtime_error("ECDSA param set failed");

    EVP_PKEY* raw = nullptr;
    if (EVP_PKEY_keygen(ctx.get(), &raw) <= 0)
        throw std::runtime_error("ECDSA keygen failed");
    std::unique_ptr<EVP_PKEY, EvpPkeyDeleter> pkey(raw);

    // public DER (SubjectPublicKeyInfo)
    int lenPub = i2d_PUBKEY(pkey.get(), nullptr);
    if (lenPub <= 0) throw std::runtime_error("ECDSA pub DER size failed");
    pubDer.resize(lenPub);
    {
        unsigned char* p = pubDer.data();
        if (i2d_PUBKEY(pkey.get(), &p) != lenPub) throw std::runtime_error("ECDSA pub DER write failed");
    }

    // private DER (PKCS#8 PrivateKeyInfo)
    int lenPriv = i2d_PrivateKey(pkey.get(), nullptr);
    if (lenPriv <= 0) throw std::runtime_error("ECDSA priv DER size failed");
    privDer.resize(lenPriv);
    {
        unsigned char* p = privDer.data();
        if (i2d_PrivateKey(pkey.get(), &p) != lenPriv) throw std::runtime_error("ECDSA priv DER write failed");
    }
}

static std::string ecdsaSign(const std::vector<uint8_t>& msg, const std::vector<uint8_t>& privDer) {
    const unsigned char* p = privDer.data();
    std::unique_ptr<EVP_PKEY, EvpPkeyDeleter> pkey(d2i_AutoPrivateKey(nullptr, &p, (long)privDer.size()));
    if (!pkey) throw std::runtime_error("ECDSA load priv failed");

    std::unique_ptr<EVP_MD_CTX, MdCtxDel> md(EVP_MD_CTX_new());
    if (!md) throw std::runtime_error("ECDSA mdctx alloc failed");

    if (EVP_DigestSignInit(md.get(), nullptr, EVP_sha256(), nullptr, pkey.get()) <= 0)
        throw std::runtime_error("ECDSA DigestSignInit failed");

    if (EVP_DigestSignUpdate(md.get(), msg.data(), msg.size()) <= 0)
        throw std::runtime_error("ECDSA DigestSignUpdate failed");

    size_t sigLen = 0;
    if (EVP_DigestSignFinal(md.get(), nullptr, &sigLen) <= 0)
        throw std::runtime_error("ECDSA sig len failed");

    std::vector<uint8_t> sig(sigLen);
    if (EVP_DigestSignFinal(md.get(), sig.data(), &sigLen) <= 0)
        throw std::runtime_error("ECDSA sign failed");
    sig.resize(sigLen);
    return bytesToHex(sig.data(), sig.size()); // DER-encoded ECDSA signature
}

static bool ecdsaVerify(const std::vector<uint8_t>& msg,
                        const std::vector<uint8_t>& sigDer,
                        const std::vector<uint8_t>& pubDer) {
    const unsigned char* p = pubDer.data();
    std::unique_ptr<EVP_PKEY, EvpPkeyDeleter> pkey(d2i_PUBKEY(nullptr, &p, (long)pubDer.size()));
    if (!pkey) return false;

    std::unique_ptr<EVP_MD_CTX, MdCtxDel> md(EVP_MD_CTX_new());
    if (!md) return false;

    if (EVP_DigestVerifyInit(md.get(), nullptr, EVP_sha256(), nullptr, pkey.get()) <= 0)
        return false;

    if (EVP_DigestVerifyUpdate(md.get(), msg.data(), msg.size()) <= 0)
        return false;

    int rc = EVP_DigestVerifyFinal(md.get(), sigDer.data(), sigDer.size());
    return rc == 1;
}

// --------------- liboqs helpers ---------------
static std::string oqsSign(const std::vector<uint8_t>& msg,
                           const std::string& oqsAlgName,
                           const std::vector<uint8_t>& sk) {
    OQS_SIG* sig = OQS_SIG_new(oqsAlgName.c_str());
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

static bool oqsVerify(const std::vector<uint8_t>& msg,
                      const std::string& oqsAlgName,
                      const std::vector<uint8_t>& sigBytes,
                      const std::vector<uint8_t>& pk) {
    OQS_SIG* sig = OQS_SIG_new(oqsAlgName.c_str());
    if (!sig) return false;
    bool ok = (OQS_SIG_verify(sig, msg.data(), msg.size(), sigBytes.data(), sigBytes.size(), pk.data()) == OQS_SUCCESS);
    OQS_SIG_free(sig);
    return ok;
}

} // unnamed namespace


// ---------------- public wrappers ----------------
namespace pqcdsa {

std::string toHex(const uint8_t* buf, size_t len) { return bytesToHex(buf, len); }

std::vector<uint8_t> fromHex(const std::string& maybePrefixedHex) {
    const std::string hex = stripAlgPrefixIfAny(maybePrefixedHex);
    return hexToBytes_strict(hex);
}

KeyPair generateKeyPair() {
    KeyPair kp;
    // Choose default algorithm; can be changed via kDefaultAlgo or env if you wish.
    Alg alg = kDefaultAlgo;

    if (alg == Alg::ECDSA_P256) {
        std::vector<uint8_t> pubDer, privDer;
        genECDSAp256(pubDer, privDer);
        kp.pubHex   = "ALG:" + ::algName(alg) + ":" + bytesToHex(pubDer.data(), pubDer.size());
        kp.privHex  = "ALG:" + ::algName(alg) + ":" + bytesToHex(privDer.data(), privDer.size());
        kp.pubKeyLength  = pubDer.size();
        kp.privKeyLength = privDer.size();
        return kp;
    }

    // PQC via liboqs
    const char* oqsName = (alg == Alg::FALCON_512) ? OQS_SIG_alg_falcon_512
                                                   : OQS_SIG_alg_dilithium_2;
    OQS_SIG* sig = OQS_SIG_new(oqsName);
    if (!sig) throw std::runtime_error("OQS alg unavailable");

    std::vector<uint8_t> pk(sig->length_public_key);
    std::vector<uint8_t> sk(sig->length_secret_key);
    if (OQS_SIG_keypair(sig, pk.data(), sk.data()) != OQS_SUCCESS) {
        OQS_SIG_free(sig);
        throw std::runtime_error("OQS keypair gen failed");
    }
    kp.pubHex   = "ALG:" + ::algName(alg) + ":" + bytesToHex(pk.data(), pk.size());
    kp.privHex  = "ALG:" + ::algName(alg) + ":" + bytesToHex(sk.data(), sk.size());
    kp.pubKeyLength  = pk.size();
    kp.privKeyLength = sk.size();
    OQS_SIG_free(sig);
    return kp;
}

std::string sign(const std::string& dataHex, const std::string& privHex) {
    // parse algorithm from priv key prefix
    Alg alg = ::algFromPrefixed(privHex);
    const std::vector<uint8_t> msg = hexToBytes_strict(stripAlgPrefixIfAny(dataHex));

    if (alg == Alg::ECDSA_P256) {
        const std::vector<uint8_t> sk = hexToBytes_strict(stripAlgPrefixIfAny(privHex));
        return ecdsaSign(msg, sk);
    }

    // PQC
    const std::vector<uint8_t> sk = hexToBytes_strict(stripAlgPrefixIfAny(privHex));
    const char* oqsName = (alg == Alg::FALCON_512) ? OQS_SIG_alg_falcon_512
                                                   : OQS_SIG_alg_dilithium_2;
    return oqsSign(msg, oqsName, sk);
}

bool verify(const std::string& dataHex, const std::string& sigHex, const std::string& pubHex) {
    Alg alg = ::algFromPrefixed(pubHex);
    const std::vector<uint8_t> msg = hexToBytes_strict(stripAlgPrefixIfAny(dataHex));

    if (alg == Alg::ECDSA_P256) {
        const std::vector<uint8_t> sig = hexToBytes_strict(stripAlgPrefixIfAny(sigHex));
        const std::vector<uint8_t> pk  = hexToBytes_strict(stripAlgPrefixIfAny(pubHex));
        return ecdsaVerify(msg, sig, pk);
    }

    // PQC
    const std::vector<uint8_t> sig = hexToBytes_strict(stripAlgPrefixIfAny(sigHex));
    const std::vector<uint8_t> pk  = hexToBytes_strict(stripAlgPrefixIfAny(pubHex));
    const char* oqsName = (alg == Alg::FALCON_512) ? OQS_SIG_alg_falcon_512
                                                   : OQS_SIG_alg_dilithium_2;
    return oqsVerify(msg, oqsName, sig, pk);
}

/* -------- helpers to bridge cert <-> prefixed keys -------- */

static std::string toLower(std::string s) {
    for (auto& ch : s) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return s;
}

std::string algoTagFromKey(const std::string& prefixedHex) {
    // If key has ALG prefix parse it; otherwise fall back to default
    try {
        Alg a = ::algFromPrefixed(prefixedHex);
        return ::algName(a);
    } catch (...) {
        return ::algName(defaultAlgFromEnv());
    }
}

std::string prettyNameFromTag(const std::string& tagIn) {
    const std::string tag = toLower(tagIn);
    if (tag == "ecdsa" || tag == "ecdsa-p256" || tag == "p256") return "ECDSA P-256";
    if (tag == "falcon-512" || tag == "falcon")                return "Falcon-512";
    return "Dilithium 2";
}

static std::string tagFromPrettyName(const std::string& prettyIn) {
    const std::string s = toLower(prettyIn);
    if (s.find("ecdsa") != std::string::npos)        return "ecdsa";
    if (s.find("falcon") != std::string::npos)       return "falcon-512";
    if (s.find("dilithium") != std::string::npos)    return "dilithium-2";
    return ::algName(defaultAlgFromEnv());
}

std::string prefixKeyWithCertAlgo(const std::string& rawHex, const std::string& certAlgoName) {
    const std::string tag = tagFromPrettyName(certAlgoName);
    return "ALG:" + tag + ":" + rawHex;
}

} // namespace pqcdsa
