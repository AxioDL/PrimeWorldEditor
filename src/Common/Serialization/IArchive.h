#ifndef IARCHIVE
#define IARCHIVE

#include "CSerialVersion.h"
#include "Common/AssertMacro.h"
#include "Common/CAssetID.h"
#include "Common/CFourCC.h"
#include "Common/EGame.h"
#include "Common/TString.h"
#include "Common/types.h"

#include <type_traits>

#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

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
 * wrapped in a call to SerialParameter(), which allows each value to be associated with a name
 * and other parameters. This primarily helps make the output files more human-readable and
 * assists with backwards compatibility, as well as customizing how each parameter is serialized.
 *
 * Polymorphism is supported. There are two requirements for a polymorphic class to work with the
 * serialization system. First, the base class must contain a virtual Type() function, as well as
 * a virtual Serialize(IArchive&) function. Second, the class must have a static ArchiveConstructor()
 * method that takes the a Type value (which should be the same type as Type() returns), and returns
 * an object of the correct class.
 *
 * SerialParameter() can take flags that provides hints to the serializer. This can either customize
 * how the parameter is displayed in the file, or it can modify how the serialization is done under
 * the hood. For a list of possible hints, check the definition of ESerialHint.
 */

/** ESerialHint - Parameter hint flags */
enum ESerialHint
{
    SH_HexDisplay           = 0x1,      // The parameter should be written in hex in text formats.
    SH_Optional             = 0x2,      // The parameter should not be written to the file if its value matches the default value.
    SH_NeverSave            = 0x4,      // The parameter should not be saved to files.
    SH_AlwaysSave           = 0x8,      // The parameter should always be saved regardless of if it matches the default value.
    SH_Attribute            = 0x10,     // The parameter is an attribute of its parent. Attributes cannot have children.
    SH_IgnoreName           = 0x20,     // The parameter name will not be used to validate file data. May yield incorrect results if used improperly!
    SH_InheritHints         = 0x40,     // The parameter will inherit hints from its parent parameter (except for this flag).
    SH_Proxy                = 0x80,     // The parameter is a proxy of the parent and will display inline instead of as a child parameter.
};

// Hints that can be inherited by SH_InheritHints and SH_Proxy
const int gkInheritableSerialHints = (SH_HexDisplay | SH_NeverSave | SH_AlwaysSave);

/** EArchiveFlags */
enum EArchiveFlags
{
    AF_Reader               = 0x1,      // Archive reads data.
    AF_Writer               = 0x2,      // Archive writes data.
    AF_Text                 = 0x4,      // Archive reads/writes to a text format.
    AF_Binary               = 0x8,      // Archive reads/writes to a binary format.
    AF_NoSkipping           = 0x10,     // Properties are never skipped.
};

/** Shortcut macro for enable_if */
#define ENABLE_IF(Conditions, ReturnType) typename std::enable_if< Conditions, ReturnType >::type

/** Check for whether the equality operator has been implemented for a given type */
template<typename ValType, class = decltype(std::declval<ValType>() == std::declval<ValType>())>
std::true_type  THasEqualToOperator(const ValType&);
std::false_type THasEqualToOperator(...);
template<typename ValType> using THasEqualTo = decltype(THasEqualToOperator(std::declval<ValType>()));

/** Class that determines if the type is a container */
template<typename>      struct TIsContainer : std::false_type {};
template<typename T>    struct TIsContainer< std::vector<T> > : std::true_type {};
template<typename T>    struct TIsContainer< std::list<T> > : std::true_type {};
template<typename T>    struct TIsContainer< std::set<T> > : std::true_type {};
template<typename T, typename V>    struct TIsContainer< std::map<T,V> > : std::true_type {};
template<typename T, typename V>    struct TIsContainer< std::unordered_map<T,V> > : std::true_type {};

/** Class that determines if the type is a smart pointer */
template<typename>      struct TIsSmartPointer : std::false_type {};
template<typename T>    struct TIsSmartPointer< std::shared_ptr<T> > : std::true_type {};
template<typename T>    struct TIsSmartPointer< std::unique_ptr<T> > : std::true_type {};

/** Helper macro that tells us whether the parameter supports default property values */
#define SUPPORTS_DEFAULT_VALUES (!std::is_pointer_v<ValType> && std::is_copy_assignable_v<ValType> && THasEqualTo<ValType>::value && !TIsContainer<ValType>::value && !TIsSmartPointer<ValType>::value)

/** TSerialParameter - name/value pair for generic serial parameters */
template<typename ValType>
struct TSerialParameter
{
    const char*         pkName;
    ValType&            rValue;
    u32                 HintFlags;
    const ValType*      pDefaultValue;
};

/** Function that creates a SerialParameter */
template<typename ValType>
ENABLE_IF( SUPPORTS_DEFAULT_VALUES, TSerialParameter<ValType> )
inline SerialParameter(const char* pkName, ValType& rValue, u32 HintFlags = 0, const ValType& rkDefaultValue = ValType())
{
    return TSerialParameter<ValType> { pkName, rValue, HintFlags, &rkDefaultValue };
}
template<typename ValType>
ENABLE_IF( !SUPPORTS_DEFAULT_VALUES, TSerialParameter<ValType> )
inline SerialParameter(const char* pkName, ValType& rValue, u32 HintFlags = 0)
{
    return TSerialParameter<ValType> { pkName, rValue, HintFlags, nullptr };
}

/** Returns whether the parameter value matches its default value */
template<typename ValType>
ENABLE_IF( SUPPORTS_DEFAULT_VALUES, bool )
inline ParameterMatchesDefault( const TSerialParameter<ValType>& kParameter )
{
    return kParameter.pDefaultValue != nullptr && kParameter.rValue == *kParameter.pDefaultValue;
}

template<typename ValType>
ENABLE_IF( !SUPPORTS_DEFAULT_VALUES && TIsContainer<ValType>::value, bool )
inline ParameterMatchesDefault( const TSerialParameter<ValType>& kParameter )
{
    return kParameter.rValue.size() == 0;
}

template<typename ValType>
ENABLE_IF( !SUPPORTS_DEFAULT_VALUES && !TIsContainer<ValType>::value, bool )
inline ParameterMatchesDefault( const TSerialParameter<ValType>& )
{
    return false;
}

/** Initializes the parameter to its default value */
template<typename ValType>
ENABLE_IF( SUPPORTS_DEFAULT_VALUES, bool )
inline InitParameterToDefault( TSerialParameter<ValType>& Param )
{
    if (Param.pDefaultValue != nullptr)
    {
        Param.rValue = *Param.pDefaultValue;
        return true;
    }
    else
        return false;
}

template<typename ValType>
ENABLE_IF( !SUPPORTS_DEFAULT_VALUES && TIsContainer<ValType>::value, bool )
inline InitParameterToDefault( TSerialParameter<ValType>& Param )
{
    Param.rValue.clear();
    return true;
}

template<typename ValType>
ENABLE_IF( !SUPPORTS_DEFAULT_VALUES && !TIsContainer<ValType>::value, bool )
inline InitParameterToDefault( TSerialParameter<ValType>& )
{
    return false;
}

/** SFINAE serialize function type check */
// https://jguegant.github.io/blogs/tech/sfinae-introduction.html
void Serialize(); // This needs to be here or else the global Serialize method handling causes a compiler error. Not sure of a better fix
void BulkSerialize();

/** Helper struct to verify that function Func exists and matches the given function signature (FuncType) */
template<typename FuncType, FuncType Func> struct FunctionExists;

/** Determine what kind of Serialize function the type has */
template<typename ValueType, class ArchiveType>
struct SerialType
{
    enum { Primitive, Member, Global, None };

    /** Check for ArcType::SerializePrimitive(ValType&) method */
    template<typename V, typename A>
    static constexpr auto HasPrimitiveSerialize(int) -> decltype(
            std::declval<A>().SerializePrimitive( std::declval<V&>(), u32() ), bool()
        )
    {
        return true;
    }

    template<typename V, typename A>
    static constexpr bool HasPrimitiveSerialize(...)
    {
        return false;
    }

    /** Check for ValType::Serialize(ArcType&) */
    template<typename V, typename A>
    static constexpr auto HasMemberSerialize(int) -> decltype(
            std::declval<V>().Serialize( std::declval<A&>() ), bool()
        )
    {
        return true;
    }

    template<typename V, typename A>
    static constexpr bool HasMemberSerialize(...)
    {
        return false;
    }

    // Check for global Serialize(ArcType&,ValType&) function
    template<typename V, typename A>
    static constexpr auto HasGlobalSerialize(int) -> decltype(
            Serialize( std::declval<A&>(), std::declval<V&>() ), bool()
        )
    {
        return true;
    }

    template<typename V, typename A>
    static constexpr bool HasGlobalSerialize(...)
    {
        return false;
    }

    // Set Type enum to correspond to whichever function exists
    static const int Type = (HasPrimitiveSerialize<ValueType, ArchiveType>(0) ? Primitive :
                             HasMemberSerialize<ValueType, ArchiveType>(0) ? Member :
                             HasGlobalSerialize<ValueType, ArchiveType>(0) ? Global :
                             None);
};

/** For abstract types, determine what kind of ArchiveConstructor the type has */
template<typename ValType, class ArchiveType>
struct ArchiveConstructorType
{
    /** Figure out the type being used to represent the object type.
     *  If there isn't a type function, then it doesn't matter; just substitute int.
     */
    template<typename T>
    static constexpr auto HasTypeMethod(int) -> decltype( std::declval<T>().Type() )
    {
        return std::declval<T>().Type();
    }

    template<typename T>
    static constexpr int HasTypeMethod(...)
    {
        return 0;
    }

    using ObjType = decltype(HasTypeMethod<ValType>(0));

    enum { Basic, Advanced, None };

    /** Check for ValType::ArchiveConstructor(ObjectType) */
    template<typename V, typename O>
    static constexpr auto HasBasicArchiveConstructor(int) -> decltype(
            std::declval<V>().ArchiveConstructor( std::declval<O>()), bool()
        )
    {
        return true;
    }

    template<typename V, typename O>
    static constexpr bool HasBasicArchiveConstructor(...)
    {
        return false;
    }

    /** Check for ValType::ArchiveConstructor(ObjectType, IArchive&) */
    template<typename V, typename A, typename O>
    static constexpr auto HasAdvancedArchiveConstructor(int) -> decltype(
            std::declval<V>().ArchiveConstructor( std::declval<O>(), std::declval<A&>() ), bool()
        )
    {
        return true;
    }

    template<typename V, typename A, typename O>
    static constexpr bool HasAdvancedArchiveConstructor(...)
    {
        return false;
    }

    // Set Type enum to correspond to whichever function exists
    static const int Type = (HasAdvancedArchiveConstructor<ValType, ArchiveType, ObjType>(0) ? Advanced :
                             HasBasicArchiveConstructor<ValType, ObjType>(0) ? Basic :
                             None);
};

/** Helper that turns functions on or off depending on their serialize type */
#define IS_SERIAL_TYPE(SType) (SerialType<ValType, IArchive>::Type == SerialType<ValType, IArchive>::##SType)

/** Helper that turns functions on or off depending on their StaticConstructor type */
#define IS_ARCHIVE_CONSTRUCTOR_TYPE(CType) (ArchiveConstructorType<ValType, IArchive>::Type == ArchiveConstructorType<ValType, IArchive>::##CType)

/** Helper that turns functions on or off depending on if the parameter type is abstract */
#define IS_ABSTRACT ( std::is_abstract_v<ValType> || (std::is_polymorphic_v<ValType> && ArchiveConstructorType<ValType, IArchive>::Type != ArchiveConstructorType<ValType, IArchive>::None) )

/** IArchive - Main serializer archive interface */
class IArchive
{
protected:
    u16 mArchiveVersion;
    u16 mFileVersion;
    EGame mGame;

    // Subclasses must fill in flags in their constructors!!!
    u32 mArchiveFlags;

    // Info about the stack of parameters being serialized
    struct SParmStackEntry
    {
        size_t TypeID;
        size_t TypeSize;
        void* pDataPointer;
        u32 HintFlags;
    };
    std::vector<SParmStackEntry> mParmStack;

public:
    enum EArchiveVersion
    {
        eArVer_Initial,
        eArVer_32BitBinarySize,
        eArVer_Refactor,
        eArVer_MapAttributes,
        eArVer_GameEnumClass,
        // Insert new versions before this line
        eArVer_Max
    };
    static const u32 skCurrentArchiveVersion = (eArVer_Max - 1);

    IArchive()
        : mFileVersion(0)
        , mArchiveVersion(skCurrentArchiveVersion)
        , mGame(EGame::Invalid)
        , mArchiveFlags(0)
    {
        // hack to reduce allocations
        mParmStack.reserve(16);
    }

    virtual ~IArchive() { ASSERT(mParmStack.empty()); }

    // Serialize archive version. Always call after opening a file.
    void SerializeVersion()
    {
        *this << SerialParameter("ArchiveVer",  mArchiveVersion,    SH_Attribute)
              << SerialParameter("FileVer",     mFileVersion,       SH_Attribute | SH_Optional,     (u16) 0)
              << SerialParameter("Game",        mGame,              SH_Attribute | SH_Optional,     EGame::Invalid);

        if (IsReader())
        {
            if (mArchiveVersion > skCurrentArchiveVersion)
            {
                mArchiveVersion = skCurrentArchiveVersion;
            }
        }
    }

private:
    // Attempts to start a new parameter. Return whether the parameter should be serialized.
    template<typename ValType>
    bool InternalStartParam(const TSerialParameter<ValType>& Param)
    {
        bool IsProxy = (Param.HintFlags & SH_Proxy) != 0;
        return ShouldSerializeParameter(Param) && (IsProxy || ParamBegin(Param.pkName, Param.HintFlags) );
    }

    // Ends a parameter.
    template<typename ValType>
    void InternalEndParam(const TSerialParameter<ValType>& Param)
    {
        if ((Param.HintFlags & SH_Proxy) == 0)
        {
            ParamEnd();
        }
    }

    // Return whether this parameter should be serialized
    template<typename ValType>
    bool ShouldSerializeParameter(const TSerialParameter<ValType>& Param)
    {
        if (mArchiveFlags & AF_NoSkipping)
            return true;

        if ( IsWriter() )
        {
            if (Param.HintFlags & SH_NeverSave)
                return false;

            if ( ( Param.HintFlags & SH_Optional ) &&
                 ( Param.HintFlags & SH_AlwaysSave ) == 0 &&
                 ( ParameterMatchesDefault(Param) ) )
                return false;
        }

        return true;
    }

    // Instantiate an abstract object from the file
    // Only readers are allowed to instantiate objects
    template<typename ValType, typename ObjType = ABSTRACT_TYPE>
    ENABLE_IF( IS_ARCHIVE_CONSTRUCTOR_TYPE(Basic), ValType* )
    inline InstantiateAbstractObject(const TSerialParameter<ValType*>& Param, ObjType Type)
    {
        // Variant for basic static constructor
        ASSERT( IsReader() );
        return (ValType*) ValType::ArchiveConstructor(Type);
    }

    template<typename ValType, typename ObjType = ABSTRACT_TYPE>
    ENABLE_IF( IS_ARCHIVE_CONSTRUCTOR_TYPE(Advanced), ValType* )
    InstantiateAbstractObject(const TSerialParameter<ValType*>& Param, ObjType Type)
    {
        // Variant for advanced static constructor
        ASSERT( IsReader() );
        return (ValType*) ValType::ArchiveConstructor(Type, *this);
    }

    template<typename ValType, typename ObjType = ABSTRACT_TYPE>
    ENABLE_IF( IS_ARCHIVE_CONSTRUCTOR_TYPE(None), ValType* )
    InstantiateAbstractObject(const TSerialParameter<ValType*>& Param, ObjType Type)
    {
        // If you fail here, you are missing an ArchiveConstructor() function, or you do have one but it is malformed.
        // Check the comments at the top of this source file for details on serialization requirements for abstract objects.
        static_assert(false, "Abstract objects being serialized must have virtual Type() and static ArchiveConstructor() functions.");
    }

    // Parameter stack handling
    template<typename ValType>
    inline void PushParameter(TSerialParameter<ValType>& Param)
    {
#if _DEBUG
        if (mParmStack.size() > 0)
        {
            // Attribute properties cannot have children!
            ASSERT( (mParmStack.back().HintFlags & SH_Attribute) == 0 );
        }
#endif

        // For InheritHints parameters, and for proxy parameters, copy the hint flags from the parent parameter.
        if (Param.HintFlags & (SH_InheritHints | SH_Proxy))
        {
            Param.HintFlags |= (mParmStack.back().HintFlags & gkInheritableSerialHints);
        }

        SParmStackEntry Entry;
        Entry.TypeID = typeid(ValType).hash_code();
        Entry.TypeSize = sizeof(ValType);
        Entry.pDataPointer = &Param.rValue;
        Entry.HintFlags = Param.HintFlags;
        mParmStack.push_back(Entry);
    }

    template<typename ValType>
    inline void PopParameter(const TSerialParameter<ValType>& Param)
    {
#if _DEBUG
        // Make sure the entry matches the param that has been passed in
        ASSERT(mParmStack.size() > 0);
        const SParmStackEntry& kEntry = mParmStack.back();
        ASSERT(kEntry.TypeID == typeid(ValType).hash_code());
        ASSERT(kEntry.pDataPointer == &Param.rValue);
#endif
        mParmStack.pop_back();
    }

public:
    // Serialize primitives
    template<typename ValType>
    ENABLE_IF( IS_SERIAL_TYPE(Primitive), IArchive& )
    operator<<(TSerialParameter<ValType> rParam)
    {
        PushParameter(rParam);

        if (InternalStartParam(rParam))
        {
            SerializePrimitive(rParam.rValue, rParam.HintFlags);
            InternalEndParam(rParam);
        }
        else if (IsReader())
            InitParameterToDefault(rParam);

        PopParameter(rParam);
        return *this;
    }

    template<typename ValType>
    ENABLE_IF( IS_SERIAL_TYPE(Primitive) && !IS_ABSTRACT, IArchive& )
    operator<<(TSerialParameter<ValType*> rParam)
    {
        ASSERT( !(mArchiveFlags & AF_Writer) || rParam.rValue != nullptr );
        PushParameter(rParam);

        if (InternalStartParam(rParam))
        {
            // Support for old versions of archives that serialize types on non-abstract polymorphic pointers
            if (ArchiveVersion() < eArVer_Refactor && IsReader() && std::is_polymorphic_v<ValType>)
            {
                u32 Type;
                *this << SerialParameter("Type", Type, SH_Attribute);
            }

            if (PreSerializePointer(rParam.rValue, rParam.HintFlags))
            {
                if (rParam.rValue == nullptr && (mArchiveFlags & AF_Reader))
                    rParam.rValue = new ValType;

                if (rParam.rValue != nullptr)
                    SerializePrimitive(*rParam.rValue, rParam.HintFlags);
            }
            else if (IsReader())
                rParam.rValue = nullptr;

            InternalEndParam(rParam);
        }

        PopParameter(rParam);
        return *this;
    }

    // Serialize objects with global Serialize functions
    template<typename ValType>
    ENABLE_IF( IS_SERIAL_TYPE(Global), IArchive& )
    inline operator<<(TSerialParameter<ValType> rParam)
    {
        PushParameter(rParam);

        if (InternalStartParam(rParam))
        {
            Serialize(*this, rParam.rValue);
            InternalEndParam(rParam);
        }
        else if (IsReader())
            InitParameterToDefault(rParam);

        PopParameter(rParam);
        return *this;
    }

    template<typename ValType>
    ENABLE_IF( IS_SERIAL_TYPE(Global) && !IS_ABSTRACT, IArchive&)
    operator<<(TSerialParameter<ValType*> rParam)
    {
        ASSERT( !IsWriter() || rParam.rValue != nullptr );
        PushParameter(rParam);

        if (InternalStartParam(rParam))
        {
            // Support for old versions of archives that serialize types on non-abstract polymorphic pointers
            if (ArchiveVersion() < eArVer_Refactor && IsReader() && std::is_polymorphic_v<ValType>)
            {
                u32 Type;
                *this << SerialParameter("Type", Type, SH_Attribute);
            }

            if (PreSerializePointer(rParam.rValue, rParam.HintFlags, rParam.HintFlags))
            {
                if (rParam.rValue == nullptr && IsReader())
                    rParam.rValue = new ValType;

                if (rParam.rValue != nullptr)
                    Serialize(*this, *rParam.rValue);
            }
            else if (IsReader())
                rParam.rValue = nullptr;

            InternalEndParam(rParam);
        }

        PopParameter(rParam);
        return *this;
    }

    // Serialize objects with Serialize methods
    template<typename ValType>
    ENABLE_IF( IS_SERIAL_TYPE(Member), IArchive& )
    operator<<(TSerialParameter<ValType> rParam)
    {
        PushParameter(rParam);

        if (InternalStartParam(rParam))
        {
            rParam.rValue.Serialize(*this);
            InternalEndParam(rParam);
        }
        else if (IsReader())
            InitParameterToDefault(rParam);

        PopParameter(rParam);
        return *this;
    }

    template<typename ValType>
    ENABLE_IF( IS_SERIAL_TYPE(Member) && !IS_ABSTRACT, IArchive& )
    operator<<(TSerialParameter<ValType*> rParam)
    {
        PushParameter(rParam);

        if (InternalStartParam(rParam))
        {
            // Support for old versions of archives that serialize types on non-abstract polymorphic pointers
            if (ArchiveVersion() < eArVer_Refactor && IsReader() && std::is_polymorphic_v<ValType>)
            {
                u32 Type;
                *this << SerialParameter("Type", Type, SH_Attribute);
            }

            if (PreSerializePointer((void*&) rParam.rValue, rParam.HintFlags))
            {
                if (rParam.rValue == nullptr && IsReader())
                    rParam.rValue = new ValType;

                if (rParam.rValue != nullptr)
                    rParam.rValue->Serialize(*this);
            }
            else if (IsReader())
                rParam.rValue = nullptr;

            InternalEndParam(rParam);
        }

        PopParameter(rParam);
        return *this;
    }

    // Serialize polymorphic objects
    template<typename ValType, typename ObjectType = decltype( std::declval<ValType>().Type() )>
    ENABLE_IF( IS_SERIAL_TYPE(Member) && IS_ABSTRACT, IArchive&)
    operator<<(TSerialParameter<ValType*> rParam)
    {
        PushParameter(rParam);

        if (InternalStartParam(rParam))
        {
            if (PreSerializePointer( (void*&) rParam.rValue, rParam.HintFlags ))
            {
                // Non-reader archives cannot instantiate the class. It must exist already.
                if (IsWriter())
                {
                    ObjectType Type = rParam.rValue->Type();
                    *this << SerialParameter("Type", Type, SH_Attribute);
                }
                else
                {
                    // NOTE: If you crash here, it likely means that the pointer was initialized to a garbage value.
                    // It is legal to serialize a pointer that already exists, so you still need to initialize it.
                    ObjectType Type = (rParam.rValue ? rParam.rValue->Type() : ObjectType());
                    ObjectType TypeCopy = Type;
                    *this << SerialParameter("Type", Type, SH_Attribute);

                    if (IsReader() && rParam.rValue == nullptr)
                    {
                        rParam.rValue = (ValType*) InstantiateAbstractObject(rParam, Type);
                    }
                    else if (rParam.rValue != nullptr)
                    {
                        // Make sure the type is what we are expecting
                        ASSERT(Type == TypeCopy);
                    }
                }

                // At this point, the object should exist and is ready for serializing.
                if (rParam.rValue)
                {
                    rParam.rValue->Serialize(*this);
                }
            }
            else if (IsReader())
                rParam.rValue = nullptr;

            InternalEndParam(rParam);
        }
        else
        {
            // Polymorphic types don't support default values
        }

        PopParameter(rParam);
        return *this;
    }

    // Error
    template<typename ValType>
    ENABLE_IF( IS_SERIAL_TYPE(Global) && IS_ABSTRACT, IArchive& )
    operator<<(TSerialParameter<ValType*>)
    {
        static_assert(false, "Global Serialize method for polymorphic type pointers is not supported.");
    }

    // Generate compiler errors for classes with no valid Serialize function defined
    template<typename ValType>
    ENABLE_IF( IS_SERIAL_TYPE(None), IArchive& )
    operator<<(TSerialParameter<ValType>)
    {
        static_assert(false, "Object being serialized has no valid Serialize method defined.");
    }

    // Interface
    virtual bool ParamBegin(const char *pkName, u32 Flags) = 0;
    virtual void ParamEnd() = 0;

    virtual bool PreSerializePointer(void*& Pointer, u32 Flags) = 0;
    virtual void SerializePrimitive(bool& rValue, u32 Flags) = 0;
    virtual void SerializePrimitive(char& rValue, u32 Flags) = 0;
    virtual void SerializePrimitive(s8& rValue, u32 Flags) = 0;
    virtual void SerializePrimitive(u8& rValue, u32 Flags) = 0;
    virtual void SerializePrimitive(s16& rValue, u32 Flags) = 0;
    virtual void SerializePrimitive(u16& rValue, u32 Flags) = 0;
    virtual void SerializePrimitive(s32& rValue, u32 Flags) = 0;
    virtual void SerializePrimitive(u32& rValue, u32 Flags) = 0;
    virtual void SerializePrimitive(s64& rValue, u32 Flags) = 0;
    virtual void SerializePrimitive(u64& rValue, u32 Flags) = 0;
    virtual void SerializePrimitive(float& rValue, u32 Flags) = 0;
    virtual void SerializePrimitive(double& rValue, u32 Flags) = 0;
    virtual void SerializePrimitive(TString& rValue, u32 Flags) = 0;
    virtual void SerializePrimitive(TWideString& rValue, u32 Flags) = 0;
    virtual void SerializePrimitive(CFourCC& rValue, u32 Flags) = 0;
    virtual void SerializePrimitive(CAssetID& rValue, u32 Flags) = 0;
    virtual void SerializeBulkData(void* pData, u32 DataSize, u32 Flags) = 0;

    // Optional - serialize in an array size. By default, just stores size as an attribute property.
    virtual void SerializeArraySize(u32& Value)
    {
        *this << SerialParameter("Size", Value, SH_Attribute);
    }

    // Accessors
    inline bool IsReader() const                { return (mArchiveFlags & AF_Reader) != 0; }
    inline bool IsWriter() const                { return (mArchiveFlags & AF_Writer) != 0; }
    inline bool IsTextFormat() const            { return (mArchiveFlags & AF_Text) != 0; }
    inline bool IsBinaryFormat() const          { return (mArchiveFlags & AF_Binary) != 0; }
    inline bool CanSkipParameters() const       { return (mArchiveFlags & AF_NoSkipping) == 0; }

    inline u16 ArchiveVersion() const   { return mArchiveVersion; }
    inline u16 FileVersion() const      { return mFileVersion; }
    inline EGame Game() const           { return mGame; }

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

    /** Utility function for class versioning */
    u32 SerializeClassVersion(u32 CurrentVersion)
    {
        *this << SerialParameter("ClassVer", CurrentVersion, SH_Attribute | SH_Optional, (u32) 0);
        return CurrentVersion;
    }
};

/** Class that determines if the type is a primitive */
template<typename T>
class TIsPrimitive : std::conditional< SerialType<T,IArchive>::Type == SerialType<T,IArchive>::Primitive, std::true_type, std::false_type >::type
{};

#if WITH_CODEGEN
// Default enum serializer; can be overridden
#include <codegen/EnumReflection.h>

template<typename T, typename = typename std::enable_if< std::is_enum<T>::value >::type>
inline void DefaultEnumSerialize(IArchive& Arc, T& Val)
{
    if (Arc.IsTextFormat())
    {
        if (Arc.IsReader())
        {
            TString ValueName;
            Arc.SerializePrimitive(ValueName, 0);
            Val = TEnumReflection<T>::ConvertStringToValue( *ValueName );
        }
        else
        {
            TString ValueName = TEnumReflection<T>::ConvertValueToString(Val);
            Arc.SerializePrimitive(ValueName, 0);
        }
    }
    else
    {
        Arc.SerializePrimitive((u32&) Val, 0);
    }
}

template<typename T, typename = typename std::enable_if< std::is_enum<T>::value >::type>
inline void Serialize(IArchive& Arc, T& Val)
{
    DefaultEnumSerialize(Arc, Val);
}
#endif

// Container serialize methods

// std::vector
template<typename T>
inline void Serialize(IArchive& Arc, std::vector<T>& Vector)
{
    u32 Size = Vector.size();
    Arc.SerializeArraySize(Size);

    if (Arc.IsReader())
    {
        Vector.resize(Size);
    }

    for (u32 i = 0; i < Size; i++)
    {
        // SH_IgnoreName to preserve compatibility with older files that may have differently-named items
        Arc << SerialParameter("Element", Vector[i], SH_InheritHints | SH_IgnoreName);
    }
}

// overload for std::vector<u8> that serializes in bulk
template<>
inline void Serialize(IArchive& Arc, std::vector<u8>& Vector)
{
    // Don't use SerializeArraySize, bulk data is a special case that overloads may not handle correctly
    u32 Size = Vector.size();
    Arc << SerialParameter("Size", Size, SH_Attribute);

    if (Arc.IsReader())
    {
        Vector.resize(Size);
    }

    Arc.SerializeBulkData(Vector.data(), Vector.size(), 0);
}

// std::list
template<typename T>
inline void Serialize(IArchive& Arc, std::list<T>& List)
{
    u32 Size = List.size();
    Arc.SerializeArraySize(Size);

    if (Arc.IsReader())
    {
        List.resize(Size);
    }

    for (auto Iter = List.begin(); Iter != List.end(); Iter++)
        Arc << SerialParameter("Element", *Iter, SH_IgnoreName | SH_InheritHints);
}

// Overload for TStringList and TWideStringList so they can use the TString/TWideString serialize functions
inline void Serialize(IArchive& Arc, TStringList& List)
{
    std::list<TString>& GenericList = *reinterpret_cast< std::list<TString>* >(&List);
    Serialize(Arc, GenericList);
}

inline void Serialize(IArchive& Arc, TWideStringList& List)
{
    std::list<TWideString>& GenericList = *reinterpret_cast< std::list<TWideString>* >(&List);
    Serialize(Arc, GenericList);
}

// std::set
template<typename T>
inline void Serialize(IArchive& Arc, std::set<T>& Set)
{
    u32 Size = Set.size();
    Arc.SerializeArraySize(Size);

    if (Arc.IsReader())
    {
        for (u32 i = 0; i < Size; i++)
        {
            T Val;
            Arc << SerialParameter("Element", Val, SH_IgnoreName | SH_InheritHints);
            Set.insert(Val);
        }
    }

    else
    {
        for (auto Iter = Set.begin(); Iter != Set.end(); Iter++)
        {
            T Val = *Iter;
            Arc << SerialParameter("Element", Val, SH_IgnoreName | SH_InheritHints);
        }
    }
}

// std::map and std::unordered_map
template<typename KeyType, typename ValType, typename MapType>
inline void SerializeMap_Internal(IArchive& Arc, MapType& Map)
{
    u32 Size = Map.size();
    Arc.SerializeArraySize(Size);

    u32 Hints = SH_IgnoreName | SH_InheritHints;

    // Serialize the key/value as attributes if they are both primitive types.
    if (Arc.ArchiveVersion() >= IArchive::eArVer_MapAttributes && TIsPrimitive<KeyType>::value && TIsPrimitive<ValType>::value)
    {
        Hints |= SH_Attribute;
    }

    if (Arc.IsReader())
    {
        for (u32 i = 0; i < Size; i++)
        {
            KeyType Key;
            ValType Val;

            if (Arc.ParamBegin("Element", SH_IgnoreName | SH_InheritHints))
            {
                Arc << SerialParameter("Key", Key, Hints)
                    << SerialParameter("Value", Val, Hints);

                ASSERT(Map.find(Key) == Map.end());
                Map[Key] = Val;
                Arc.ParamEnd();
            }
        }
    }

    else
    {
        for (auto Iter = Map.begin(); Iter != Map.end(); Iter++)
        {
            // Creating copies is not ideal, but necessary because parameters cannot be const.
            // Maybe this can be avoided somehow?
            KeyType Key = Iter->first;
            ValType Val = Iter->second;

            if (Arc.ParamBegin("Element", SH_IgnoreName | SH_InheritHints))
            {
                Arc << SerialParameter("Key", Key, Hints)
                    << SerialParameter("Value", Val, Hints);

                Arc.ParamEnd();
            }
        }
    }
}

template<typename KeyType, typename ValType, typename HashFunc>
inline void Serialize(IArchive& Arc, std::map<KeyType, ValType, HashFunc>& Map)
{
    SerializeMap_Internal<KeyType, ValType, std::map<KeyType, ValType, HashFunc> >(Arc, Map);
}

template<typename KeyType, typename ValType, typename HashFunc>
inline void Serialize(IArchive& Arc, std::unordered_map<KeyType, ValType, HashFunc>& Map)
{
    SerializeMap_Internal<KeyType, ValType, std::unordered_map<KeyType, ValType, HashFunc> >(Arc, Map);
}

// Smart pointer serialize methods
template<typename T>
void Serialize(IArchive& Arc, std::unique_ptr<T>& Pointer)
{
    T* pRawPtr = Pointer.get();
    Arc << SerialParameter("RawPointer", pRawPtr, SH_Proxy);

    if (Arc.IsReader())
        Pointer = std::unique_ptr<T>(pRawPtr);
}

template<typename T>
void Serialize(IArchive& Arc, std::shared_ptr<T>& Pointer)
{
    T* pRawPtr = Pointer.get();
    Arc << SerialParameter("RawPointer", pRawPtr, SH_Proxy);

    if (Arc.IsReader())
        Pointer = std::shared_ptr<T>(pRawPtr);
}

// Remove header-only macros
#undef ENABLE_IF
#undef SUPPORTS_DEFAULT_VALUES
#undef IS_ABSTRACT
#undef IS_SERIAL_TYPE
#undef IS_ARCHIVE_CONSTRUCTOR_TYPE
#undef ABSTRACT_TYPE

#endif // IARCHIVE
