//
// Generated file, do not edit! Created by nedtool 5.7 from stack/d2dModeSelection/D2DModeSwitchNotification.msg.
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
#include "D2DModeSwitchNotification_m.h"

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

Register_Class(D2DModeSwitchNotification)

D2DModeSwitchNotification::D2DModeSwitchNotification(const char *name, short kind) : ::omnetpp::cPacket(name,kind)
{
    this->peerId = 0;
    this->txSide = false;
    this->oldConnection = false;
}

D2DModeSwitchNotification::D2DModeSwitchNotification(const D2DModeSwitchNotification& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

D2DModeSwitchNotification::~D2DModeSwitchNotification()
{
}

D2DModeSwitchNotification& D2DModeSwitchNotification::operator=(const D2DModeSwitchNotification& other)
{
    if (this==&other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void D2DModeSwitchNotification::copy(const D2DModeSwitchNotification& other)
{
    this->peerId = other.peerId;
    this->txSide = other.txSide;
    this->oldMode = other.oldMode;
    this->newMode = other.newMode;
    this->oldConnection = other.oldConnection;
}

void D2DModeSwitchNotification::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->peerId);
    doParsimPacking(b,this->txSide);
    doParsimPacking(b,this->oldMode);
    doParsimPacking(b,this->newMode);
    doParsimPacking(b,this->oldConnection);
}

void D2DModeSwitchNotification::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->peerId);
    doParsimUnpacking(b,this->txSide);
    doParsimUnpacking(b,this->oldMode);
    doParsimUnpacking(b,this->newMode);
    doParsimUnpacking(b,this->oldConnection);
}

unsigned int D2DModeSwitchNotification::getPeerId() const
{
    return this->peerId;
}

void D2DModeSwitchNotification::setPeerId(unsigned int peerId)
{
    this->peerId = peerId;
}

bool D2DModeSwitchNotification::getTxSide() const
{
    return this->txSide;
}

void D2DModeSwitchNotification::setTxSide(bool txSide)
{
    this->txSide = txSide;
}

LteD2DMode& D2DModeSwitchNotification::getOldMode()
{
    return this->oldMode;
}

void D2DModeSwitchNotification::setOldMode(const LteD2DMode& oldMode)
{
    this->oldMode = oldMode;
}

LteD2DMode& D2DModeSwitchNotification::getNewMode()
{
    return this->newMode;
}

void D2DModeSwitchNotification::setNewMode(const LteD2DMode& newMode)
{
    this->newMode = newMode;
}

bool D2DModeSwitchNotification::getOldConnection() const
{
    return this->oldConnection;
}

void D2DModeSwitchNotification::setOldConnection(bool oldConnection)
{
    this->oldConnection = oldConnection;
}

class D2DModeSwitchNotificationDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    D2DModeSwitchNotificationDescriptor();
    virtual ~D2DModeSwitchNotificationDescriptor();

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

Register_ClassDescriptor(D2DModeSwitchNotificationDescriptor)

D2DModeSwitchNotificationDescriptor::D2DModeSwitchNotificationDescriptor() : omnetpp::cClassDescriptor("D2DModeSwitchNotification", "omnetpp::cPacket")
{
    propertynames = nullptr;
}

D2DModeSwitchNotificationDescriptor::~D2DModeSwitchNotificationDescriptor()
{
    delete[] propertynames;
}

bool D2DModeSwitchNotificationDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<D2DModeSwitchNotification *>(obj)!=nullptr;
}

const char **D2DModeSwitchNotificationDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *D2DModeSwitchNotificationDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int D2DModeSwitchNotificationDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 5+basedesc->getFieldCount() : 5;
}

unsigned int D2DModeSwitchNotificationDescriptor::getFieldTypeFlags(int field) const
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
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<5) ? fieldTypeFlags[field] : 0;
}

const char *D2DModeSwitchNotificationDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "peerId",
        "txSide",
        "oldMode",
        "newMode",
        "oldConnection",
    };
    return (field>=0 && field<5) ? fieldNames[field] : nullptr;
}

int D2DModeSwitchNotificationDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='p' && strcmp(fieldName, "peerId")==0) return base+0;
    if (fieldName[0]=='t' && strcmp(fieldName, "txSide")==0) return base+1;
    if (fieldName[0]=='o' && strcmp(fieldName, "oldMode")==0) return base+2;
    if (fieldName[0]=='n' && strcmp(fieldName, "newMode")==0) return base+3;
    if (fieldName[0]=='o' && strcmp(fieldName, "oldConnection")==0) return base+4;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *D2DModeSwitchNotificationDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "unsigned int",
        "bool",
        "LteD2DMode",
        "LteD2DMode",
        "bool",
    };
    return (field>=0 && field<5) ? fieldTypeStrings[field] : nullptr;
}

const char **D2DModeSwitchNotificationDescriptor::getFieldPropertyNames(int field) const
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

const char *D2DModeSwitchNotificationDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int D2DModeSwitchNotificationDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    D2DModeSwitchNotification *pp = (D2DModeSwitchNotification *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *D2DModeSwitchNotificationDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    D2DModeSwitchNotification *pp = (D2DModeSwitchNotification *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string D2DModeSwitchNotificationDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    D2DModeSwitchNotification *pp = (D2DModeSwitchNotification *)object; (void)pp;
    switch (field) {
        case 0: return ulong2string(pp->getPeerId());
        case 1: return bool2string(pp->getTxSide());
        case 2: {std::stringstream out; out << pp->getOldMode(); return out.str();}
        case 3: {std::stringstream out; out << pp->getNewMode(); return out.str();}
        case 4: return bool2string(pp->getOldConnection());
        default: return "";
    }
}

bool D2DModeSwitchNotificationDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    D2DModeSwitchNotification *pp = (D2DModeSwitchNotification *)object; (void)pp;
    switch (field) {
        case 0: pp->setPeerId(string2ulong(value)); return true;
        case 1: pp->setTxSide(string2bool(value)); return true;
        case 4: pp->setOldConnection(string2bool(value)); return true;
        default: return false;
    }
}

const char *D2DModeSwitchNotificationDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case 2: return omnetpp::opp_typename(typeid(LteD2DMode));
        case 3: return omnetpp::opp_typename(typeid(LteD2DMode));
        default: return nullptr;
    };
}

void *D2DModeSwitchNotificationDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    D2DModeSwitchNotification *pp = (D2DModeSwitchNotification *)object; (void)pp;
    switch (field) {
        case 2: return (void *)(&pp->getOldMode()); break;
        case 3: return (void *)(&pp->getNewMode()); break;
        default: return nullptr;
    }
}


