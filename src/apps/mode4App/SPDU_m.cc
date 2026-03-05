//
// Generated file, do not edit! Created by nedtool 5.7 from veins/pqcdsa/SPDU.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include "SPDU_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

namespace {
template <class T> inline
typename std::enable_if<std::is_polymorphic<T>::value && std::is_base_of<omnetpp::cObject,T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)(static_cast<const omnetpp::cObject *>(t));
}

template <class T> inline
typename std::enable_if<std::is_polymorphic<T>::value && !std::is_base_of<omnetpp::cObject,T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)dynamic_cast<const void *>(t);
}

template <class T> inline
typename std::enable_if<!std::is_polymorphic<T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)static_cast<const void *>(t);
}

}


// forward
template<typename T, typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec);

// Template rule to generate operator<< for shared_ptr<T>
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const std::shared_ptr<T>& t) { return out << t.get(); }

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// operator<< for std::vector<T>
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');

    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

Register_Class(SPDU)

SPDU::SPDU(const char *name, short kind) : ::omnetpp::cPacket(name, kind)
{
    take(&this->bsm);
    take(&this->cert);
}

SPDU::SPDU(const SPDU& other) : ::omnetpp::cPacket(other)
{
    take(&this->bsm);
    take(&this->cert);
    copy(other);
}

SPDU::~SPDU()
{
    drop(&this->bsm);
    drop(&this->cert);
    delete [] this->signature;
}

SPDU& SPDU::operator=(const SPDU& other)
{
    if (this == &other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void SPDU::copy(const SPDU& other)
{
    this->protocolVersion = other.protocolVersion;
    this->psid = other.psid;
    this->generationTime = other.generationTime;
    this->genLocation_lat = other.genLocation_lat;
    this->genLocation_lon = other.genLocation_lon;
    this->genLocation_elev = other.genLocation_elev;
    this->bsm = other.bsm;
    this->bsm.setName(other.bsm.getName());
    this->signerType = other.signerType;
    for (size_t i = 0; i < 8; i++) {
        this->signerDigest[i] = other.signerDigest[i];
    }
    this->cert = other.cert;
    this->cert.setName(other.cert.getName());
    delete [] this->signature;
    this->signature = (other.signature_arraysize==0) ? nullptr : new uint8_t[other.signature_arraysize];
    signature_arraysize = other.signature_arraysize;
    for (size_t i = 0; i < signature_arraysize; i++) {
        this->signature[i] = other.signature[i];
    }
}

void SPDU::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->protocolVersion);
    doParsimPacking(b,this->psid);
    doParsimPacking(b,this->generationTime);
    doParsimPacking(b,this->genLocation_lat);
    doParsimPacking(b,this->genLocation_lon);
    doParsimPacking(b,this->genLocation_elev);
    doParsimPacking(b,this->bsm);
    doParsimPacking(b,this->signerType);
    doParsimArrayPacking(b,this->signerDigest,8);
    doParsimPacking(b,this->cert);
    b->pack(signature_arraysize);
    doParsimArrayPacking(b,this->signature,signature_arraysize);
}

void SPDU::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->protocolVersion);
    doParsimUnpacking(b,this->psid);
    doParsimUnpacking(b,this->generationTime);
    doParsimUnpacking(b,this->genLocation_lat);
    doParsimUnpacking(b,this->genLocation_lon);
    doParsimUnpacking(b,this->genLocation_elev);
    doParsimUnpacking(b,this->bsm);
    doParsimUnpacking(b,this->signerType);
    doParsimArrayUnpacking(b,this->signerDigest,8);
    doParsimUnpacking(b,this->cert);
    delete [] this->signature;
    b->unpack(signature_arraysize);
    if (signature_arraysize == 0) {
        this->signature = nullptr;
    } else {
        this->signature = new uint8_t[signature_arraysize];
        doParsimArrayUnpacking(b,this->signature,signature_arraysize);
    }
}

uint8_t SPDU::getProtocolVersion() const
{
    return this->protocolVersion;
}

void SPDU::setProtocolVersion(uint8_t protocolVersion)
{
    this->protocolVersion = protocolVersion;
}

uint32_t SPDU::getPsid() const
{
    return this->psid;
}

void SPDU::setPsid(uint32_t psid)
{
    this->psid = psid;
}

int64_t SPDU::getGenerationTime() const
{
    return this->generationTime;
}

void SPDU::setGenerationTime(int64_t generationTime)
{
    this->generationTime = generationTime;
}

int32_t SPDU::getGenLocation_lat() const
{
    return this->genLocation_lat;
}

void SPDU::setGenLocation_lat(int32_t genLocation_lat)
{
    this->genLocation_lat = genLocation_lat;
}

int32_t SPDU::getGenLocation_lon() const
{
    return this->genLocation_lon;
}

void SPDU::setGenLocation_lon(int32_t genLocation_lon)
{
    this->genLocation_lon = genLocation_lon;
}

int16_t SPDU::getGenLocation_elev() const
{
    return this->genLocation_elev;
}

void SPDU::setGenLocation_elev(int16_t genLocation_elev)
{
    this->genLocation_elev = genLocation_elev;
}

const BSM& SPDU::getBsm() const
{
    return this->bsm;
}

void SPDU::setBsm(const BSM& bsm)
{
    this->bsm = bsm;
}

uint8_t SPDU::getSignerType() const
{
    return this->signerType;
}

void SPDU::setSignerType(uint8_t signerType)
{
    this->signerType = signerType;
}

size_t SPDU::getSignerDigestArraySize() const
{
    return 8;
}

uint8_t SPDU::getSignerDigest(size_t k) const
{
    if (k >= 8) throw omnetpp::cRuntimeError("Array of size 8 indexed by %lu", (unsigned long)k);
    return this->signerDigest[k];
}

void SPDU::setSignerDigest(size_t k, uint8_t signerDigest)
{
    if (k >= 8) throw omnetpp::cRuntimeError("Array of size 8 indexed by %lu", (unsigned long)k);
    this->signerDigest[k] = signerDigest;
}

const Certificate& SPDU::getCert() const
{
    return this->cert;
}

void SPDU::setCert(const Certificate& cert)
{
    this->cert = cert;
}

size_t SPDU::getSignatureArraySize() const
{
    return signature_arraysize;
}

uint8_t SPDU::getSignature(size_t k) const
{
    if (k >= signature_arraysize) throw omnetpp::cRuntimeError("Array of size signature_arraysize indexed by %lu", (unsigned long)k);
    return this->signature[k];
}

void SPDU::setSignatureArraySize(size_t newSize)
{
    uint8_t *signature2 = (newSize==0) ? nullptr : new uint8_t[newSize];
    size_t minSize = signature_arraysize < newSize ? signature_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        signature2[i] = this->signature[i];
    for (size_t i = minSize; i < newSize; i++)
        signature2[i] = 0;
    delete [] this->signature;
    this->signature = signature2;
    signature_arraysize = newSize;
}

void SPDU::setSignature(size_t k, uint8_t signature)
{
    if (k >= signature_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    this->signature[k] = signature;
}

void SPDU::insertSignature(size_t k, uint8_t signature)
{
    if (k > signature_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = signature_arraysize + 1;
    uint8_t *signature2 = new uint8_t[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        signature2[i] = this->signature[i];
    signature2[k] = signature;
    for (i = k + 1; i < newSize; i++)
        signature2[i] = this->signature[i-1];
    delete [] this->signature;
    this->signature = signature2;
    signature_arraysize = newSize;
}

void SPDU::insertSignature(uint8_t signature)
{
    insertSignature(signature_arraysize, signature);
}

void SPDU::eraseSignature(size_t k)
{
    if (k >= signature_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = signature_arraysize - 1;
    uint8_t *signature2 = (newSize == 0) ? nullptr : new uint8_t[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        signature2[i] = this->signature[i];
    for (i = k; i < newSize; i++)
        signature2[i] = this->signature[i+1];
    delete [] this->signature;
    this->signature = signature2;
    signature_arraysize = newSize;
}

class SPDUDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_protocolVersion,
        FIELD_psid,
        FIELD_generationTime,
        FIELD_genLocation_lat,
        FIELD_genLocation_lon,
        FIELD_genLocation_elev,
        FIELD_bsm,
        FIELD_signerType,
        FIELD_signerDigest,
        FIELD_cert,
        FIELD_signature,
    };
  public:
    SPDUDescriptor();
    virtual ~SPDUDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(SPDUDescriptor)

SPDUDescriptor::SPDUDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(SPDU)), "omnetpp::cPacket")
{
    propertynames = nullptr;
}

SPDUDescriptor::~SPDUDescriptor()
{
    delete[] propertynames;
}

bool SPDUDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<SPDU *>(obj)!=nullptr;
}

const char **SPDUDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *SPDUDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int SPDUDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 11+basedesc->getFieldCount() : 11;
}

unsigned int SPDUDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_protocolVersion
        FD_ISEDITABLE,    // FIELD_psid
        FD_ISEDITABLE,    // FIELD_generationTime
        FD_ISEDITABLE,    // FIELD_genLocation_lat
        FD_ISEDITABLE,    // FIELD_genLocation_lon
        FD_ISEDITABLE,    // FIELD_genLocation_elev
        FD_ISCOMPOUND | FD_ISCOBJECT | FD_ISCOWNEDOBJECT,    // FIELD_bsm
        FD_ISEDITABLE,    // FIELD_signerType
        FD_ISARRAY | FD_ISEDITABLE,    // FIELD_signerDigest
        FD_ISCOMPOUND | FD_ISCOBJECT | FD_ISCOWNEDOBJECT,    // FIELD_cert
        FD_ISARRAY | FD_ISEDITABLE,    // FIELD_signature
    };
    return (field >= 0 && field < 11) ? fieldTypeFlags[field] : 0;
}

const char *SPDUDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "protocolVersion",
        "psid",
        "generationTime",
        "genLocation_lat",
        "genLocation_lon",
        "genLocation_elev",
        "bsm",
        "signerType",
        "signerDigest",
        "cert",
        "signature",
    };
    return (field >= 0 && field < 11) ? fieldNames[field] : nullptr;
}

int SPDUDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'p' && strcmp(fieldName, "protocolVersion") == 0) return base+0;
    if (fieldName[0] == 'p' && strcmp(fieldName, "psid") == 0) return base+1;
    if (fieldName[0] == 'g' && strcmp(fieldName, "generationTime") == 0) return base+2;
    if (fieldName[0] == 'g' && strcmp(fieldName, "genLocation_lat") == 0) return base+3;
    if (fieldName[0] == 'g' && strcmp(fieldName, "genLocation_lon") == 0) return base+4;
    if (fieldName[0] == 'g' && strcmp(fieldName, "genLocation_elev") == 0) return base+5;
    if (fieldName[0] == 'b' && strcmp(fieldName, "bsm") == 0) return base+6;
    if (fieldName[0] == 's' && strcmp(fieldName, "signerType") == 0) return base+7;
    if (fieldName[0] == 's' && strcmp(fieldName, "signerDigest") == 0) return base+8;
    if (fieldName[0] == 'c' && strcmp(fieldName, "cert") == 0) return base+9;
    if (fieldName[0] == 's' && strcmp(fieldName, "signature") == 0) return base+10;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *SPDUDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "uint8_t",    // FIELD_protocolVersion
        "uint32_t",    // FIELD_psid
        "int64_t",    // FIELD_generationTime
        "int32_t",    // FIELD_genLocation_lat
        "int32_t",    // FIELD_genLocation_lon
        "int16_t",    // FIELD_genLocation_elev
        "BSM",    // FIELD_bsm
        "uint8_t",    // FIELD_signerType
        "uint8_t",    // FIELD_signerDigest
        "Certificate",    // FIELD_cert
        "uint8_t",    // FIELD_signature
    };
    return (field >= 0 && field < 11) ? fieldTypeStrings[field] : nullptr;
}

const char **SPDUDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *SPDUDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int SPDUDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    SPDU *pp = (SPDU *)object; (void)pp;
    switch (field) {
        case FIELD_signerDigest: return 8;
        case FIELD_signature: return pp->getSignatureArraySize();
        default: return 0;
    }
}

const char *SPDUDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    SPDU *pp = (SPDU *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string SPDUDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    SPDU *pp = (SPDU *)object; (void)pp;
    switch (field) {
        case FIELD_protocolVersion: return ulong2string(pp->getProtocolVersion());
        case FIELD_psid: return ulong2string(pp->getPsid());
        case FIELD_generationTime: return int642string(pp->getGenerationTime());
        case FIELD_genLocation_lat: return long2string(pp->getGenLocation_lat());
        case FIELD_genLocation_lon: return long2string(pp->getGenLocation_lon());
        case FIELD_genLocation_elev: return long2string(pp->getGenLocation_elev());
        case FIELD_bsm: {std::stringstream out; out << pp->getBsm(); return out.str();}
        case FIELD_signerType: return ulong2string(pp->getSignerType());
        case FIELD_signerDigest: return ulong2string(pp->getSignerDigest(i));
        case FIELD_cert: {std::stringstream out; out << pp->getCert(); return out.str();}
        case FIELD_signature: return ulong2string(pp->getSignature(i));
        default: return "";
    }
}

bool SPDUDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    SPDU *pp = (SPDU *)object; (void)pp;
    switch (field) {
        case FIELD_protocolVersion: pp->setProtocolVersion(string2ulong(value)); return true;
        case FIELD_psid: pp->setPsid(string2ulong(value)); return true;
        case FIELD_generationTime: pp->setGenerationTime(string2int64(value)); return true;
        case FIELD_genLocation_lat: pp->setGenLocation_lat(string2long(value)); return true;
        case FIELD_genLocation_lon: pp->setGenLocation_lon(string2long(value)); return true;
        case FIELD_genLocation_elev: pp->setGenLocation_elev(string2long(value)); return true;
        case FIELD_signerType: pp->setSignerType(string2ulong(value)); return true;
        case FIELD_signerDigest: pp->setSignerDigest(i,string2ulong(value)); return true;
        case FIELD_signature: pp->setSignature(i,string2ulong(value)); return true;
        default: return false;
    }
}

const char *SPDUDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_bsm: return omnetpp::opp_typename(typeid(BSM));
        case FIELD_cert: return omnetpp::opp_typename(typeid(Certificate));
        default: return nullptr;
    };
}

void *SPDUDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    SPDU *pp = (SPDU *)object; (void)pp;
    switch (field) {
        case FIELD_bsm: return toVoidPtr(&pp->getBsm()); break;
        case FIELD_cert: return toVoidPtr(&pp->getCert()); break;
        default: return nullptr;
    }
}

