//
// Generated file, do not edit! Created by nedtool 5.7 from veins/pqcdsa//Certificate.msg.
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
#include "Certificate_m.h"

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

Register_Class(Certificate)

Certificate::Certificate(const char *name, short kind) : ::omnetpp::cMessage(name, kind)
{
}

Certificate::Certificate(const Certificate& other) : ::omnetpp::cMessage(other)
{
    copy(other);
}

Certificate::~Certificate()
{
    delete [] this->publicKey;
}

Certificate& Certificate::operator=(const Certificate& other)
{
    if (this == &other) return *this;
    ::omnetpp::cMessage::operator=(other);
    copy(other);
    return *this;
}

void Certificate::copy(const Certificate& other)
{
    this->version = other.version;
    this->certType = other.certType;
    this->issuerType = other.issuerType;
    for (size_t i = 0; i < 8; i++) {
        this->issuerDigest[i] = other.issuerDigest[i];
    }
    this->subjectId = other.subjectId;
    for (size_t i = 0; i < 3; i++) {
        this->cracaId[i] = other.cracaId[i];
    }
    this->crlSeries = other.crlSeries;
    this->validityStart = other.validityStart;
    this->validityDuration = other.validityDuration;
    this->appPermPsid = other.appPermPsid;
    this->algoName = other.algoName;
    delete [] this->publicKey;
    this->publicKey = (other.publicKey_arraysize==0) ? nullptr : new uint8_t[other.publicKey_arraysize];
    publicKey_arraysize = other.publicKey_arraysize;
    for (size_t i = 0; i < publicKey_arraysize; i++) {
        this->publicKey[i] = other.publicKey[i];
    }
}

void Certificate::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cMessage::parsimPack(b);
    doParsimPacking(b,this->version);
    doParsimPacking(b,this->certType);
    doParsimPacking(b,this->issuerType);
    doParsimArrayPacking(b,this->issuerDigest,8);
    doParsimPacking(b,this->subjectId);
    doParsimArrayPacking(b,this->cracaId,3);
    doParsimPacking(b,this->crlSeries);
    doParsimPacking(b,this->validityStart);
    doParsimPacking(b,this->validityDuration);
    doParsimPacking(b,this->appPermPsid);
    doParsimPacking(b,this->algoName);
    b->pack(publicKey_arraysize);
    doParsimArrayPacking(b,this->publicKey,publicKey_arraysize);
}

void Certificate::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->version);
    doParsimUnpacking(b,this->certType);
    doParsimUnpacking(b,this->issuerType);
    doParsimArrayUnpacking(b,this->issuerDigest,8);
    doParsimUnpacking(b,this->subjectId);
    doParsimArrayUnpacking(b,this->cracaId,3);
    doParsimUnpacking(b,this->crlSeries);
    doParsimUnpacking(b,this->validityStart);
    doParsimUnpacking(b,this->validityDuration);
    doParsimUnpacking(b,this->appPermPsid);
    doParsimUnpacking(b,this->algoName);
    delete [] this->publicKey;
    b->unpack(publicKey_arraysize);
    if (publicKey_arraysize == 0) {
        this->publicKey = nullptr;
    } else {
        this->publicKey = new uint8_t[publicKey_arraysize];
        doParsimArrayUnpacking(b,this->publicKey,publicKey_arraysize);
    }
}

uint8_t Certificate::getVersion() const
{
    return this->version;
}

void Certificate::setVersion(uint8_t version)
{
    this->version = version;
}

uint8_t Certificate::getCertType() const
{
    return this->certType;
}

void Certificate::setCertType(uint8_t certType)
{
    this->certType = certType;
}

uint8_t Certificate::getIssuerType() const
{
    return this->issuerType;
}

void Certificate::setIssuerType(uint8_t issuerType)
{
    this->issuerType = issuerType;
}

size_t Certificate::getIssuerDigestArraySize() const
{
    return 8;
}

uint8_t Certificate::getIssuerDigest(size_t k) const
{
    if (k >= 8) throw omnetpp::cRuntimeError("Array of size 8 indexed by %lu", (unsigned long)k);
    return this->issuerDigest[k];
}

void Certificate::setIssuerDigest(size_t k, uint8_t issuerDigest)
{
    if (k >= 8) throw omnetpp::cRuntimeError("Array of size 8 indexed by %lu", (unsigned long)k);
    this->issuerDigest[k] = issuerDigest;
}

const char * Certificate::getSubjectId() const
{
    return this->subjectId.c_str();
}

void Certificate::setSubjectId(const char * subjectId)
{
    this->subjectId = subjectId;
}

size_t Certificate::getCracaIdArraySize() const
{
    return 3;
}

uint8_t Certificate::getCracaId(size_t k) const
{
    if (k >= 3) throw omnetpp::cRuntimeError("Array of size 3 indexed by %lu", (unsigned long)k);
    return this->cracaId[k];
}

void Certificate::setCracaId(size_t k, uint8_t cracaId)
{
    if (k >= 3) throw omnetpp::cRuntimeError("Array of size 3 indexed by %lu", (unsigned long)k);
    this->cracaId[k] = cracaId;
}

uint16_t Certificate::getCrlSeries() const
{
    return this->crlSeries;
}

void Certificate::setCrlSeries(uint16_t crlSeries)
{
    this->crlSeries = crlSeries;
}

int64_t Certificate::getValidityStart() const
{
    return this->validityStart;
}

void Certificate::setValidityStart(int64_t validityStart)
{
    this->validityStart = validityStart;
}

int64_t Certificate::getValidityDuration() const
{
    return this->validityDuration;
}

void Certificate::setValidityDuration(int64_t validityDuration)
{
    this->validityDuration = validityDuration;
}

uint32_t Certificate::getAppPermPsid() const
{
    return this->appPermPsid;
}

void Certificate::setAppPermPsid(uint32_t appPermPsid)
{
    this->appPermPsid = appPermPsid;
}

const char * Certificate::getAlgoName() const
{
    return this->algoName.c_str();
}

void Certificate::setAlgoName(const char * algoName)
{
    this->algoName = algoName;
}

size_t Certificate::getPublicKeyArraySize() const
{
    return publicKey_arraysize;
}

uint8_t Certificate::getPublicKey(size_t k) const
{
    if (k >= publicKey_arraysize) throw omnetpp::cRuntimeError("Array of size publicKey_arraysize indexed by %lu", (unsigned long)k);
    return this->publicKey[k];
}

void Certificate::setPublicKeyArraySize(size_t newSize)
{
    uint8_t *publicKey2 = (newSize==0) ? nullptr : new uint8_t[newSize];
    size_t minSize = publicKey_arraysize < newSize ? publicKey_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        publicKey2[i] = this->publicKey[i];
    for (size_t i = minSize; i < newSize; i++)
        publicKey2[i] = 0;
    delete [] this->publicKey;
    this->publicKey = publicKey2;
    publicKey_arraysize = newSize;
}

void Certificate::setPublicKey(size_t k, uint8_t publicKey)
{
    if (k >= publicKey_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    this->publicKey[k] = publicKey;
}

void Certificate::insertPublicKey(size_t k, uint8_t publicKey)
{
    if (k > publicKey_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = publicKey_arraysize + 1;
    uint8_t *publicKey2 = new uint8_t[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        publicKey2[i] = this->publicKey[i];
    publicKey2[k] = publicKey;
    for (i = k + 1; i < newSize; i++)
        publicKey2[i] = this->publicKey[i-1];
    delete [] this->publicKey;
    this->publicKey = publicKey2;
    publicKey_arraysize = newSize;
}

void Certificate::insertPublicKey(uint8_t publicKey)
{
    insertPublicKey(publicKey_arraysize, publicKey);
}

void Certificate::erasePublicKey(size_t k)
{
    if (k >= publicKey_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = publicKey_arraysize - 1;
    uint8_t *publicKey2 = (newSize == 0) ? nullptr : new uint8_t[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        publicKey2[i] = this->publicKey[i];
    for (i = k; i < newSize; i++)
        publicKey2[i] = this->publicKey[i+1];
    delete [] this->publicKey;
    this->publicKey = publicKey2;
    publicKey_arraysize = newSize;
}

class CertificateDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_version,
        FIELD_certType,
        FIELD_issuerType,
        FIELD_issuerDigest,
        FIELD_subjectId,
        FIELD_cracaId,
        FIELD_crlSeries,
        FIELD_validityStart,
        FIELD_validityDuration,
        FIELD_appPermPsid,
        FIELD_algoName,
        FIELD_publicKey,
    };
  public:
    CertificateDescriptor();
    virtual ~CertificateDescriptor();

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

Register_ClassDescriptor(CertificateDescriptor)

CertificateDescriptor::CertificateDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(Certificate)), "omnetpp::cMessage")
{
    propertynames = nullptr;
}

CertificateDescriptor::~CertificateDescriptor()
{
    delete[] propertynames;
}

bool CertificateDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<Certificate *>(obj)!=nullptr;
}

const char **CertificateDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *CertificateDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int CertificateDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 12+basedesc->getFieldCount() : 12;
}

unsigned int CertificateDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_version
        FD_ISEDITABLE,    // FIELD_certType
        FD_ISEDITABLE,    // FIELD_issuerType
        FD_ISARRAY | FD_ISEDITABLE,    // FIELD_issuerDigest
        FD_ISEDITABLE,    // FIELD_subjectId
        FD_ISARRAY | FD_ISEDITABLE,    // FIELD_cracaId
        FD_ISEDITABLE,    // FIELD_crlSeries
        FD_ISEDITABLE,    // FIELD_validityStart
        FD_ISEDITABLE,    // FIELD_validityDuration
        FD_ISEDITABLE,    // FIELD_appPermPsid
        FD_ISEDITABLE,    // FIELD_algoName
        FD_ISARRAY | FD_ISEDITABLE,    // FIELD_publicKey
    };
    return (field >= 0 && field < 12) ? fieldTypeFlags[field] : 0;
}

const char *CertificateDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "version",
        "certType",
        "issuerType",
        "issuerDigest",
        "subjectId",
        "cracaId",
        "crlSeries",
        "validityStart",
        "validityDuration",
        "appPermPsid",
        "algoName",
        "publicKey",
    };
    return (field >= 0 && field < 12) ? fieldNames[field] : nullptr;
}

int CertificateDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'v' && strcmp(fieldName, "version") == 0) return base+0;
    if (fieldName[0] == 'c' && strcmp(fieldName, "certType") == 0) return base+1;
    if (fieldName[0] == 'i' && strcmp(fieldName, "issuerType") == 0) return base+2;
    if (fieldName[0] == 'i' && strcmp(fieldName, "issuerDigest") == 0) return base+3;
    if (fieldName[0] == 's' && strcmp(fieldName, "subjectId") == 0) return base+4;
    if (fieldName[0] == 'c' && strcmp(fieldName, "cracaId") == 0) return base+5;
    if (fieldName[0] == 'c' && strcmp(fieldName, "crlSeries") == 0) return base+6;
    if (fieldName[0] == 'v' && strcmp(fieldName, "validityStart") == 0) return base+7;
    if (fieldName[0] == 'v' && strcmp(fieldName, "validityDuration") == 0) return base+8;
    if (fieldName[0] == 'a' && strcmp(fieldName, "appPermPsid") == 0) return base+9;
    if (fieldName[0] == 'a' && strcmp(fieldName, "algoName") == 0) return base+10;
    if (fieldName[0] == 'p' && strcmp(fieldName, "publicKey") == 0) return base+11;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *CertificateDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "uint8_t",    // FIELD_version
        "uint8_t",    // FIELD_certType
        "uint8_t",    // FIELD_issuerType
        "uint8_t",    // FIELD_issuerDigest
        "string",    // FIELD_subjectId
        "uint8_t",    // FIELD_cracaId
        "uint16_t",    // FIELD_crlSeries
        "int64_t",    // FIELD_validityStart
        "int64_t",    // FIELD_validityDuration
        "uint32_t",    // FIELD_appPermPsid
        "string",    // FIELD_algoName
        "uint8_t",    // FIELD_publicKey
    };
    return (field >= 0 && field < 12) ? fieldTypeStrings[field] : nullptr;
}

const char **CertificateDescriptor::getFieldPropertyNames(int field) const
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

const char *CertificateDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int CertificateDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    Certificate *pp = (Certificate *)object; (void)pp;
    switch (field) {
        case FIELD_issuerDigest: return 8;
        case FIELD_cracaId: return 3;
        case FIELD_publicKey: return pp->getPublicKeyArraySize();
        default: return 0;
    }
}

const char *CertificateDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    Certificate *pp = (Certificate *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string CertificateDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    Certificate *pp = (Certificate *)object; (void)pp;
    switch (field) {
        case FIELD_version: return ulong2string(pp->getVersion());
        case FIELD_certType: return ulong2string(pp->getCertType());
        case FIELD_issuerType: return ulong2string(pp->getIssuerType());
        case FIELD_issuerDigest: return ulong2string(pp->getIssuerDigest(i));
        case FIELD_subjectId: return oppstring2string(pp->getSubjectId());
        case FIELD_cracaId: return ulong2string(pp->getCracaId(i));
        case FIELD_crlSeries: return ulong2string(pp->getCrlSeries());
        case FIELD_validityStart: return int642string(pp->getValidityStart());
        case FIELD_validityDuration: return int642string(pp->getValidityDuration());
        case FIELD_appPermPsid: return ulong2string(pp->getAppPermPsid());
        case FIELD_algoName: return oppstring2string(pp->getAlgoName());
        case FIELD_publicKey: return ulong2string(pp->getPublicKey(i));
        default: return "";
    }
}

bool CertificateDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    Certificate *pp = (Certificate *)object; (void)pp;
    switch (field) {
        case FIELD_version: pp->setVersion(string2ulong(value)); return true;
        case FIELD_certType: pp->setCertType(string2ulong(value)); return true;
        case FIELD_issuerType: pp->setIssuerType(string2ulong(value)); return true;
        case FIELD_issuerDigest: pp->setIssuerDigest(i,string2ulong(value)); return true;
        case FIELD_subjectId: pp->setSubjectId((value)); return true;
        case FIELD_cracaId: pp->setCracaId(i,string2ulong(value)); return true;
        case FIELD_crlSeries: pp->setCrlSeries(string2ulong(value)); return true;
        case FIELD_validityStart: pp->setValidityStart(string2int64(value)); return true;
        case FIELD_validityDuration: pp->setValidityDuration(string2int64(value)); return true;
        case FIELD_appPermPsid: pp->setAppPermPsid(string2ulong(value)); return true;
        case FIELD_algoName: pp->setAlgoName((value)); return true;
        case FIELD_publicKey: pp->setPublicKey(i,string2ulong(value)); return true;
        default: return false;
    }
}

const char *CertificateDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *CertificateDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    Certificate *pp = (Certificate *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

