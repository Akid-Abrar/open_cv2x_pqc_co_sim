//
// Generated file, do not edit! Created by nedtool 5.7 from common/LteControlInfo.msg.
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
#include "LteControlInfo_m.h"

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
    for (int i=0; i<n; i++) {
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
    for (int i=0; i<n; i++) {
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
    for (int i=0; i<n; i++) {
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


// forward
template<typename T, typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec);

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

Register_Class(LteControlInfo)

LteControlInfo::LteControlInfo() : ::omnetpp::cObject()
{
    this->sourceId = 0;
    this->destId = 0;
    this->direction = 0;
    this->d2dTxPeerId = 0;
    this->d2dRxPeerId = 0;
    this->application = 0;
    this->traffic = 0;
    this->rlcType = 0;
    this->lcid = 0;
    this->multicastGroupId = -1;
    this->sequenceNumber = 0;
}

LteControlInfo::LteControlInfo(const LteControlInfo& other) : ::omnetpp::cObject(other)
{
    copy(other);
}

LteControlInfo::~LteControlInfo()
{
}

LteControlInfo& LteControlInfo::operator=(const LteControlInfo& other)
{
    if (this==&other) return *this;
    ::omnetpp::cObject::operator=(other);
    copy(other);
    return *this;
}

void LteControlInfo::copy(const LteControlInfo& other)
{
    this->sourceId = other.sourceId;
    this->destId = other.destId;
    this->direction = other.direction;
    this->d2dTxPeerId = other.d2dTxPeerId;
    this->d2dRxPeerId = other.d2dRxPeerId;
    this->application = other.application;
    this->traffic = other.traffic;
    this->rlcType = other.rlcType;
    this->lcid = other.lcid;
    this->multicastGroupId = other.multicastGroupId;
    this->sequenceNumber = other.sequenceNumber;
}

void LteControlInfo::parsimPack(omnetpp::cCommBuffer *b) const
{
    doParsimPacking(b,this->sourceId);
    doParsimPacking(b,this->destId);
    doParsimPacking(b,this->direction);
    doParsimPacking(b,this->d2dTxPeerId);
    doParsimPacking(b,this->d2dRxPeerId);
    doParsimPacking(b,this->application);
    doParsimPacking(b,this->traffic);
    doParsimPacking(b,this->rlcType);
    doParsimPacking(b,this->lcid);
    doParsimPacking(b,this->multicastGroupId);
    doParsimPacking(b,this->sequenceNumber);
}

void LteControlInfo::parsimUnpack(omnetpp::cCommBuffer *b)
{
    doParsimUnpacking(b,this->sourceId);
    doParsimUnpacking(b,this->destId);
    doParsimUnpacking(b,this->direction);
    doParsimUnpacking(b,this->d2dTxPeerId);
    doParsimUnpacking(b,this->d2dRxPeerId);
    doParsimUnpacking(b,this->application);
    doParsimUnpacking(b,this->traffic);
    doParsimUnpacking(b,this->rlcType);
    doParsimUnpacking(b,this->lcid);
    doParsimUnpacking(b,this->multicastGroupId);
    doParsimUnpacking(b,this->sequenceNumber);
}

uint16_t LteControlInfo::getSourceId() const
{
    return this->sourceId;
}

void LteControlInfo::setSourceId(uint16_t sourceId)
{
    this->sourceId = sourceId;
}

uint16_t LteControlInfo::getDestId() const
{
    return this->destId;
}

void LteControlInfo::setDestId(uint16_t destId)
{
    this->destId = destId;
}

unsigned short LteControlInfo::getDirection() const
{
    return this->direction;
}

void LteControlInfo::setDirection(unsigned short direction)
{
    this->direction = direction;
}

uint16_t LteControlInfo::getD2dTxPeerId() const
{
    return this->d2dTxPeerId;
}

void LteControlInfo::setD2dTxPeerId(uint16_t d2dTxPeerId)
{
    this->d2dTxPeerId = d2dTxPeerId;
}

uint16_t LteControlInfo::getD2dRxPeerId() const
{
    return this->d2dRxPeerId;
}

void LteControlInfo::setD2dRxPeerId(uint16_t d2dRxPeerId)
{
    this->d2dRxPeerId = d2dRxPeerId;
}

unsigned short LteControlInfo::getApplication() const
{
    return this->application;
}

void LteControlInfo::setApplication(unsigned short application)
{
    this->application = application;
}

unsigned short LteControlInfo::getTraffic() const
{
    return this->traffic;
}

void LteControlInfo::setTraffic(unsigned short traffic)
{
    this->traffic = traffic;
}

unsigned short LteControlInfo::getRlcType() const
{
    return this->rlcType;
}

void LteControlInfo::setRlcType(unsigned short rlcType)
{
    this->rlcType = rlcType;
}

uint16_t LteControlInfo::getLcid() const
{
    return this->lcid;
}

void LteControlInfo::setLcid(uint16_t lcid)
{
    this->lcid = lcid;
}

int32_t LteControlInfo::getMulticastGroupId() const
{
    return this->multicastGroupId;
}

void LteControlInfo::setMulticastGroupId(int32_t multicastGroupId)
{
    this->multicastGroupId = multicastGroupId;
}

unsigned int LteControlInfo::getSequenceNumber() const
{
    return this->sequenceNumber;
}

void LteControlInfo::setSequenceNumber(unsigned int sequenceNumber)
{
    this->sequenceNumber = sequenceNumber;
}

class LteControlInfoDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    LteControlInfoDescriptor();
    virtual ~LteControlInfoDescriptor();

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

Register_ClassDescriptor(LteControlInfoDescriptor)

LteControlInfoDescriptor::LteControlInfoDescriptor() : omnetpp::cClassDescriptor("LteControlInfo", "omnetpp::cObject")
{
    propertynames = nullptr;
}

LteControlInfoDescriptor::~LteControlInfoDescriptor()
{
    delete[] propertynames;
}

bool LteControlInfoDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<LteControlInfo *>(obj)!=nullptr;
}

const char **LteControlInfoDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *LteControlInfoDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int LteControlInfoDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 11+basedesc->getFieldCount() : 11;
}

unsigned int LteControlInfoDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<11) ? fieldTypeFlags[field] : 0;
}

const char *LteControlInfoDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "sourceId",
        "destId",
        "direction",
        "d2dTxPeerId",
        "d2dRxPeerId",
        "application",
        "traffic",
        "rlcType",
        "lcid",
        "multicastGroupId",
        "sequenceNumber",
    };
    return (field>=0 && field<11) ? fieldNames[field] : nullptr;
}

int LteControlInfoDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "sourceId")==0) return base+0;
    if (fieldName[0]=='d' && strcmp(fieldName, "destId")==0) return base+1;
    if (fieldName[0]=='d' && strcmp(fieldName, "direction")==0) return base+2;
    if (fieldName[0]=='d' && strcmp(fieldName, "d2dTxPeerId")==0) return base+3;
    if (fieldName[0]=='d' && strcmp(fieldName, "d2dRxPeerId")==0) return base+4;
    if (fieldName[0]=='a' && strcmp(fieldName, "application")==0) return base+5;
    if (fieldName[0]=='t' && strcmp(fieldName, "traffic")==0) return base+6;
    if (fieldName[0]=='r' && strcmp(fieldName, "rlcType")==0) return base+7;
    if (fieldName[0]=='l' && strcmp(fieldName, "lcid")==0) return base+8;
    if (fieldName[0]=='m' && strcmp(fieldName, "multicastGroupId")==0) return base+9;
    if (fieldName[0]=='s' && strcmp(fieldName, "sequenceNumber")==0) return base+10;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *LteControlInfoDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "uint16",
        "uint16",
        "unsigned short",
        "uint16",
        "uint16",
        "unsigned short",
        "unsigned short",
        "unsigned short",
        "uint16",
        "int32",
        "unsigned int",
    };
    return (field>=0 && field<11) ? fieldTypeStrings[field] : nullptr;
}

const char **LteControlInfoDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case 2: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        case 5: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        case 6: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        case 7: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *LteControlInfoDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case 2:
            if (!strcmp(propertyname,"enum")) return "Direction";
            return nullptr;
        case 5:
            if (!strcmp(propertyname,"enum")) return "ApplicationType";
            return nullptr;
        case 6:
            if (!strcmp(propertyname,"enum")) return "LteTrafficClass";
            return nullptr;
        case 7:
            if (!strcmp(propertyname,"enum")) return "LteRlcType";
            return nullptr;
        default: return nullptr;
    }
}

int LteControlInfoDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    LteControlInfo *pp = (LteControlInfo *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *LteControlInfoDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    LteControlInfo *pp = (LteControlInfo *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string LteControlInfoDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    LteControlInfo *pp = (LteControlInfo *)object; (void)pp;
    switch (field) {
        case 0: return ulong2string(pp->getSourceId());
        case 1: return ulong2string(pp->getDestId());
        case 2: return enum2string(pp->getDirection(), "Direction");
        case 3: return ulong2string(pp->getD2dTxPeerId());
        case 4: return ulong2string(pp->getD2dRxPeerId());
        case 5: return enum2string(pp->getApplication(), "ApplicationType");
        case 6: return enum2string(pp->getTraffic(), "LteTrafficClass");
        case 7: return enum2string(pp->getRlcType(), "LteRlcType");
        case 8: return ulong2string(pp->getLcid());
        case 9: return long2string(pp->getMulticastGroupId());
        case 10: return ulong2string(pp->getSequenceNumber());
        default: return "";
    }
}

bool LteControlInfoDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    LteControlInfo *pp = (LteControlInfo *)object; (void)pp;
    switch (field) {
        case 0: pp->setSourceId(string2ulong(value)); return true;
        case 1: pp->setDestId(string2ulong(value)); return true;
        case 2: pp->setDirection((Direction)string2enum(value, "Direction")); return true;
        case 3: pp->setD2dTxPeerId(string2ulong(value)); return true;
        case 4: pp->setD2dRxPeerId(string2ulong(value)); return true;
        case 5: pp->setApplication((ApplicationType)string2enum(value, "ApplicationType")); return true;
        case 6: pp->setTraffic((LteTrafficClass)string2enum(value, "LteTrafficClass")); return true;
        case 7: pp->setRlcType((LteRlcType)string2enum(value, "LteRlcType")); return true;
        case 8: pp->setLcid(string2ulong(value)); return true;
        case 9: pp->setMulticastGroupId(string2long(value)); return true;
        case 10: pp->setSequenceNumber(string2ulong(value)); return true;
        default: return false;
    }
}

const char *LteControlInfoDescriptor::getFieldStructName(int field) const
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

void *LteControlInfoDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    LteControlInfo *pp = (LteControlInfo *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(FlowControlInfo)

FlowControlInfo::FlowControlInfo() : ::LteControlInfo()
{
    this->srcAddr = 0;
    this->dstAddr = 0;
    this->srcPort = 0;
    this->dstPort = 0;
    this->headerSize = 0;
}

FlowControlInfo::FlowControlInfo(const FlowControlInfo& other) : ::LteControlInfo(other)
{
    copy(other);
}

FlowControlInfo::~FlowControlInfo()
{
}

FlowControlInfo& FlowControlInfo::operator=(const FlowControlInfo& other)
{
    if (this==&other) return *this;
    ::LteControlInfo::operator=(other);
    copy(other);
    return *this;
}

void FlowControlInfo::copy(const FlowControlInfo& other)
{
    this->srcAddr = other.srcAddr;
    this->dstAddr = other.dstAddr;
    this->srcPort = other.srcPort;
    this->dstPort = other.dstPort;
    this->headerSize = other.headerSize;
}

void FlowControlInfo::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::LteControlInfo::parsimPack(b);
    doParsimPacking(b,this->srcAddr);
    doParsimPacking(b,this->dstAddr);
    doParsimPacking(b,this->srcPort);
    doParsimPacking(b,this->dstPort);
    doParsimPacking(b,this->headerSize);
}

void FlowControlInfo::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::LteControlInfo::parsimUnpack(b);
    doParsimUnpacking(b,this->srcAddr);
    doParsimUnpacking(b,this->dstAddr);
    doParsimUnpacking(b,this->srcPort);
    doParsimUnpacking(b,this->dstPort);
    doParsimUnpacking(b,this->headerSize);
}

uint32_t FlowControlInfo::getSrcAddr() const
{
    return this->srcAddr;
}

void FlowControlInfo::setSrcAddr(uint32_t srcAddr)
{
    this->srcAddr = srcAddr;
}

uint32_t FlowControlInfo::getDstAddr() const
{
    return this->dstAddr;
}

void FlowControlInfo::setDstAddr(uint32_t dstAddr)
{
    this->dstAddr = dstAddr;
}

uint16_t FlowControlInfo::getSrcPort() const
{
    return this->srcPort;
}

void FlowControlInfo::setSrcPort(uint16_t srcPort)
{
    this->srcPort = srcPort;
}

uint16_t FlowControlInfo::getDstPort() const
{
    return this->dstPort;
}

void FlowControlInfo::setDstPort(uint16_t dstPort)
{
    this->dstPort = dstPort;
}

int FlowControlInfo::getHeaderSize() const
{
    return this->headerSize;
}

void FlowControlInfo::setHeaderSize(int headerSize)
{
    this->headerSize = headerSize;
}

class FlowControlInfoDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    FlowControlInfoDescriptor();
    virtual ~FlowControlInfoDescriptor();

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

Register_ClassDescriptor(FlowControlInfoDescriptor)

FlowControlInfoDescriptor::FlowControlInfoDescriptor() : omnetpp::cClassDescriptor("FlowControlInfo", "LteControlInfo")
{
    propertynames = nullptr;
}

FlowControlInfoDescriptor::~FlowControlInfoDescriptor()
{
    delete[] propertynames;
}

bool FlowControlInfoDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<FlowControlInfo *>(obj)!=nullptr;
}

const char **FlowControlInfoDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *FlowControlInfoDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int FlowControlInfoDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 5+basedesc->getFieldCount() : 5;
}

unsigned int FlowControlInfoDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<5) ? fieldTypeFlags[field] : 0;
}

const char *FlowControlInfoDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "srcAddr",
        "dstAddr",
        "srcPort",
        "dstPort",
        "headerSize",
    };
    return (field>=0 && field<5) ? fieldNames[field] : nullptr;
}

int FlowControlInfoDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "srcAddr")==0) return base+0;
    if (fieldName[0]=='d' && strcmp(fieldName, "dstAddr")==0) return base+1;
    if (fieldName[0]=='s' && strcmp(fieldName, "srcPort")==0) return base+2;
    if (fieldName[0]=='d' && strcmp(fieldName, "dstPort")==0) return base+3;
    if (fieldName[0]=='h' && strcmp(fieldName, "headerSize")==0) return base+4;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *FlowControlInfoDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "uint32",
        "uint32",
        "uint16",
        "uint16",
        "int",
    };
    return (field>=0 && field<5) ? fieldTypeStrings[field] : nullptr;
}

const char **FlowControlInfoDescriptor::getFieldPropertyNames(int field) const
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

const char *FlowControlInfoDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int FlowControlInfoDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    FlowControlInfo *pp = (FlowControlInfo *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *FlowControlInfoDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    FlowControlInfo *pp = (FlowControlInfo *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string FlowControlInfoDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    FlowControlInfo *pp = (FlowControlInfo *)object; (void)pp;
    switch (field) {
        case 0: return ulong2string(pp->getSrcAddr());
        case 1: return ulong2string(pp->getDstAddr());
        case 2: return ulong2string(pp->getSrcPort());
        case 3: return ulong2string(pp->getDstPort());
        case 4: return long2string(pp->getHeaderSize());
        default: return "";
    }
}

bool FlowControlInfoDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    FlowControlInfo *pp = (FlowControlInfo *)object; (void)pp;
    switch (field) {
        case 0: pp->setSrcAddr(string2ulong(value)); return true;
        case 1: pp->setDstAddr(string2ulong(value)); return true;
        case 2: pp->setSrcPort(string2ulong(value)); return true;
        case 3: pp->setDstPort(string2ulong(value)); return true;
        case 4: pp->setHeaderSize(string2long(value)); return true;
        default: return false;
    }
}

const char *FlowControlInfoDescriptor::getFieldStructName(int field) const
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

void *FlowControlInfoDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    FlowControlInfo *pp = (FlowControlInfo *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(FlowControlInfoNonIp)

FlowControlInfoNonIp::FlowControlInfoNonIp() : ::LteControlInfo()
{
    this->priority = 0;
    this->duration = 0;
    this->srcAddr = 0;
    this->dstAddr = 0;
    this->creationTime = 0;
}

FlowControlInfoNonIp::FlowControlInfoNonIp(const FlowControlInfoNonIp& other) : ::LteControlInfo(other)
{
    copy(other);
}

FlowControlInfoNonIp::~FlowControlInfoNonIp()
{
}

FlowControlInfoNonIp& FlowControlInfoNonIp::operator=(const FlowControlInfoNonIp& other)
{
    if (this==&other) return *this;
    ::LteControlInfo::operator=(other);
    copy(other);
    return *this;
}

void FlowControlInfoNonIp::copy(const FlowControlInfoNonIp& other)
{
    this->priority = other.priority;
    this->duration = other.duration;
    this->srcAddr = other.srcAddr;
    this->dstAddr = other.dstAddr;
    this->creationTime = other.creationTime;
}

void FlowControlInfoNonIp::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::LteControlInfo::parsimPack(b);
    doParsimPacking(b,this->priority);
    doParsimPacking(b,this->duration);
    doParsimPacking(b,this->srcAddr);
    doParsimPacking(b,this->dstAddr);
    doParsimPacking(b,this->creationTime);
}

void FlowControlInfoNonIp::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::LteControlInfo::parsimUnpack(b);
    doParsimUnpacking(b,this->priority);
    doParsimUnpacking(b,this->duration);
    doParsimUnpacking(b,this->srcAddr);
    doParsimUnpacking(b,this->dstAddr);
    doParsimUnpacking(b,this->creationTime);
}

int FlowControlInfoNonIp::getPriority() const
{
    return this->priority;
}

void FlowControlInfoNonIp::setPriority(int priority)
{
    this->priority = priority;
}

long FlowControlInfoNonIp::getDuration() const
{
    return this->duration;
}

void FlowControlInfoNonIp::setDuration(long duration)
{
    this->duration = duration;
}

long FlowControlInfoNonIp::getSrcAddr() const
{
    return this->srcAddr;
}

void FlowControlInfoNonIp::setSrcAddr(long srcAddr)
{
    this->srcAddr = srcAddr;
}

long FlowControlInfoNonIp::getDstAddr() const
{
    return this->dstAddr;
}

void FlowControlInfoNonIp::setDstAddr(long dstAddr)
{
    this->dstAddr = dstAddr;
}

::omnetpp::simtime_t FlowControlInfoNonIp::getCreationTime() const
{
    return this->creationTime;
}

void FlowControlInfoNonIp::setCreationTime(::omnetpp::simtime_t creationTime)
{
    this->creationTime = creationTime;
}

class FlowControlInfoNonIpDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    FlowControlInfoNonIpDescriptor();
    virtual ~FlowControlInfoNonIpDescriptor();

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

Register_ClassDescriptor(FlowControlInfoNonIpDescriptor)

FlowControlInfoNonIpDescriptor::FlowControlInfoNonIpDescriptor() : omnetpp::cClassDescriptor("FlowControlInfoNonIp", "LteControlInfo")
{
    propertynames = nullptr;
}

FlowControlInfoNonIpDescriptor::~FlowControlInfoNonIpDescriptor()
{
    delete[] propertynames;
}

bool FlowControlInfoNonIpDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<FlowControlInfoNonIp *>(obj)!=nullptr;
}

const char **FlowControlInfoNonIpDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *FlowControlInfoNonIpDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int FlowControlInfoNonIpDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 5+basedesc->getFieldCount() : 5;
}

unsigned int FlowControlInfoNonIpDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<5) ? fieldTypeFlags[field] : 0;
}

const char *FlowControlInfoNonIpDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "priority",
        "duration",
        "srcAddr",
        "dstAddr",
        "creationTime",
    };
    return (field>=0 && field<5) ? fieldNames[field] : nullptr;
}

int FlowControlInfoNonIpDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='p' && strcmp(fieldName, "priority")==0) return base+0;
    if (fieldName[0]=='d' && strcmp(fieldName, "duration")==0) return base+1;
    if (fieldName[0]=='s' && strcmp(fieldName, "srcAddr")==0) return base+2;
    if (fieldName[0]=='d' && strcmp(fieldName, "dstAddr")==0) return base+3;
    if (fieldName[0]=='c' && strcmp(fieldName, "creationTime")==0) return base+4;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *FlowControlInfoNonIpDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "long",
        "long",
        "long",
        "simtime_t",
    };
    return (field>=0 && field<5) ? fieldTypeStrings[field] : nullptr;
}

const char **FlowControlInfoNonIpDescriptor::getFieldPropertyNames(int field) const
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

const char *FlowControlInfoNonIpDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int FlowControlInfoNonIpDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    FlowControlInfoNonIp *pp = (FlowControlInfoNonIp *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *FlowControlInfoNonIpDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    FlowControlInfoNonIp *pp = (FlowControlInfoNonIp *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string FlowControlInfoNonIpDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    FlowControlInfoNonIp *pp = (FlowControlInfoNonIp *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getPriority());
        case 1: return long2string(pp->getDuration());
        case 2: return long2string(pp->getSrcAddr());
        case 3: return long2string(pp->getDstAddr());
        case 4: return simtime2string(pp->getCreationTime());
        default: return "";
    }
}

bool FlowControlInfoNonIpDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    FlowControlInfoNonIp *pp = (FlowControlInfoNonIp *)object; (void)pp;
    switch (field) {
        case 0: pp->setPriority(string2long(value)); return true;
        case 1: pp->setDuration(string2long(value)); return true;
        case 2: pp->setSrcAddr(string2long(value)); return true;
        case 3: pp->setDstAddr(string2long(value)); return true;
        case 4: pp->setCreationTime(string2simtime(value)); return true;
        default: return false;
    }
}

const char *FlowControlInfoNonIpDescriptor::getFieldStructName(int field) const
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

void *FlowControlInfoNonIpDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    FlowControlInfoNonIp *pp = (FlowControlInfoNonIp *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

UserControlInfo_Base::UserControlInfo_Base() : ::LteControlInfo()
{
    this->acid = 0;
    this->cw = 0;
    this->txNumber = 0;
    this->ndi = true;
    this->isCorruptible = false;
    this->isBroadcast = false;
    this->deciderResult = false;
    this->blerTh = 0;
    this->blerValue = 0;
    this->txMode = 0;
    this->frameType = 0;
    this->txPower = 0;
    this->d2dTxPower = 0;
    this->totalGrantedBlocks = 0;
    this->subchannelNumber = 0;
    this->subchannelLength = 0;
    this->periodic = false;
    this->grantStartTime = 0;
}

UserControlInfo_Base::UserControlInfo_Base(const UserControlInfo_Base& other) : ::LteControlInfo(other)
{
    copy(other);
}

UserControlInfo_Base::~UserControlInfo_Base()
{
}

UserControlInfo_Base& UserControlInfo_Base::operator=(const UserControlInfo_Base& other)
{
    if (this==&other) return *this;
    ::LteControlInfo::operator=(other);
    copy(other);
    return *this;
}

void UserControlInfo_Base::copy(const UserControlInfo_Base& other)
{
    this->acid = other.acid;
    this->cw = other.cw;
    this->txNumber = other.txNumber;
    this->ndi = other.ndi;
    this->isCorruptible = other.isCorruptible;
    this->isBroadcast = other.isBroadcast;
    this->deciderResult = other.deciderResult;
    this->blerTh = other.blerTh;
    this->blerValue = other.blerValue;
    this->txMode = other.txMode;
    this->frameType = other.frameType;
    this->txPower = other.txPower;
    this->d2dTxPower = other.d2dTxPower;
    this->totalGrantedBlocks = other.totalGrantedBlocks;
    this->subchannelNumber = other.subchannelNumber;
    this->subchannelLength = other.subchannelLength;
    this->periodic = other.periodic;
    this->grantStartTime = other.grantStartTime;
}

void UserControlInfo_Base::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::LteControlInfo::parsimPack(b);
    doParsimPacking(b,this->acid);
    doParsimPacking(b,this->cw);
    doParsimPacking(b,this->txNumber);
    doParsimPacking(b,this->ndi);
    doParsimPacking(b,this->isCorruptible);
    doParsimPacking(b,this->isBroadcast);
    doParsimPacking(b,this->deciderResult);
    doParsimPacking(b,this->blerTh);
    doParsimPacking(b,this->blerValue);
    doParsimPacking(b,this->txMode);
    doParsimPacking(b,this->frameType);
    doParsimPacking(b,this->txPower);
    doParsimPacking(b,this->d2dTxPower);
    doParsimPacking(b,this->totalGrantedBlocks);
    doParsimPacking(b,this->subchannelNumber);
    doParsimPacking(b,this->subchannelLength);
    doParsimPacking(b,this->periodic);
    doParsimPacking(b,this->grantStartTime);
}

void UserControlInfo_Base::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::LteControlInfo::parsimUnpack(b);
    doParsimUnpacking(b,this->acid);
    doParsimUnpacking(b,this->cw);
    doParsimUnpacking(b,this->txNumber);
    doParsimUnpacking(b,this->ndi);
    doParsimUnpacking(b,this->isCorruptible);
    doParsimUnpacking(b,this->isBroadcast);
    doParsimUnpacking(b,this->deciderResult);
    doParsimUnpacking(b,this->blerTh);
    doParsimUnpacking(b,this->blerValue);
    doParsimUnpacking(b,this->txMode);
    doParsimUnpacking(b,this->frameType);
    doParsimUnpacking(b,this->txPower);
    doParsimUnpacking(b,this->d2dTxPower);
    doParsimUnpacking(b,this->totalGrantedBlocks);
    doParsimUnpacking(b,this->subchannelNumber);
    doParsimUnpacking(b,this->subchannelLength);
    doParsimUnpacking(b,this->periodic);
    doParsimUnpacking(b,this->grantStartTime);
}

unsigned char UserControlInfo_Base::getAcid() const
{
    return this->acid;
}

void UserControlInfo_Base::setAcid(unsigned char acid)
{
    this->acid = acid;
}

unsigned char UserControlInfo_Base::getCw() const
{
    return this->cw;
}

void UserControlInfo_Base::setCw(unsigned char cw)
{
    this->cw = cw;
}

unsigned char UserControlInfo_Base::getTxNumber() const
{
    return this->txNumber;
}

void UserControlInfo_Base::setTxNumber(unsigned char txNumber)
{
    this->txNumber = txNumber;
}

bool UserControlInfo_Base::getNdi() const
{
    return this->ndi;
}

void UserControlInfo_Base::setNdi(bool ndi)
{
    this->ndi = ndi;
}

bool UserControlInfo_Base::getIsCorruptible() const
{
    return this->isCorruptible;
}

void UserControlInfo_Base::setIsCorruptible(bool isCorruptible)
{
    this->isCorruptible = isCorruptible;
}

bool UserControlInfo_Base::getIsBroadcast() const
{
    return this->isBroadcast;
}

void UserControlInfo_Base::setIsBroadcast(bool isBroadcast)
{
    this->isBroadcast = isBroadcast;
}

bool UserControlInfo_Base::getDeciderResult() const
{
    return this->deciderResult;
}

void UserControlInfo_Base::setDeciderResult(bool deciderResult)
{
    this->deciderResult = deciderResult;
}

double UserControlInfo_Base::getBlerTh() const
{
    return this->blerTh;
}

void UserControlInfo_Base::setBlerTh(double blerTh)
{
    this->blerTh = blerTh;
}

double UserControlInfo_Base::getBlerValue() const
{
    return this->blerValue;
}

void UserControlInfo_Base::setBlerValue(double blerValue)
{
    this->blerValue = blerValue;
}

unsigned short UserControlInfo_Base::getTxMode() const
{
    return this->txMode;
}

void UserControlInfo_Base::setTxMode(unsigned short txMode)
{
    this->txMode = txMode;
}

unsigned int UserControlInfo_Base::getFrameType() const
{
    return this->frameType;
}

void UserControlInfo_Base::setFrameType(unsigned int frameType)
{
    this->frameType = frameType;
}

double UserControlInfo_Base::getTxPower() const
{
    return this->txPower;
}

void UserControlInfo_Base::setTxPower(double txPower)
{
    this->txPower = txPower;
}

double UserControlInfo_Base::getD2dTxPower() const
{
    return this->d2dTxPower;
}

void UserControlInfo_Base::setD2dTxPower(double d2dTxPower)
{
    this->d2dTxPower = d2dTxPower;
}

unsigned int UserControlInfo_Base::getTotalGrantedBlocks() const
{
    return this->totalGrantedBlocks;
}

void UserControlInfo_Base::setTotalGrantedBlocks(unsigned int totalGrantedBlocks)
{
    this->totalGrantedBlocks = totalGrantedBlocks;
}

int UserControlInfo_Base::getSubchannelNumber() const
{
    return this->subchannelNumber;
}

void UserControlInfo_Base::setSubchannelNumber(int subchannelNumber)
{
    this->subchannelNumber = subchannelNumber;
}

int UserControlInfo_Base::getSubchannelLength() const
{
    return this->subchannelLength;
}

void UserControlInfo_Base::setSubchannelLength(int subchannelLength)
{
    this->subchannelLength = subchannelLength;
}

bool UserControlInfo_Base::getPeriodic() const
{
    return this->periodic;
}

void UserControlInfo_Base::setPeriodic(bool periodic)
{
    this->periodic = periodic;
}

::omnetpp::simtime_t UserControlInfo_Base::getGrantStartTime() const
{
    return this->grantStartTime;
}

void UserControlInfo_Base::setGrantStartTime(::omnetpp::simtime_t grantStartTime)
{
    this->grantStartTime = grantStartTime;
}

class UserControlInfoDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    UserControlInfoDescriptor();
    virtual ~UserControlInfoDescriptor();

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

Register_ClassDescriptor(UserControlInfoDescriptor)

UserControlInfoDescriptor::UserControlInfoDescriptor() : omnetpp::cClassDescriptor("UserControlInfo", "LteControlInfo")
{
    propertynames = nullptr;
}

UserControlInfoDescriptor::~UserControlInfoDescriptor()
{
    delete[] propertynames;
}

bool UserControlInfoDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<UserControlInfo_Base *>(obj)!=nullptr;
}

const char **UserControlInfoDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = { "customize",  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *UserControlInfoDescriptor::getProperty(const char *propertyname) const
{
    if (!strcmp(propertyname,"customize")) return "true";
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int UserControlInfoDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 18+basedesc->getFieldCount() : 18;
}

unsigned int UserControlInfoDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<18) ? fieldTypeFlags[field] : 0;
}

const char *UserControlInfoDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "acid",
        "cw",
        "txNumber",
        "ndi",
        "isCorruptible",
        "isBroadcast",
        "deciderResult",
        "blerTh",
        "blerValue",
        "txMode",
        "frameType",
        "txPower",
        "d2dTxPower",
        "totalGrantedBlocks",
        "subchannelNumber",
        "subchannelLength",
        "periodic",
        "grantStartTime",
    };
    return (field>=0 && field<18) ? fieldNames[field] : nullptr;
}

int UserControlInfoDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='a' && strcmp(fieldName, "acid")==0) return base+0;
    if (fieldName[0]=='c' && strcmp(fieldName, "cw")==0) return base+1;
    if (fieldName[0]=='t' && strcmp(fieldName, "txNumber")==0) return base+2;
    if (fieldName[0]=='n' && strcmp(fieldName, "ndi")==0) return base+3;
    if (fieldName[0]=='i' && strcmp(fieldName, "isCorruptible")==0) return base+4;
    if (fieldName[0]=='i' && strcmp(fieldName, "isBroadcast")==0) return base+5;
    if (fieldName[0]=='d' && strcmp(fieldName, "deciderResult")==0) return base+6;
    if (fieldName[0]=='b' && strcmp(fieldName, "blerTh")==0) return base+7;
    if (fieldName[0]=='b' && strcmp(fieldName, "blerValue")==0) return base+8;
    if (fieldName[0]=='t' && strcmp(fieldName, "txMode")==0) return base+9;
    if (fieldName[0]=='f' && strcmp(fieldName, "frameType")==0) return base+10;
    if (fieldName[0]=='t' && strcmp(fieldName, "txPower")==0) return base+11;
    if (fieldName[0]=='d' && strcmp(fieldName, "d2dTxPower")==0) return base+12;
    if (fieldName[0]=='t' && strcmp(fieldName, "totalGrantedBlocks")==0) return base+13;
    if (fieldName[0]=='s' && strcmp(fieldName, "subchannelNumber")==0) return base+14;
    if (fieldName[0]=='s' && strcmp(fieldName, "subchannelLength")==0) return base+15;
    if (fieldName[0]=='p' && strcmp(fieldName, "periodic")==0) return base+16;
    if (fieldName[0]=='g' && strcmp(fieldName, "grantStartTime")==0) return base+17;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *UserControlInfoDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "unsigned char",
        "unsigned char",
        "unsigned char",
        "bool",
        "bool",
        "bool",
        "bool",
        "double",
        "double",
        "unsigned short",
        "unsigned int",
        "double",
        "double",
        "unsigned int",
        "int",
        "int",
        "bool",
        "simtime_t",
    };
    return (field>=0 && field<18) ? fieldTypeStrings[field] : nullptr;
}

const char **UserControlInfoDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case 9: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        case 10: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *UserControlInfoDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case 9:
            if (!strcmp(propertyname,"enum")) return "TxMode";
            return nullptr;
        case 10:
            if (!strcmp(propertyname,"enum")) return "LtePhyFrameType";
            return nullptr;
        default: return nullptr;
    }
}

int UserControlInfoDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    UserControlInfo_Base *pp = (UserControlInfo_Base *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *UserControlInfoDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    UserControlInfo_Base *pp = (UserControlInfo_Base *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string UserControlInfoDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    UserControlInfo_Base *pp = (UserControlInfo_Base *)object; (void)pp;
    switch (field) {
        case 0: return ulong2string(pp->getAcid());
        case 1: return ulong2string(pp->getCw());
        case 2: return ulong2string(pp->getTxNumber());
        case 3: return bool2string(pp->getNdi());
        case 4: return bool2string(pp->getIsCorruptible());
        case 5: return bool2string(pp->getIsBroadcast());
        case 6: return bool2string(pp->getDeciderResult());
        case 7: return double2string(pp->getBlerTh());
        case 8: return double2string(pp->getBlerValue());
        case 9: return enum2string(pp->getTxMode(), "TxMode");
        case 10: return enum2string(pp->getFrameType(), "LtePhyFrameType");
        case 11: return double2string(pp->getTxPower());
        case 12: return double2string(pp->getD2dTxPower());
        case 13: return ulong2string(pp->getTotalGrantedBlocks());
        case 14: return long2string(pp->getSubchannelNumber());
        case 15: return long2string(pp->getSubchannelLength());
        case 16: return bool2string(pp->getPeriodic());
        case 17: return simtime2string(pp->getGrantStartTime());
        default: return "";
    }
}

bool UserControlInfoDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    UserControlInfo_Base *pp = (UserControlInfo_Base *)object; (void)pp;
    switch (field) {
        case 0: pp->setAcid(string2ulong(value)); return true;
        case 1: pp->setCw(string2ulong(value)); return true;
        case 2: pp->setTxNumber(string2ulong(value)); return true;
        case 3: pp->setNdi(string2bool(value)); return true;
        case 4: pp->setIsCorruptible(string2bool(value)); return true;
        case 5: pp->setIsBroadcast(string2bool(value)); return true;
        case 6: pp->setDeciderResult(string2bool(value)); return true;
        case 7: pp->setBlerTh(string2double(value)); return true;
        case 8: pp->setBlerValue(string2double(value)); return true;
        case 9: pp->setTxMode((TxMode)string2enum(value, "TxMode")); return true;
        case 10: pp->setFrameType((LtePhyFrameType)string2enum(value, "LtePhyFrameType")); return true;
        case 11: pp->setTxPower(string2double(value)); return true;
        case 12: pp->setD2dTxPower(string2double(value)); return true;
        case 13: pp->setTotalGrantedBlocks(string2ulong(value)); return true;
        case 14: pp->setSubchannelNumber(string2long(value)); return true;
        case 15: pp->setSubchannelLength(string2long(value)); return true;
        case 16: pp->setPeriodic(string2bool(value)); return true;
        case 17: pp->setGrantStartTime(string2simtime(value)); return true;
        default: return false;
    }
}

const char *UserControlInfoDescriptor::getFieldStructName(int field) const
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

void *UserControlInfoDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    UserControlInfo_Base *pp = (UserControlInfo_Base *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}


