#ifndef IPROPERTYNEW_H
#define IPROPERTYNEW_H

#include "Core/Resource/Animation/CAnimationParameters.h"
#include <Common/Common.h>
#include <Math/CVector3f.h>
#include <Math/MathUtil.h>

#include <memory>

/** Forward declares */
class CMasterTemplate;
class CScriptTemplate;
class CStructPropertyNew;

/** Typedefs */
typedef TString TIDString;

/** Property flags */
enum class EPropertyFlag : u32
{
    /** Property is an archetype (a template for other properties to copy from) */
    IsArchetype					= 0x1,
    /** Property is an array archetype (a template for elements of an array property) */
    IsArrayArchetype			= 0x2,
    /** This property and all its children are a single unit and do not have individual property IDs, sizes, etc. */
    IsAtomic					= 0x4,
    /** We have cached whether the property name is correct */
    HasCachedNameCheck			= 0x40000000,
    /** The name of the property is a match for the property ID hash */
    HasCorrectPropertyName 		= 0x80000000,

    /** Flags that are left intact when copying from an archetype */
    ArchetypeCopyFlags 			= EPropertyFlag::IsAtomic,
    /** Flags that are inheritable from parent */
    InheritableFlags 			= EPropertyFlag::IsArchetype | EPropertyFlag::IsArrayArchetype | EPropertyFlag::IsAtomic,
};
DECLARE_FLAGS_ENUMCLASS(EPropertyFlag, FPropertyFlags)

/** Property type */
enum class EPropertyTypeNew
{
    Bool            = FOURCC('BOOL'),
    Byte            = FOURCC('BYTE'),
    Short           = FOURCC('SHRT'),
    Int             = FOURCC('INT '),
    Float           = FOURCC('REAL'),
    Choice          = FOURCC('CHOI'),
    Enum            = FOURCC('ENUM'),
    Flags           = FOURCC('FLAG'),
    String          = FOURCC('STRG'),
    Vector          = FOURCC('VECT'),
    Color           = FOURCC('COLR'),
    Asset           = FOURCC('ASST'),
    Sound           = FOURCC('SOND'),
    Animation       = FOURCC('ANIM'),
    AnimationSet    = FOURCC('ANMS'),
    Sequence        = FOURCC('SQNC'),
    Spline          = FOURCC('SPLN'),
    Guid            = FOURCC('GUID'),
    Pointer         = FOURCC('PNTR'),
    Struct          = FOURCC('STRC'),
    Array           = FOURCC('ARRY'),
    Invalid         = FOURCC('INVD')
};

inline const char* PropEnumToHashableTypeName(EPropertyTypeNew Type)
{
    switch (Type)
    {
    // these names are required to generate accurate property ID hashes
    case EPropertyTypeNew::Bool:    return "bool";
    case EPropertyTypeNew::Int:     return "int";
    case EPropertyTypeNew::Float:   return "float";
    case EPropertyTypeNew::Choice:  return "choice";
    case EPropertyTypeNew::Enum:    return "enum";
    case EPropertyTypeNew::Flags:   return "Flags";
    case EPropertyTypeNew::String:  return "string";
    case EPropertyTypeNew::Vector:  return "Vector";
    case EPropertyTypeNew::Color:   return "Color";
    case EPropertyTypeNew::Asset:   return "asset";
    case EPropertyTypeNew::Sound:   return "sound";
    case EPropertyTypeNew::Spline:  return "spline";
    case EPropertyTypeNew::Guid:    return "guid";
    // unknown hashable types - used in hashes but these names are inaccurate
    case EPropertyTypeNew::Animation:   return "animation"; // hashable but real name unknown
    case EPropertyTypeNew::Sequence:    return "sequence";
    // non hashable types - not used in ID hashes but still displayed on the UI
    case EPropertyTypeNew::Byte:    return "byte";
    case EPropertyTypeNew::Short:   return "short";
    case EPropertyTypeNew::Array:   return "array";
    // fallback
    default:                        return "";
    }
}

/** Enum that describes when/how properties should be cooked out */
enum class ECookPreferenceNew
{
    Default,
    Always,
    Never
};

/** New property class */
class IPropertyNew
{
    friend class CTemplateLoader;
    friend class CPropertyFactory;

protected:
    /** Flags */
    FPropertyFlags mFlags;

    /** Parent property */
    IPropertyNew* mpParent;

    /** Pointer parent; if non-null, this parent needs to be dereferenced to access the correct
     *  memory region that our property data is stored in */
    IPropertyNew* mpPointerParent;

    /** Archetype property; source property that we copied metadata from */
    IPropertyNew* mpArchetype;

    /** Sub-instances of archetype properties. For non-archetypes, will be empty. @todo better
     *  method of storing this? maybe a linked list? */
    std::vector<IPropertyNew*> mSubInstances;

    /** Child properties; these appear underneath this property on the UI */
    std::vector<IPropertyNew*> mChildren;

    /** Game this property belongs to */
    EGame mGame;

    /** Script template that this property belongs to. Null for struct/enum/flag archetypes. */
    CScriptTemplate* mpScriptTemplate;

    /** Offset of this property within the property block */
    u32 mOffset;

    /** Property ID. This ID is used to uniquely identify this property within this struct. */
    u32 mID;

    /** Property metadata */
    TString mName;
    TString mDescription;
    TString mSuffix;
    ECookPreferenceNew mCookPreference;

    /** Min/max allowed version number. These numbers correspond to the game's internal build number.
     *  This is not used yet but in the future it can be used to configure certain properties to only
     *  show up when certain versions of the game are being edited. The default values allow the
     *  property to show up in all versions. */
    float mMinVersion;
    float mMaxVersion;

    /** Private constructor - use static methods to instantiate */
    IPropertyNew();
    void _CalcOffset();
    void _ClearChildren();

    /** Called after property is created and fully initialized */
    virtual void PostInitialize() {}

public:
    virtual ~IPropertyNew();

    /** Interface */
    virtual EPropertyTypeNew Type() const = 0;
    virtual u32 DataSize() const = 0;
    virtual u32 DataAlignment() const = 0;
    virtual void Construct(void* pData) const = 0;
    virtual void Destruct(void* pData) const = 0;
    virtual bool MatchesDefault(void* pData) const = 0;
    virtual void RevertToDefault(void* pData) const = 0;
    virtual void SerializeValue(void* pData, IArchive& Arc) const = 0;

    virtual void PropertyValueChanged(void* pPropertyData)  {}
    virtual bool IsNumericalType() const                    { return false; }
    virtual bool IsPointerType() const                      { return false; }
    virtual TString ValueAsString(void* pData) const        { return ""; }

    virtual const char* HashableTypeName() const;
    virtual void* GetChildDataPointer(void* pPropertyData) const;
    virtual void Serialize(IArchive& rArc);
    virtual void InitFromArchetype(IPropertyNew* pOther);
    virtual bool ShouldSerialize() const;
    virtual TString GetTemplateFileName();
    
    /** Utility methods */
    void* RawValuePtr(void* pData) const;
    IPropertyNew* ChildByID(u32 ID) const;
    IPropertyNew* ChildByIDString(const TIDString& rkIdString);
    bool ShouldCook(void* pPropertyData) const;
    void SetName(const TString& rkNewName);
    void SetDescription(const TString& rkNewDescription);
    void SetSuffix(const TString& rkNewSuffix);
    bool HasAccurateName();

    /** Accessors */
    EGame Game() const;
    inline ECookPreferenceNew CookPreference() const;
    inline u32 NumChildren() const;
    inline IPropertyNew* ChildByIndex(u32 ChildIndex) const;
    inline IPropertyNew* Parent() const;
    inline IPropertyNew* RootParent();
    inline IPropertyNew* Archetype() const;
    inline CScriptTemplate* ScriptTemplate() const;
    inline TString Name() const;
    inline TString Description() const;
    inline TString Suffix() const;
    inline TIDString IDString(bool FullyQualified) const;
    inline u32 Offset() const;
    inline u32 ID() const;

    inline bool IsArchetype() const         { return mFlags.HasFlag(EPropertyFlag::IsArchetype); }
    inline bool IsArrayArchetype() const    { return mFlags.HasFlag(EPropertyFlag::IsArrayArchetype); }
    inline bool IsAtomic() const            { return mFlags.HasFlag(EPropertyFlag::IsAtomic); }
    inline bool IsRootParent() const        { return mpParent == nullptr; }

    /** Create */
    static IPropertyNew* Create(EPropertyTypeNew Type,
                                IPropertyNew* pParent,
                                EGame Game,
                                CScriptTemplate* pScript,
                                bool CallPostInit = true);

    static IPropertyNew* CreateCopy(IPropertyNew* pArchetype,
                                    IPropertyNew* pParent);

    static IPropertyNew* CreateIntrinsic(EPropertyTypeNew Type,
                                         IPropertyNew* pParent,
                                         u32 Offset,
                                         const TString& rkName);

    static IPropertyNew* ArchiveConstructor(EPropertyTypeNew Type,
                                           const IArchive& Arc);
};

inline ECookPreferenceNew IPropertyNew::CookPreference() const
{
    return mCookPreference;
}

inline u32 IPropertyNew::NumChildren() const
{
    return mChildren.size();
}

inline IPropertyNew* IPropertyNew::ChildByIndex(u32 ChildIndex) const
{
    ASSERT(ChildIndex >= 0 && ChildIndex < mChildren.size());
    return mChildren[ChildIndex];
}

inline IPropertyNew* IPropertyNew::Parent() const
{
    return mpParent;
}

inline IPropertyNew* IPropertyNew::RootParent()
{
    IPropertyNew* pParent = Parent();
    IPropertyNew* pOut = this;

    while (pParent)
    {
        pOut = pParent;
        pParent = pParent->Parent();
    }

    return pOut;
}

inline IPropertyNew* IPropertyNew::Archetype() const
{
    return mpArchetype;
}

inline CScriptTemplate* IPropertyNew::ScriptTemplate() const
{
    return mpScriptTemplate;
}

inline TString IPropertyNew::Name() const
{
    return mName;
}

inline TString IPropertyNew::Description() const
{
    return mDescription;
}

inline TString IPropertyNew::Suffix() const
{
    return mSuffix;
}

inline TString IPropertyNew::IDString(bool FullyQualified) const
{
    if (FullyQualified && mpParent != nullptr && mpParent->Parent() != nullptr)
        return mpParent->IDString(FullyQualified) + ":" + TString::HexString(mID);
    else
        return TString::HexString(mID);
}

inline u32 IPropertyNew::Offset() const
{
    return mOffset;
}

inline u32 IPropertyNew::ID() const
{
    return mID;
}

template<typename PropType, EPropertyTypeNew PropEnum>
class TTypedPropertyNew : public IPropertyNew
{
    friend class IPropertyNew;
    friend class CTemplateLoader;
public:
    typedef PropType ValueType;
    
protected:
    PropType mDefaultValue;

    TTypedPropertyNew()
        : IPropertyNew()
    {
        memset(&mDefaultValue, 0, sizeof(PropType));
    }

public:
    virtual EPropertyTypeNew Type() const           { return PropEnum; }
    virtual u32 DataSize() const                    { return sizeof(PropType); }
    virtual u32 DataAlignment() const               { return alignof(PropType); }
    virtual void Construct(void* pData) const       { new(ValuePtr(pData)) PropType(mDefaultValue); }
    virtual void Destruct(void* pData) const        { ValueRef(pData).~PropType(); }
    virtual bool MatchesDefault(void* pData) const  { return ValueRef(pData) == mDefaultValue; }
    virtual void RevertToDefault(void* pData) const { ValueRef(pData) = mDefaultValue; }

    virtual bool CanHaveDefault() const { return true; }

    virtual void InitFromArchetype(IPropertyNew* pOther)
    {
        IPropertyNew::InitFromArchetype(pOther);
        mDefaultValue = static_cast<TTypedPropertyNew*>(pOther)->mDefaultValue;
    }

    inline PropType* ValuePtr(void* pData) const
    {
        return (PropType*) RawValuePtr(pData);
    }

    inline PropType& ValueRef(void* pData) const
    {
        return *ValuePtr(pData);
    }

    inline PropType Value(void* pData) const
    {
        return *ValuePtr(pData);
    }

    inline const PropType& DefaultValue() const
    {
        return mDefaultValue;
    }

    inline static EPropertyTypeNew StaticType()     { return PropEnum; }
};

template<typename PropType, EPropertyTypeNew PropEnum>
class TSerializeableTypedProperty : public TTypedPropertyNew<PropType, PropEnum>
{
protected:
    TSerializeableTypedProperty()
        : TTypedPropertyNew()
    {}

public:
    virtual void Serialize(IArchive& rArc)
    {
        TTypedPropertyNew::Serialize(rArc);
        TSerializeableTypedProperty* pArchetype = static_cast<TSerializeableTypedProperty*>(mpArchetype);

        // Determine if default value should be serialized as optional.
        // All MP1 properties should be optional. For MP2 and on, we set optional
        // on property types that don't have default values in the game executable.
        bool MakeOptional = false;

        if (Game() <= ePrime || pArchetype != nullptr)
        {
            MakeOptional = true;
        }
        else
        {
            switch (Type())
            {
            case EPropertyTypeNew::String:
            case EPropertyTypeNew::Asset:
            case EPropertyTypeNew::Animation:
            case EPropertyTypeNew::AnimationSet:
            case EPropertyTypeNew::Sequence:
            case EPropertyTypeNew::Spline:
            case EPropertyTypeNew::Guid:
                MakeOptional = true;
                break;
            }
        }

        // Branch here to avoid constructing a default value if we don't need to.
        if (MakeOptional)
            rArc << SerialParameter("DefaultValue", mDefaultValue, SH_Optional, pArchetype ? pArchetype->mDefaultValue : GetSerializationDefaultValue());
        else
            rArc << SerialParameter("DefaultValue", mDefaultValue);
    }

    virtual bool ShouldSerialize() const
    {
        TTypedPropertyNew* pArchetype = static_cast<TTypedPropertyNew*>(mpArchetype);

        return TTypedPropertyNew::ShouldSerialize() ||
                !(mDefaultValue == pArchetype->DefaultValue());
    }

    /** Return default value for serialization - can be customized per type */
    virtual PropType GetSerializationDefaultValue()
    {
        return PropType();
    }
};

template<typename PropType, EPropertyTypeNew PropEnum>
class TNumericalPropertyNew : public TSerializeableTypedProperty<PropType, PropEnum>
{
    friend class IPropertyNew;
    friend class CTemplateLoader;

protected:
    PropType mMinValue;
    PropType mMaxValue;

    TNumericalPropertyNew()
        : TSerializeableTypedProperty()
        , mMinValue( -1 )
        , mMaxValue( -1 )
    {}

public:
    virtual void Serialize(IArchive& rArc)
    {
        TSerializeableTypedProperty::Serialize(rArc);
        TNumericalPropertyNew* pArchetype = static_cast<TNumericalPropertyNew*>(mpArchetype);

        rArc << SerialParameter("Min", mMinValue, SH_Optional, pArchetype ? pArchetype->mMinValue : (PropType) -1)
             << SerialParameter("Max", mMaxValue, SH_Optional, pArchetype ? pArchetype->mMaxValue : (PropType) -1);
    }

    virtual bool ShouldSerialize() const
    {
        TNumericalPropertyNew* pArchetype = static_cast<TNumericalPropertyNew*>(mpArchetype);
        return TSerializeableTypedProperty::ShouldSerialize() ||
                mMinValue != pArchetype->mMinValue ||
                mMaxValue != pArchetype->mMaxValue;
    }

    virtual void InitFromArchetype(IPropertyNew* pOther)
    {
        TSerializeableTypedProperty::InitFromArchetype(pOther);
        TNumericalPropertyNew* pCastOther = static_cast<TNumericalPropertyNew*>(pOther);
        mMinValue = pCastOther->mMinValue;
        mMaxValue = pCastOther->mMaxValue;
    }

    virtual void PropertyValueChanged(void* pPropertyData)
    {
        TSerializeableTypedProperty::PropertyValueChanged(pPropertyData);

        if (mMinValue >= 0 && mMaxValue >= 0)
        {
            PropType& rValue = ValueRef(pPropertyData);
            rValue = Math::Clamp(mMinValue, mMaxValue, rValue);
        }
    }
};

/** Property casting with dynamic type checking */
template<class PropertyClass>
inline PropertyClass* TPropCast(IPropertyNew* pProperty)
{
    if (pProperty && pProperty->Type() == PropertyClass::StaticType())
    {
        return static_cast<PropertyClass*>(pProperty);
    }
    else
    {
        return nullptr;
    }
}

#endif // IPROPERTYNEW_H
