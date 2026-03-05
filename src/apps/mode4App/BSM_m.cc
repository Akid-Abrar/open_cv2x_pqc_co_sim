//
// Generated file, do not edit! Created by nedtool 5.7 from veins/pqcdsa/BSM.msg.
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
#include "BSM_m.h"

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

Register_Class(BSM)

BSM::BSM(const char *name, short kind) : ::omnetpp::cMessage(name, kind)
{
}

BSM::BSM(const BSM& other) : ::omnetpp::cMessage(other)
{
    copy(other);
}

BSM::~BSM()
{
}

BSM& BSM::operator=(const BSM& other)
{
    if (this == &other) return *this;
    ::omnetpp::cMessage::operator=(other);
    copy(other);
    return *this;
}

void BSM::copy(const BSM& other)
{
    this->msgCnt = other.msgCnt;
    this->msgId = other.msgId;
    this->tempId = other.tempId;
    this->secMark = other.secMark;
    this->lat = other.lat;
    this->lon = other.lon;
    this->elev = other.elev;
    this->semiMajor = other.semiMajor;
    this->semiMinor = other.semiMinor;
    this->semiMajorOrient = other.semiMajorOrient;
    this->transmission = other.transmission;
    this->speed_j = other.speed_j;
    this->heading_j = other.heading_j;
    this->angle = other.angle;
    this->accelLong = other.accelLong;
    this->accelLat = other.accelLat;
    this->accelVert = other.accelVert;
    this->yawRate = other.yawRate;
    this->brakes = other.brakes;
    this->vehWidth = other.vehWidth;
    this->vehLength = other.vehLength;
}

void BSM::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cMessage::parsimPack(b);
    doParsimPacking(b,this->msgCnt);
    doParsimPacking(b,this->msgId);
    doParsimPacking(b,this->tempId);
    doParsimPacking(b,this->secMark);
    doParsimPacking(b,this->lat);
    doParsimPacking(b,this->lon);
    doParsimPacking(b,this->elev);
    doParsimPacking(b,this->semiMajor);
    doParsimPacking(b,this->semiMinor);
    doParsimPacking(b,this->semiMajorOrient);
    doParsimPacking(b,this->transmission);
    doParsimPacking(b,this->speed_j);
    doParsimPacking(b,this->heading_j);
    doParsimPacking(b,this->angle);
    doParsimPacking(b,this->accelLong);
    doParsimPacking(b,this->accelLat);
    doParsimPacking(b,this->accelVert);
    doParsimPacking(b,this->yawRate);
    doParsimPacking(b,this->brakes);
    doParsimPacking(b,this->vehWidth);
    doParsimPacking(b,this->vehLength);
}

void BSM::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->msgCnt);
    doParsimUnpacking(b,this->msgId);
    doParsimUnpacking(b,this->tempId);
    doParsimUnpacking(b,this->secMark);
    doParsimUnpacking(b,this->lat);
    doParsimUnpacking(b,this->lon);
    doParsimUnpacking(b,this->elev);
    doParsimUnpacking(b,this->semiMajor);
    doParsimUnpacking(b,this->semiMinor);
    doParsimUnpacking(b,this->semiMajorOrient);
    doParsimUnpacking(b,this->transmission);
    doParsimUnpacking(b,this->speed_j);
    doParsimUnpacking(b,this->heading_j);
    doParsimUnpacking(b,this->angle);
    doParsimUnpacking(b,this->accelLong);
    doParsimUnpacking(b,this->accelLat);
    doParsimUnpacking(b,this->accelVert);
    doParsimUnpacking(b,this->yawRate);
    doParsimUnpacking(b,this->brakes);
    doParsimUnpacking(b,this->vehWidth);
    doParsimUnpacking(b,this->vehLength);
}

uint8_t BSM::getMsgCnt() const
{
    return this->msgCnt;
}

void BSM::setMsgCnt(uint8_t msgCnt)
{
    this->msgCnt = msgCnt;
}

int32_t BSM::getMsgId() const
{
    return this->msgId;
}

void BSM::setMsgId(int32_t msgId)
{
    this->msgId = msgId;
}

const char * BSM::getTempId() const
{
    return this->tempId.c_str();
}

void BSM::setTempId(const char * tempId)
{
    this->tempId = tempId;
}

uint16_t BSM::getSecMark() const
{
    return this->secMark;
}

void BSM::setSecMark(uint16_t secMark)
{
    this->secMark = secMark;
}

int32_t BSM::getLat() const
{
    return this->lat;
}

void BSM::setLat(int32_t lat)
{
    this->lat = lat;
}

int32_t BSM::getLon() const
{
    return this->lon;
}

void BSM::setLon(int32_t lon)
{
    this->lon = lon;
}

int16_t BSM::getElev() const
{
    return this->elev;
}

void BSM::setElev(int16_t elev)
{
    this->elev = elev;
}

uint8_t BSM::getSemiMajor() const
{
    return this->semiMajor;
}

void BSM::setSemiMajor(uint8_t semiMajor)
{
    this->semiMajor = semiMajor;
}

uint8_t BSM::getSemiMinor() const
{
    return this->semiMinor;
}

void BSM::setSemiMinor(uint8_t semiMinor)
{
    this->semiMinor = semiMinor;
}

uint16_t BSM::getSemiMajorOrient() const
{
    return this->semiMajorOrient;
}

void BSM::setSemiMajorOrient(uint16_t semiMajorOrient)
{
    this->semiMajorOrient = semiMajorOrient;
}

uint8_t BSM::getTransmission() const
{
    return this->transmission;
}

void BSM::setTransmission(uint8_t transmission)
{
    this->transmission = transmission;
}

uint16_t BSM::getSpeed_j() const
{
    return this->speed_j;
}

void BSM::setSpeed_j(uint16_t speed_j)
{
    this->speed_j = speed_j;
}

uint16_t BSM::getHeading_j() const
{
    return this->heading_j;
}

void BSM::setHeading_j(uint16_t heading_j)
{
    this->heading_j = heading_j;
}

int8_t BSM::getAngle() const
{
    return this->angle;
}

void BSM::setAngle(int8_t angle)
{
    this->angle = angle;
}

int16_t BSM::getAccelLong() const
{
    return this->accelLong;
}

void BSM::setAccelLong(int16_t accelLong)
{
    this->accelLong = accelLong;
}

int16_t BSM::getAccelLat() const
{
    return this->accelLat;
}

void BSM::setAccelLat(int16_t accelLat)
{
    this->accelLat = accelLat;
}

int8_t BSM::getAccelVert() const
{
    return this->accelVert;
}

void BSM::setAccelVert(int8_t accelVert)
{
    this->accelVert = accelVert;
}

int16_t BSM::getYawRate() const
{
    return this->yawRate;
}

void BSM::setYawRate(int16_t yawRate)
{
    this->yawRate = yawRate;
}

uint16_t BSM::getBrakes() const
{
    return this->brakes;
}

void BSM::setBrakes(uint16_t brakes)
{
    this->brakes = brakes;
}

uint16_t BSM::getVehWidth() const
{
    return this->vehWidth;
}

void BSM::setVehWidth(uint16_t vehWidth)
{
    this->vehWidth = vehWidth;
}

uint8_t BSM::getVehLength() const
{
    return this->vehLength;
}

void BSM::setVehLength(uint8_t vehLength)
{
    this->vehLength = vehLength;
}

class BSMDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_msgCnt,
        FIELD_msgId,
        FIELD_tempId,
        FIELD_secMark,
        FIELD_lat,
        FIELD_lon,
        FIELD_elev,
        FIELD_semiMajor,
        FIELD_semiMinor,
        FIELD_semiMajorOrient,
        FIELD_transmission,
        FIELD_speed_j,
        FIELD_heading_j,
        FIELD_angle,
        FIELD_accelLong,
        FIELD_accelLat,
        FIELD_accelVert,
        FIELD_yawRate,
        FIELD_brakes,
        FIELD_vehWidth,
        FIELD_vehLength,
    };
  public:
    BSMDescriptor();
    virtual ~BSMDescriptor();

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

Register_ClassDescriptor(BSMDescriptor)

BSMDescriptor::BSMDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(BSM)), "omnetpp::cMessage")
{
    propertynames = nullptr;
}

BSMDescriptor::~BSMDescriptor()
{
    delete[] propertynames;
}

bool BSMDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<BSM *>(obj)!=nullptr;
}

const char **BSMDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *BSMDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int BSMDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 21+basedesc->getFieldCount() : 21;
}

unsigned int BSMDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_msgCnt
        FD_ISEDITABLE,    // FIELD_msgId
        FD_ISEDITABLE,    // FIELD_tempId
        FD_ISEDITABLE,    // FIELD_secMark
        FD_ISEDITABLE,    // FIELD_lat
        FD_ISEDITABLE,    // FIELD_lon
        FD_ISEDITABLE,    // FIELD_elev
        FD_ISEDITABLE,    // FIELD_semiMajor
        FD_ISEDITABLE,    // FIELD_semiMinor
        FD_ISEDITABLE,    // FIELD_semiMajorOrient
        FD_ISEDITABLE,    // FIELD_transmission
        FD_ISEDITABLE,    // FIELD_speed_j
        FD_ISEDITABLE,    // FIELD_heading_j
        FD_ISEDITABLE,    // FIELD_angle
        FD_ISEDITABLE,    // FIELD_accelLong
        FD_ISEDITABLE,    // FIELD_accelLat
        FD_ISEDITABLE,    // FIELD_accelVert
        FD_ISEDITABLE,    // FIELD_yawRate
        FD_ISEDITABLE,    // FIELD_brakes
        FD_ISEDITABLE,    // FIELD_vehWidth
        FD_ISEDITABLE,    // FIELD_vehLength
    };
    return (field >= 0 && field < 21) ? fieldTypeFlags[field] : 0;
}

const char *BSMDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "msgCnt",
        "msgId",
        "tempId",
        "secMark",
        "lat",
        "lon",
        "elev",
        "semiMajor",
        "semiMinor",
        "semiMajorOrient",
        "transmission",
        "speed_j",
        "heading_j",
        "angle",
        "accelLong",
        "accelLat",
        "accelVert",
        "yawRate",
        "brakes",
        "vehWidth",
        "vehLength",
    };
    return (field >= 0 && field < 21) ? fieldNames[field] : nullptr;
}

int BSMDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'm' && strcmp(fieldName, "msgCnt") == 0) return base+0;
    if (fieldName[0] == 'm' && strcmp(fieldName, "msgId") == 0) return base+1;
    if (fieldName[0] == 't' && strcmp(fieldName, "tempId") == 0) return base+2;
    if (fieldName[0] == 's' && strcmp(fieldName, "secMark") == 0) return base+3;
    if (fieldName[0] == 'l' && strcmp(fieldName, "lat") == 0) return base+4;
    if (fieldName[0] == 'l' && strcmp(fieldName, "lon") == 0) return base+5;
    if (fieldName[0] == 'e' && strcmp(fieldName, "elev") == 0) return base+6;
    if (fieldName[0] == 's' && strcmp(fieldName, "semiMajor") == 0) return base+7;
    if (fieldName[0] == 's' && strcmp(fieldName, "semiMinor") == 0) return base+8;
    if (fieldName[0] == 's' && strcmp(fieldName, "semiMajorOrient") == 0) return base+9;
    if (fieldName[0] == 't' && strcmp(fieldName, "transmission") == 0) return base+10;
    if (fieldName[0] == 's' && strcmp(fieldName, "speed_j") == 0) return base+11;
    if (fieldName[0] == 'h' && strcmp(fieldName, "heading_j") == 0) return base+12;
    if (fieldName[0] == 'a' && strcmp(fieldName, "angle") == 0) return base+13;
    if (fieldName[0] == 'a' && strcmp(fieldName, "accelLong") == 0) return base+14;
    if (fieldName[0] == 'a' && strcmp(fieldName, "accelLat") == 0) return base+15;
    if (fieldName[0] == 'a' && strcmp(fieldName, "accelVert") == 0) return base+16;
    if (fieldName[0] == 'y' && strcmp(fieldName, "yawRate") == 0) return base+17;
    if (fieldName[0] == 'b' && strcmp(fieldName, "brakes") == 0) return base+18;
    if (fieldName[0] == 'v' && strcmp(fieldName, "vehWidth") == 0) return base+19;
    if (fieldName[0] == 'v' && strcmp(fieldName, "vehLength") == 0) return base+20;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *BSMDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "uint8_t",    // FIELD_msgCnt
        "int32_t",    // FIELD_msgId
        "string",    // FIELD_tempId
        "uint16_t",    // FIELD_secMark
        "int32_t",    // FIELD_lat
        "int32_t",    // FIELD_lon
        "int16_t",    // FIELD_elev
        "uint8_t",    // FIELD_semiMajor
        "uint8_t",    // FIELD_semiMinor
        "uint16_t",    // FIELD_semiMajorOrient
        "uint8_t",    // FIELD_transmission
        "uint16_t",    // FIELD_speed_j
        "uint16_t",    // FIELD_heading_j
        "int8_t",    // FIELD_angle
        "int16_t",    // FIELD_accelLong
        "int16_t",    // FIELD_accelLat
        "int8_t",    // FIELD_accelVert
        "int16_t",    // FIELD_yawRate
        "uint16_t",    // FIELD_brakes
        "uint16_t",    // FIELD_vehWidth
        "uint8_t",    // FIELD_vehLength
    };
    return (field >= 0 && field < 21) ? fieldTypeStrings[field] : nullptr;
}

const char **BSMDescriptor::getFieldPropertyNames(int field) const
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

const char *BSMDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int BSMDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    BSM *pp = (BSM *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *BSMDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    BSM *pp = (BSM *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string BSMDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    BSM *pp = (BSM *)object; (void)pp;
    switch (field) {
        case FIELD_msgCnt: return ulong2string(pp->getMsgCnt());
        case FIELD_msgId: return long2string(pp->getMsgId());
        case FIELD_tempId: return oppstring2string(pp->getTempId());
        case FIELD_secMark: return ulong2string(pp->getSecMark());
        case FIELD_lat: return long2string(pp->getLat());
        case FIELD_lon: return long2string(pp->getLon());
        case FIELD_elev: return long2string(pp->getElev());
        case FIELD_semiMajor: return ulong2string(pp->getSemiMajor());
        case FIELD_semiMinor: return ulong2string(pp->getSemiMinor());
        case FIELD_semiMajorOrient: return ulong2string(pp->getSemiMajorOrient());
        case FIELD_transmission: return ulong2string(pp->getTransmission());
        case FIELD_speed_j: return ulong2string(pp->getSpeed_j());
        case FIELD_heading_j: return ulong2string(pp->getHeading_j());
        case FIELD_angle: return long2string(pp->getAngle());
        case FIELD_accelLong: return long2string(pp->getAccelLong());
        case FIELD_accelLat: return long2string(pp->getAccelLat());
        case FIELD_accelVert: return long2string(pp->getAccelVert());
        case FIELD_yawRate: return long2string(pp->getYawRate());
        case FIELD_brakes: return ulong2string(pp->getBrakes());
        case FIELD_vehWidth: return ulong2string(pp->getVehWidth());
        case FIELD_vehLength: return ulong2string(pp->getVehLength());
        default: return "";
    }
}

bool BSMDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    BSM *pp = (BSM *)object; (void)pp;
    switch (field) {
        case FIELD_msgCnt: pp->setMsgCnt(string2ulong(value)); return true;
        case FIELD_msgId: pp->setMsgId(string2long(value)); return true;
        case FIELD_tempId: pp->setTempId((value)); return true;
        case FIELD_secMark: pp->setSecMark(string2ulong(value)); return true;
        case FIELD_lat: pp->setLat(string2long(value)); return true;
        case FIELD_lon: pp->setLon(string2long(value)); return true;
        case FIELD_elev: pp->setElev(string2long(value)); return true;
        case FIELD_semiMajor: pp->setSemiMajor(string2ulong(value)); return true;
        case FIELD_semiMinor: pp->setSemiMinor(string2ulong(value)); return true;
        case FIELD_semiMajorOrient: pp->setSemiMajorOrient(string2ulong(value)); return true;
        case FIELD_transmission: pp->setTransmission(string2ulong(value)); return true;
        case FIELD_speed_j: pp->setSpeed_j(string2ulong(value)); return true;
        case FIELD_heading_j: pp->setHeading_j(string2ulong(value)); return true;
        case FIELD_angle: pp->setAngle(string2long(value)); return true;
        case FIELD_accelLong: pp->setAccelLong(string2long(value)); return true;
        case FIELD_accelLat: pp->setAccelLat(string2long(value)); return true;
        case FIELD_accelVert: pp->setAccelVert(string2long(value)); return true;
        case FIELD_yawRate: pp->setYawRate(string2long(value)); return true;
        case FIELD_brakes: pp->setBrakes(string2ulong(value)); return true;
        case FIELD_vehWidth: pp->setVehWidth(string2ulong(value)); return true;
        case FIELD_vehLength: pp->setVehLength(string2ulong(value)); return true;
        default: return false;
    }
}

const char *BSMDescriptor::getFieldStructName(int field) const
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

void *BSMDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    BSM *pp = (BSM *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

