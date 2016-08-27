#ifndef IARCHIVE
#define IARCHIVE

#include "CSerialVersion.h"
#include "Common/AssertMacro.h"
#include "Common/CAssetID.h"
#include "Common/CFourCC.h"
#include "Common/EGame.h"
#include "Common/TString.h"
#include "Common/types.h"

/* This is a custom serialization implementation intended for saving game assets out to editor-
 * friendly formats, such as XML. The main goals of the serialization system is to simplify the
 * code for reading and writing editor files and to be able to easily update those files without
 * breaking compatibility with older versions. Support for new output formats can be added by
 * implementing new subclasses of IArchive.
 *
 * To use a class with the serialization system, it must have a Serialize function implemented.
 * There are two ways this function can be defined:
 * 1. As a member function with this signature: void Serialize(IArchive&)
 * 2. As a global/friend function with this signature: void Serialize(IArchive&, YourClass&)
 *
 * Use the << operator to serialize data members to the archive. All data being serialized must be
 * wrapped in one of the SERIAL macros. The SERIAL macro ensures that all serialized parameters are
 * associated with a name. This name makes the output file more easily human-readable as well as helps
 * ensure that files are easily backwards-compatible if parameters are moved or added/removed.
 *
 * Polymorphism is supported. There are two requirements for a polymorphic class to work with the
 * serialization system. First, the base class must contain a virtual Type() function that returns
 * an integral value (an enum or an integer), as well as a virtual Serialize(IArchive&) function.
 * Second, there must be a factory object with a SpawnObject(u32) method that takes the same Type value
 * and returns an object of the correct class.
 *
 * Containers are also supported. Containers require a different macro that allows you to specify the
 * name of the elements in the container. The currently-supported containers are std::vector, std::list,
 * and std::set. Support for more container types can be added to the bottom of this file.
 *
 * These are the available SERIAL macros:
 * - SERIAL(ParamName, ParamValue)                                          [generic parameter]
 * - SERIAL_HEX(ParamName, ParamValue)                                      [integral parameter serialized as a hex number for improved readability]
 * - SERIAL_ABSTRACT(ParamName, ParamValue, Factory)                        [polymorphic parameter]
 * - SERIAL_CONTAINER(ParamName, Container, ElementName)                    [container parameter]
 * - SERIAL_ABSTRACT_CONTAINER(ParamName, Container, ElementName, Factory)  [container of polymorphic objects parameter]
 *
 * Each of these has a variant with _AUTO at the end that allows you to exclude ParamName (the name of the
 * variable will be used as the parameter name instead).
 */

// TSerialParameter - name/value pair for generic serial parameters
template<typename ValType>
struct TSerialParameter
{
    const char *pkName;
    ValType& rValue;
};

template<typename ValType>
inline TSerialParameter<ValType> MakeSerialParameter(const char *pkName, ValType& rValue)
{
    return TSerialParameter<ValType> { pkName, rValue };
}

#define SERIAL(ParamName, ParamValue) MakeSerialParameter(ParamName, ParamValue)
#define SERIAL_AUTO(ParamValue) MakeSerialParameter(#ParamValue, ParamValue)

// THexSerialParameter - same as TSerialParameter but serialized in hex for improved readability
template<typename ValType>
struct THexSerialParameter
{
    const char *pkName;
    ValType& rValue;
};

template<typename ValType/*, class = typename std::enable_if<std::is_integral<ValType>::value, int>*/>
inline THexSerialParameter<ValType> MakeHexSerialParameter(const char *pkName, ValType& rValue)
{
    return THexSerialParameter<ValType> { pkName, rValue };
}

#define SERIAL_HEX(ParamName, ParamValue) MakeHexSerialParameter(ParamName, ParamValue)
#define SERIAL_HEX_AUTO(ParamValue) MakeHexSerialParameter(#ParamValue, ParamValue)

// TAbstractSerialParameter - name/value pair for polymorphic objects a pointer to a factory
template<typename ValType, typename FactoryType>
struct TAbstractSerialParameter
{
    const char *pkName;
    ValType*& rValue;
    FactoryType *pFactory;
};

template<typename ValType, typename FactoryType>
inline TAbstractSerialParameter<ValType, FactoryType> MakeAbstractSerialParameter(const char *pkName, ValType*& rValue, FactoryType *pFactory)
{
    return TAbstractSerialParameter<ValType, FactoryType> { pkName, rValue, pFactory };
}

#define SERIAL_ABSTRACT(ParamName, ParamValue, Factory) MakeAbstractSerialParameter(ParamName, ParamValue, Factory)
#define SERIAL_ABSTRACT_AUTO(ParamValue, Factory) MakeAbstractSerialParameter(#ParamValue, ParamValue, Factory)

// TContainerSerialParameter - name/value pair for containers with a parameter name string
template<typename ValType>
struct TContainerSerialParameter
{
    const char *pkName;
    ValType& rContainer;
    const char *pkElementName;
};

template<typename ValType>
inline TContainerSerialParameter<ValType> MakeSerialParameter(const char *pkName, ValType& rContainer, const char *pkElementName)
{
    return TContainerSerialParameter<ValType> { pkName, rContainer, pkElementName };
}

#define SERIAL_CONTAINER(ParamName, Container, ElementName) MakeSerialParameter(ParamName, Container, ElementName)
#define SERIAL_CONTAINER_AUTO(Container, ElementName) MakeSerialParameter(#Container, Container, ElementName)

// TAbstractContainerSerialParameter - name/value pair for containers of polymorphic objects with a parameter name string and a pointer to a factory
template<typename ValType, typename FactoryType>
struct TAbstractContainerSerialParameter
{
    const char *pkName;
    ValType& rContainer;
    const char *pkElementName;
    FactoryType *pFactory;
};

template<typename ValType, typename FactoryType>
inline TAbstractContainerSerialParameter<ValType, FactoryType> MakeSerialParameter(const char *pkName, ValType& rContainer, const char *pkElementName, FactoryType *pFactory)
{
    return TAbstractContainerSerialParameter<ValType, FactoryType> { pkName, rContainer, pkElementName, pFactory };
}

#define SERIAL_ABSTRACT_CONTAINER(ParamName, Container, ElementName, Factory) MakeSerialParameter(ParamName, Container, ElementName, Factory)
#define SERIAL_ABSTRACT_CONTAINER_AUTO(Container, ElementName, Factory) MakeSerialParameter(#Container, Container, ElementName, Factory)

// SFINAE serialize function type check
// https://jguegant.github.io/blogs/tech/sfinae-introduction.html
void Serialize(); // This needs to be here or else the global Serialize method handling causes a compiler error. Not sure of a better fix
void SerializeContainer();

template<class ValueType, class ArchiveType>
struct SerialType
{
public:
    enum { Primitive, Member, Global, None };

    // Helper struct to verify that function Func exists and matches the given function signature (FuncType)
    template<typename FuncType, FuncType Func> struct FunctionExists;

    // Check for ArcType::SerializePrimitive(ValType&) method
    template<typename ValType, typename ArcType> static long long&  Check(FunctionExists<void (ArcType::*)(ValType&), &ArcType::SerializePrimitive>*);
    // Check for ValType::Serialize(ArcType&)
    template<typename ValType, typename ArcType> static long&       Check(FunctionExists<void (ValType::*)(ArcType&), &ValType::Serialize>*);
    // Check for global Serialize(ArcType&,ValType&) function
    template<typename ValType, typename ArcType> static short&      Check(FunctionExists<void (*)(ArcType&, ValType&), &Serialize>*);
    // Check for global SerializeContainer(ArcType&,ValType&,const TString&)
    template<typename ValType, typename ArcType> static short&      Check(FunctionExists<void (*)(ArcType&, ValType&, const TString&), &SerializeContainer>*);
    // Fallback - no valid Serialize method exists
    template<typename ValType, typename ArcType> static char&       Check(...);

    // Set Type enum to correspond to whichever function exists
    static const int Type = (sizeof(Check<ValueType, ArchiveType>(0)) == sizeof(long long) ? Primitive :
                             sizeof(Check<ValueType, ArchiveType>(0)) == sizeof(long) ? Member :
                             sizeof(Check<ValueType, ArchiveType>(0)) == sizeof(short) ? Global :
                             None);
};

// Actual archive class
class IArchive
{
protected:
    u16 mArchiveVersion;
    u16 mFileVersion;
    EGame mGame;
    bool mIsReader;
    bool mIsWriter;

public:
    static const u32 skCurrentArchiveVersion = 0;

    IArchive(bool IsReader, bool IsWriter)
        : mFileVersion(0)
        , mArchiveVersion(skCurrentArchiveVersion)
        , mGame(eUnknownGame)
        , mIsReader(IsReader)
        , mIsWriter(IsWriter)
    {}

    virtual ~IArchive() {}

    #define ENABLE_FOR_SERIAL_TYPE(SType) typename std::enable_if<SerialType<ValType, IArchive>::Type == SerialType<ValType, IArchive>::##SType, int>::type = 0

    // Serialize primitives
    template<typename ValType, ENABLE_FOR_SERIAL_TYPE(Primitive)>
    inline IArchive& operator<<(TSerialParameter<ValType>& rParam)
    {
        if (ParamBegin(rParam.pkName))
        {
            SerializePrimitive(rParam.rValue);
            ParamEnd();
        }
        return *this;
    }

    // Serialize pointers to primitives
    template<typename ValType, ENABLE_FOR_SERIAL_TYPE(Primitive)>
    inline IArchive& operator<<(TSerialParameter<ValType*>& rParam)
    {
        if (ParamBegin(rParam.pkName))
        {
            if (IsReader() && !rParam.rValue) rParam.rValue = new ValType;
            SerializePrimitive(*rParam.rValue);
            ParamEnd();
        }
        return *this;
    }

    // Serialize hex primitives
    template<typename ValType, ENABLE_FOR_SERIAL_TYPE(Primitive)>
    inline IArchive& operator<<(THexSerialParameter<ValType>& rParam)
    {
        if (ParamBegin(rParam.pkName))
        {
            SerializeHexPrimitive(rParam.rValue);
            ParamEnd();
        }
        return *this;
    }

    // Serialize pointers to hex primitives
    template<typename ValType, ENABLE_FOR_SERIAL_TYPE(Primitive)>
    inline IArchive& operator<<(THexSerialParameter<ValType*>& rParam)
    {
        if (ParamBegin(rParam.pkName))
        {
            if (IsReader() && !rParam.rValue) rParam.rValue = new ValType;
            SerializeHexPrimitive(*rParam.rValue);
            ParamEnd();
        }
        return *this;
    }

    // Serialize objects with member Serialize methods
    template<typename ValType, ENABLE_FOR_SERIAL_TYPE(Member)>
    inline IArchive& operator<<(TSerialParameter<ValType>& rParam)
    {
        if (ParamBegin(rParam.pkName))
        {
            rParam.rValue.Serialize(*this);
            ParamEnd();
        }
        return *this;
    }

    // Serialize pointers to objects with member Serialize methods
    template<typename ValType, ENABLE_FOR_SERIAL_TYPE(Member)>
    inline IArchive& operator<<(TSerialParameter<ValType*>& rParam)
    {
        if (ParamBegin(rParam.pkName))
        {
            if (IsReader() && !rParam.rValue) rParam.rValue = new ValType;
            rParam.rValue->Serialize(*this);
            ParamEnd();
        }
        return *this;
    }

    // Serialize objects with global Serialize functions
    template<typename ValType, ENABLE_FOR_SERIAL_TYPE(Global)>
    inline IArchive& operator<<(TSerialParameter<ValType>& rParam)
    {
        if (ParamBegin(rParam.pkName))
        {
            Serialize(*this, rParam.rValue);
            ParamEnd();
        }
        return *this;
    }

    // Serialize pointers to objects with global Serialize functions
    template<typename ValType, ENABLE_FOR_SERIAL_TYPE(Global)>
    inline IArchive& operator<<(TSerialParameter<ValType*>& rParam)
    {
        if (ParamBegin(rParam.pkName))
        {
            if (IsReader() && !rParam.rValue) rParam.rValue = new ValType;
            Serialize(*this, *rParam.rValue);
            ParamEnd();
        }
        return *this;
    }

    // Serialize abstract objects (global methods for abstract objects not supported)
    template<typename ValType, typename FactoryType, ENABLE_FOR_SERIAL_TYPE(Member)>
    inline IArchive& operator<<(TAbstractSerialParameter<ValType, FactoryType>& rParam)
    {
        if (ParamBegin(rParam.pkName))
        {
            // Serialize object type
            u32 Type = (IsWriter() ? (u32) rParam.rValue->Type() : 0);
            SerializeAbstractObjectType(Type);

            // Reader only - instantiate object
            if (IsReader())
            {
                rParam.rValue = static_cast<ValType*>(rParam.pFactory->SpawnObject(Type));
                ASSERT(rParam.rValue != nullptr);
            }

            // Finish serialize
            rParam.rValue->Serialize(*this);
            ParamEnd();
        }

        return *this;
    }

    // Serialize containers (member methods for containers not supported)
    template<typename ValType, ENABLE_FOR_SERIAL_TYPE(Global)>
    inline IArchive& operator<<(TContainerSerialParameter<ValType>& rParam)
    {
        if (ParamBegin(rParam.pkName))
        {
            SerializeContainer(*this, rParam.rContainer, rParam.pkElementName);
            ParamEnd();
        }

        return *this;
    }

    // Serialize abstract containers (member methods for containers not supported)
    template<typename ValType, typename FactoryType, ENABLE_FOR_SERIAL_TYPE(Global)>
    inline IArchive& operator<<(TAbstractContainerSerialParameter<ValType, FactoryType>& rParam)
    {
        if (ParamBegin(rParam.pkName))
        {
            SerializeContainer(*this, rParam.rContainer, rParam.pkElementName, rParam.pFactory);
            ParamEnd();
        }

        return *this;
    }

    // Generate compiler errors for classes with no valid Serialize function defined
    template<typename ValType, ENABLE_FOR_SERIAL_TYPE(None)>
    inline IArchive& operator<<(TSerialParameter<ValType>&)
    {
        static_assert(false, "Object being serialized has no valid Serialize method defined.");
    }

    template<typename ValType, typename FactoryType, ENABLE_FOR_SERIAL_TYPE(None)>
    inline IArchive& operator<<(TAbstractSerialParameter<ValType, FactoryType>&)
    {
        static_assert(false, "Abstract object being serialized must have a virtual Serialize method defined.");
    }

    template<typename ValType, ENABLE_FOR_SERIAL_TYPE(None)>
    inline IArchive& operator<<(TContainerSerialParameter<ValType>&)
    {
        static_assert(false, "Container being serialized has no valid Serialize method defined.");
    }

    template<typename ValType, typename FactoryType, ENABLE_FOR_SERIAL_TYPE(None)>
    inline IArchive& operator<<(TAbstractContainerSerialParameter<ValType, FactoryType>&)
    {
        static_assert(false, "Container being serialized has no valid Serialize method defined.");
    }

    // Interface
    virtual bool ParamBegin(const char *pkName) = 0;
    virtual void ParamEnd() = 0;

    virtual void SerializeContainerSize(u32& rSize) = 0;
    virtual void SerializeAbstractObjectType(u32& rType) = 0;

    virtual void SerializePrimitive(bool& rValue) = 0;
    virtual void SerializePrimitive(char& rValue) = 0;
    virtual void SerializePrimitive(s8& rValue) = 0;
    virtual void SerializePrimitive(u8& rValue) = 0;
    virtual void SerializePrimitive(s16& rValue) = 0;
    virtual void SerializePrimitive(u16& rValue) = 0;
    virtual void SerializePrimitive(s32& rValue) = 0;
    virtual void SerializePrimitive(u32& rValue) = 0;
    virtual void SerializePrimitive(s64& rValue) = 0;
    virtual void SerializePrimitive(u64& rValue) = 0;
    virtual void SerializePrimitive(float& rValue) = 0;
    virtual void SerializePrimitive(double& rValue) = 0;
    virtual void SerializePrimitive(TString& rValue) = 0;
    virtual void SerializePrimitive(TWideString& rValue) = 0;
    virtual void SerializePrimitive(CFourCC& rValue) = 0;
    virtual void SerializePrimitive(CAssetID& rValue) = 0;

    virtual void SerializeHexPrimitive(u8& rValue) = 0;
    virtual void SerializeHexPrimitive(u16& rValue) = 0;
    virtual void SerializeHexPrimitive(u32& rValue) = 0;
    virtual void SerializeHexPrimitive(u64& rValue) = 0;

    // Accessors
    inline u16 ArchiveVersion() const   { return mArchiveVersion; }
    inline u16 FileVersion() const      { return mFileVersion; }
    inline EGame Game() const           { return mGame; }
    inline bool IsReader() const        { return mIsReader; }
    inline bool IsWriter() const        { return mIsWriter; }

    inline void SetVersion(u16 ArchiveVersion, u16 FileVersion, EGame Game)
    {
        mArchiveVersion = ArchiveVersion;
        mFileVersion = FileVersion;
        mGame = Game;
    }

    inline void SetVersion(const CSerialVersion& rkVersion)
    {
        mArchiveVersion = rkVersion.ArchiveVersion();
        mFileVersion = rkVersion.FileVersion();
        mGame = rkVersion.Game();
    }

    inline CSerialVersion GetVersionInfo() const
    {
        return CSerialVersion(mArchiveVersion, mFileVersion, mGame);
    }
};

// Container serialize methods
#include <list>
#include <set>
#include <vector>

template<typename Container>
inline void SerializeContainerSize(IArchive& rArc, Container& rContainer)
{
    u32 Size = rContainer.size();
    rArc.SerializeContainerSize(Size);
    if (rArc.IsReader()) rContainer.resize(Size);
}

// std::vector
template<typename ValType>
inline void SerializeContainer(IArchive& rArc, std::vector<ValType>& rVec, const TString& rkElemName = "Item")
{
    SerializeContainerSize(rArc, rVec);

    for (u32 iElem = 0; iElem < rVec.size(); iElem++)
        rArc << SERIAL(*rkElemName, rVec[iElem]);
}

template<typename ValType, typename FactoryType>
inline void SerializeContainer(IArchive& rArc, std::vector<ValType>& rVec, const TString& rkElemName, FactoryType *pFactory)
{
    SerializeContainerSize(rArc, rVec);

    for (u32 iElem = 0; iElem < rVec.size(); iElem++)
        rArc << SERIAL_ABSTRACT(*rkElemName, rVec[iElem], pFactory);
}

// std::list
template<typename ValType>
inline void SerializeContainer(IArchive& rArc, std::list<ValType>& rList, const TString& rkElemName)
{
    SerializeContainerSize(rArc, rList);

    for (auto Iter = rList.begin(); Iter != rList.end(); Iter++)
        rArc << SERIAL(*rkElemName, *Iter);
}

template<typename ValType, typename FactoryType>
inline void SerializeContainer(IArchive& rArc, std::list<ValType>& rList, const TString& rkElemName, FactoryType *pFactory)
{
    SerializeContainerSize(rArc, rList);

    for (auto Iter = rList.begin(); Iter != rList.end(); Iter++)
        rArc << SERIAL_ABSTRACT(*rkElemName, rVec[iElem], pFactory);
}

// std::set
template<typename ValType>
inline void SerializeContainer(IArchive& rArc, std::set<ValType>& rSet, const TString& rkElemName)
{
    u32 Size = rSet.size();
    rArc.SerializeContainerSize(Size);

    if (rArc.IsReader())
    {
        for (u32 iElem = 0; iElem < Size; iElem++)
        {
            ValType Val;
            rArc << SERIAL(*rkElemName, Val);
            rSet.insert(Val);
        }
    }

    else
    {
        for (auto Iter = rSet.begin(); Iter != rSet.end(); Iter++)
        {
            ValType Val = *Iter;
            rArc << SERIAL(*rkElemName, Val);
        }
    }
}

template<typename ValType, typename FactoryType>
inline void SerializeContainer(IArchive& rArc, std::set<ValType>& rSet, const TString& rkElemName, FactoryType *pFactory)
{
    u32 Size = rSet.size();
    rArc.SerializeContainerSize(Size);

    if (rArc.IsReader())
    {
        for (u32 iElem = 0; iElem < Size; iElem++)
        {
            ValType Val;
            rArc << SERIAL_ABSTRACT(*rkElemName, Val, pFactory);
            rSet.insert(Val);
        }
    }

    else
    {
        for (auto Iter = rSet.begin(); Iter != rSet.end(); Iter++)
        {
            ValType Val = *Iter;
            rArc << SERIAL_ABSTRACT(*rkElemName, Val, pFactory);
        }
    }
}

#endif // IARCHIVE
