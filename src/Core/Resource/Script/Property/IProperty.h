#ifndef IPROPERTY_H
#define IPROPERTY_H

#include <Common/Common.h>
#include <Common/CFourCC.h>
#include <Common/Math/CVector3f.h>
#include <Common/Math/MathUtil.h>

#include <memory>

/** Forward declares */
class CGameTemplate;
class CScriptTemplate;
class CStructProperty;

/** Aliases */
using TIDString = TString;

/** Property flags */
enum class EPropertyFlag : uint32
{
    /** Property has been fully initialized and has had PostLoad called */
    IsInitialized          = 0x1,
    /** Property is an archetype (a template for other properties to copy from) */
    IsArchetype            = 0x2,
    /** Property is an array archetype (a template for elements of an array property) */
    IsArrayArchetype       = 0x4,
    /** This property and all its children are a single unit and do not have individual property IDs, sizes, etc. */
    IsAtomic               = 0x8,
    /** This is a property of a C++ class, not a script object */
    IsIntrinsic            = 0x10,
    /** Property has been modified, and needs to be resaved. Only valid on archetypes */
    IsDirty                = 0x20,
    /** We have cached whether the property name is correct */
    HasCachedNameCheck     = 0x40000000,
    /** The name of the property is a match for the property ID hash */
    HasCorrectPropertyName = 0x80000000,

    /** Flags that are left intact when copying from an archetype */
    ArchetypeCopyFlags     = IsAtomic,
    /** Flags that are inheritable from parent */
    InheritableFlags       = IsArchetype | IsArrayArchetype | IsAtomic,
};
DECLARE_FLAGS_ENUMCLASS(EPropertyFlag, FPropertyFlags)

/** Property type */
enum class EPropertyType
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

inline const char* PropEnumToHashableTypeName(EPropertyType Type)
{
    switch (Type)
    {
    // these names are required to generate accurate property ID hashes
    case EPropertyType::Bool:       return "bool";
    case EPropertyType::Int:        return "int";
    case EPropertyType::Float:      return "float";
    case EPropertyType::Choice:     return "choice";
    case EPropertyType::Enum:       return "enum";
    case EPropertyType::Flags:      return "Flags";
    case EPropertyType::String:     return "string";
    case EPropertyType::Vector:     return "Vector";
    case EPropertyType::Color:      return "Color";
    case EPropertyType::Asset:      return "asset";
    case EPropertyType::Sound:      return "sound";
    case EPropertyType::Spline:     return "spline";
    case EPropertyType::Guid:       return "guid";
    // unknown hashable types - used in hashes but these names are inaccurate
    case EPropertyType::Animation:  return "animation";
    case EPropertyType::Sequence:   return "sequence";
    // non hashable types - not used in ID hashes but still displayed on the UI
    case EPropertyType::Byte:       return "byte";
    case EPropertyType::Short:      return "short";
    case EPropertyType::Array:      return "array";
    // fallback
    default:                        return "";
    }
}

/** Enum that describes when/how properties should be cooked out */
enum class ECookPreference
{
    Default,
    Always,
    Never,
    OnlyIfModified
};

/** New property class */
class IProperty
{
protected:
    /** Flags */
    FPropertyFlags mFlags;

    /** Parent property */
    IProperty* mpParent = nullptr;

    /** Pointer parent; if non-null, this parent needs to be dereferenced to access the correct
     *  memory region that our property data is stored in */
    IProperty* mpPointerParent = nullptr;

    /** Archetype property; source property that we copied metadata from */
    IProperty* mpArchetype = nullptr;

    /** Sub-instances of archetype properties. For non-archetypes, will be empty.
     *  @todo this really oughta be a linked list */
    std::vector<IProperty*> mSubInstances;

    /** Child properties; these appear underneath this property on the UI */
    std::vector<IProperty*> mChildren;

    /** Game this property belongs to */
    EGame mGame;

    /** Script template that this property belongs to. Null for struct/enum/flag archetypes. */
    CScriptTemplate* mpScriptTemplate = nullptr;

    /** Offset of this property within the property block */
    uint32 mOffset = UINT32_MAX;

    /** Property ID. This ID is used to uniquely identify this property within this struct. */
    uint32 mID = UINT32_MAX;

    /** Property metadata */
    TString mName;
    TString mDescription;
    TString mSuffix;
    ECookPreference mCookPreference{ECookPreference::Default};

    /** Min/max allowed version number. These numbers correspond to the game's internal build number.
     *  This is not used yet but in the future it can be used to configure certain properties to only
     *  show up when certain versions of the game are being edited. The default values allow the
     *  property to show up in all versions. */
    float mMinVersion = 0.0f;
    float mMaxVersion = FLT_MAX;

    /** Private constructor - use static methods to instantiate */
    explicit IProperty(EGame Game);
    void _ClearChildren();

public:
    virtual ~IProperty();

    /** Interface */
    virtual EPropertyType Type() const = 0;
    virtual uint32 DataSize() const = 0;
    virtual uint32 DataAlignment() const = 0;
    virtual void Construct(void* pData) const = 0;
    virtual void Destruct(void* pData) const = 0;
    virtual bool MatchesDefault(void* pData) const = 0;
    virtual void RevertToDefault(void* pData) const = 0;
    virtual void SerializeValue(void* pData, IArchive& Arc) const = 0;

    virtual void PostInitialize() {}
    virtual void PropertyValueChanged(void* pPropertyData)      {}
    virtual void CopyDefaultValueTo(IProperty* pOtherProperty)  {}
    virtual void SetDefaultFromData(void* pData)                {}
    virtual bool IsNumericalType() const                    { return false; }
    virtual bool IsPointerType() const                      { return false; }
    virtual TString ValueAsString(void* pData) const        { return ""; }
    virtual const char* HashableTypeName() const;
    virtual void* GetChildDataPointer(void* pPropertyData) const;
    virtual void Serialize(IArchive& rArc);
    virtual void InitFromArchetype(IProperty* pOther);
    virtual bool ShouldSerialize() const;
    
    /** Utility methods */
    void Initialize(IProperty* pInParent, CScriptTemplate* pInTemplate, uint32 InOffset);
    void* RawValuePtr(void* pData) const;
    IProperty* ChildByID(uint32 ID) const;
    IProperty* ChildByIDString(const TIDString& rkIdString);
    void GatherAllSubInstances(std::list<IProperty*>& OutList, bool Recursive);
    TString GetTemplateFileName();
    bool ShouldCook(void* pPropertyData) const;
    void SetName(const TString& rkNewName);
    void SetDescription(const TString& rkNewDescription);
    void SetSuffix(const TString& rkNewSuffix);
    void MarkDirty();
    void ClearDirtyFlag();
    bool ConvertType(EPropertyType NewType, IProperty* pNewArchetype = nullptr);
    bool UsesNameMap() const;
    bool HasAccurateName();

    /** Accessors */
    EGame Game() const;
    ECookPreference CookPreference() const { return mCookPreference; }
    size_t NumChildren() const { return mChildren.size(); }

    IProperty* ChildByIndex(size_t ChildIndex) const
    {
        ASSERT(ChildIndex < mChildren.size());
        return mChildren[ChildIndex];
    }

    IProperty* Parent() const { return mpParent; }
    IProperty* RootParent()
    {
        IProperty* pParent = Parent();
        IProperty* pOut = this;

        while (pParent)
        {
            pOut = pParent;
            pParent = pParent->Parent();
        }

        return pOut;
    }

    IProperty* Archetype() const { return mpArchetype; }
    IProperty* RootArchetype()
    {
        IProperty* pArchetype = Archetype();
        IProperty* pOut = this;

        while (pArchetype)
        {
            pOut = pArchetype;
            pArchetype = pArchetype->Archetype();
        }

        return pOut;
    }

    CScriptTemplate* ScriptTemplate() const { return mpScriptTemplate; }
    TString Name() const { return mName; }
    TString Description() const { return mDescription; }
    TString Suffix() const { return mSuffix; }

    TIDString IDString(bool FullyQualified) const
    {
        if (FullyQualified && mpParent != nullptr && mpParent->Parent() != nullptr)
            return mpParent->IDString(FullyQualified) + ":" + TString::HexString(mID);
        else
            return TString::HexString(mID);
    }

    uint32 Offset() const { return mOffset; }
    uint32 ID() const { return mID; }

    bool IsInitialized() const       { return mFlags.HasFlag(EPropertyFlag::IsInitialized); }
    bool IsArchetype() const         { return mFlags.HasFlag(EPropertyFlag::IsArchetype); }
    bool IsArrayArchetype() const    { return mFlags.HasFlag(EPropertyFlag::IsArrayArchetype); }
    bool IsAtomic() const            { return mFlags.HasFlag(EPropertyFlag::IsAtomic); }
    bool IsIntrinsic() const         { return mFlags.HasFlag(EPropertyFlag::IsIntrinsic); }
    bool IsDirty() const             { return mFlags.HasFlag(EPropertyFlag::IsDirty); }
    bool IsRootParent() const        { return mpParent == nullptr; }
    bool IsRootArchetype() const     { return mpArchetype == nullptr; }

    /** Create */
    static IProperty* Create(EPropertyType Type,
                                EGame Game);

    static IProperty* CreateCopy(IProperty* pArchetype);

    static IProperty* CreateIntrinsic(EPropertyType Type,
                                      EGame Game,
                                      uint32 Offset,
                                      const TString& rkName);

    static IProperty* CreateIntrinsic(EPropertyType Type,
                                      IProperty* pParent,
                                      uint32 Offset,
                                      const TString& rkName);

    static IProperty* ArchiveConstructor(EPropertyType Type,
                                         const IArchive& Arc);
};

template<typename PropType, EPropertyType PropEnum>
class TTypedProperty : public IProperty
{
    friend class IProperty;
    friend class CTemplateLoader;
public:
    using ValueType = PropType;
    
protected:
    PropType mDefaultValue = {};

    explicit TTypedProperty(EGame Game) : IProperty(Game) {}

public:
    EPropertyType Type() const override              { return PropEnum; }
    uint32 DataSize() const override                 { return sizeof(PropType); }
    uint32 DataAlignment() const override            { return alignof(PropType); }
    void Construct(void* pData) const override       { new (ValuePtr(pData)) PropType(mDefaultValue); }
    void Destruct(void* pData) const override        { ValueRef(pData).~PropType(); }
    bool MatchesDefault(void* pData) const override  { return ValueRef(pData) == mDefaultValue; }
    void RevertToDefault(void* pData) const override { ValueRef(pData) = mDefaultValue; }
    void SetDefaultFromData(void* pData) override
    {
        mDefaultValue = ValueRef(pData);
        MarkDirty();
    }

    virtual bool CanHaveDefault() const { return true; }

    void InitFromArchetype(IProperty* pOther) override
    {
        IProperty::InitFromArchetype(pOther);
        mDefaultValue = static_cast<TTypedProperty*>(pOther)->mDefaultValue;
    }

    void CopyDefaultValueTo(IProperty* pOtherProperty) override
    {
        // WARNING: We don't do any type checking here because this function is used for type conversion,
        // which necessitates that the property class is allowed to be different. The underlying type is
        // assumed to be the same. It is the caller's responsibility to ensure this function is not called
        // with incompatible property types.
        TTypedProperty* pTypedOther = static_cast<TTypedProperty*>(pOtherProperty);
        pTypedOther->mDefaultValue = mDefaultValue;
    }

    PropType* ValuePtr(void* pData) const
    {
        return (PropType*) RawValuePtr(pData);
    }

    PropType& ValueRef(void* pData) const
    {
        return *ValuePtr(pData);
    }

    PropType Value(void* pData) const
    {
        return *ValuePtr(pData);
    }

    const PropType& DefaultValue() const
    {
        return mDefaultValue;
    }

    void SetDefaultValue(const PropType& kInDefaultValue)
    {
        mDefaultValue = kInDefaultValue;
    }

    static constexpr EPropertyType StaticType()     { return PropEnum; }
};

template<typename PropType, EPropertyType PropEnum>
class TSerializeableTypedProperty : public TTypedProperty<PropType, PropEnum>
{
  using base = TTypedProperty<PropType, PropEnum>;
protected:
    explicit TSerializeableTypedProperty(EGame Game)
        : base(Game)
    {}

public:
    void Serialize(IArchive& rArc) override
    {
        base::Serialize(rArc);
        TSerializeableTypedProperty* pArchetype = static_cast<TSerializeableTypedProperty*>(base::mpArchetype);

        // Determine if default value should be serialized as optional.
        // All MP1 properties should be optional. For MP2 and on, we set optional
        // on property types that don't have default values in the game executable.
        bool MakeOptional = false;

        if (base::Game() <= EGame::Prime || pArchetype != nullptr)
        {
            MakeOptional = true;
        }
        else
        {
            switch (base::Type())
            {
            case EPropertyType::String:
            case EPropertyType::Asset:
            case EPropertyType::Animation:
            case EPropertyType::AnimationSet:
            case EPropertyType::Sequence:
            case EPropertyType::Spline:
            case EPropertyType::Guid:
                MakeOptional = true;
                break;
            default:
                break;
            }
        }

        // Branch here to avoid constructing a default value if we don't need to.
        if (MakeOptional)
            rArc << SerialParameter("DefaultValue", base::mDefaultValue, SH_Optional,
                    pArchetype ? pArchetype->mDefaultValue : GetSerializationDefaultValue());
        else
            rArc << SerialParameter("DefaultValue", base::mDefaultValue);
    }

    bool ShouldSerialize() const override
    {
        base* pArchetype = static_cast<base*>(base::mpArchetype);

        return base::ShouldSerialize() ||
                !(base::mDefaultValue == pArchetype->DefaultValue());
    }

    /** Return default value for serialization - can be customized per type */
    virtual PropType GetSerializationDefaultValue()
    {
        return PropType();
    }
};

template<typename PropType, EPropertyType PropEnum>
class TNumericalProperty : public TSerializeableTypedProperty<PropType, PropEnum>
{
    using base = TSerializeableTypedProperty<PropType, PropEnum>;
    friend class IProperty;
    friend class CTemplateLoader;

protected:
    PropType mMinValue = -1;
    PropType mMaxValue = -1;

    explicit TNumericalProperty(EGame Game)
        : base(Game)
    {}

public:
    void Serialize(IArchive& rArc) override
    {
        base::Serialize(rArc);
        TNumericalProperty* pArchetype = static_cast<TNumericalProperty*>(base::mpArchetype);

        rArc << SerialParameter("Min", mMinValue, SH_Optional, pArchetype ? pArchetype->mMinValue : (PropType) -1)
             << SerialParameter("Max", mMaxValue, SH_Optional, pArchetype ? pArchetype->mMaxValue : (PropType) -1);
    }

    bool ShouldSerialize() const override
    {
        TNumericalProperty* pArchetype = static_cast<TNumericalProperty*>(base::mpArchetype);
        return base::ShouldSerialize() ||
                mMinValue != pArchetype->mMinValue ||
                mMaxValue != pArchetype->mMaxValue;
    }

    void InitFromArchetype(IProperty* pOther) override
    {
        base::InitFromArchetype(pOther);
        TNumericalProperty* pCastOther = static_cast<TNumericalProperty*>(pOther);
        mMinValue = pCastOther->mMinValue;
        mMaxValue = pCastOther->mMaxValue;
    }

    void PropertyValueChanged(void* pPropertyData) override
    {
        base::PropertyValueChanged(pPropertyData);

        if (mMinValue >= 0 && mMaxValue >= 0)
        {
            PropType& rValue = base::ValueRef(pPropertyData);
            rValue = Math::Clamp(mMinValue, mMaxValue, rValue);
        }
    }
};

/** Property casting with dynamic type checking */
template<class PropertyClass>
PropertyClass* TPropCast(IProperty* pProperty)
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

#endif // IPROPERTY_H
