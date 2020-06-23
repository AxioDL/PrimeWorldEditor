#ifndef CENUMPROPERTY_H
#define CENUMPROPERTY_H

#include "IProperty.h"

/** There are two types of enum properties in the game data: enum and choice.
 *
 *  In the game, the difference is that choice properties are index-based, while
 *  enum properties are stored as a hash of the name of the enum value.
 *
 *  In PWE, however, they are both implemented the same way under the hood.
 */
template<EPropertyType TypeEnum>
class TEnumPropertyBase : public TSerializeableTypedProperty<int32, TypeEnum>
{
    using base = TSerializeableTypedProperty<int32, TypeEnum>;
    friend class IProperty;

    struct SEnumValue
    {
        TString Name;
        uint32 ID = 0;

        SEnumValue() = default;
        SEnumValue(TString rkInName, uint32 InID)
            : Name(std::move(rkInName)), ID(InID) {}


        bool operator==(const SEnumValue& other) const
        {
            return Name == other.Name && ID == other.ID;
        }

        bool operator!=(const SEnumValue& other) const
        {
            return !operator==(other);
        }

        void Serialize(IArchive& rArc)
        {
            rArc << SerialParameter("Name", Name, SH_Attribute)
                 << SerialParameter("ID", ID, SH_Attribute | SH_HexDisplay);
        }
    };
    std::vector<SEnumValue> mValues;

    /** If true, the archetype's name will be used as the type name instead of "enum" or "choice". */
    bool mOverrideTypeName;

protected:
    /** Constructor */
    explicit TEnumPropertyBase(EGame Game)
        : base(Game)
        , mOverrideTypeName(false)
    {}

public:
    const char* HashableTypeName() const override
    {
        if (base::mpArchetype)
            return base::mpArchetype->HashableTypeName();
        else if (mOverrideTypeName)
            return *base::mName;
        else if (TypeEnum == EPropertyType::Enum)
            return "enum";
        else
            return "choice";
    }

    void Serialize(IArchive& rArc) override
    {
        // Skip TSerializeableTypedProperty, serialize default value ourselves so we can set SH_HexDisplay
        TTypedProperty<int32, TypeEnum>::Serialize(rArc);

        // Serialize default value
        TEnumPropertyBase* pArchetype = static_cast<TEnumPropertyBase*>(base::mpArchetype);
        uint32 DefaultValueFlags = SH_Optional | (TypeEnum == EPropertyType::Enum ? SH_HexDisplay : 0);
        rArc << SerialParameter("DefaultValue", base::mDefaultValue, DefaultValueFlags, pArchetype ? pArchetype->mDefaultValue : 0);

        // Only serialize type name override for root archetypes.
        if (!base::mpArchetype)
        {
            rArc << SerialParameter("OverrideTypeName", mOverrideTypeName, SH_Optional, false);
        }

        if (!pArchetype || !rArc.CanSkipParameters() || mValues != pArchetype->mValues)
        {
            rArc << SerialParameter("Values", mValues);
        }
    }

    void SerializeValue(void* pData, IArchive& Arc) const override
    {
        Arc.SerializePrimitive((uint32&)base::ValueRef(pData), 0);
    }

    void InitFromArchetype(IProperty* pOther) override
    {
        TTypedProperty<int32, TypeEnum>::InitFromArchetype(pOther);
        TEnumPropertyBase* pOtherEnum = static_cast<TEnumPropertyBase*>(pOther);
        mValues = pOtherEnum->mValues;
    }

    TString ValueAsString(void* pData) const override
    {
        return TString::FromInt32(base::Value(pData), 0, 10);
    }

    void AddValue(TString ValueName, uint32 ValueID)
    {
        mValues.emplace_back(std::move(ValueName), ValueID);
    }

    size_t NumPossibleValues() const { return mValues.size(); }

    uint32 ValueIndex(uint32 ID) const
    {
        for (uint32 ValueIdx = 0; ValueIdx < mValues.size(); ValueIdx++)
        {
            if (mValues[ValueIdx].ID == ID)
            {
                return ValueIdx;
            }
        }
        return UINT32_MAX;
    }

    uint32 ValueID(size_t Index) const
    {
        ASSERT(Index < mValues.size());
        return mValues[Index].ID;
    }

    TString ValueName(size_t Index) const
    {
        ASSERT(Index < mValues.size());
        return mValues[Index].Name;
    }

    bool HasValidValue(void* pPropertyData)
    {
        if (mValues.empty())
            return true;

        const int ID = base::ValueRef(pPropertyData);
        const uint32 Index = ValueIndex(ID);
        return Index < mValues.size();
    }

    bool OverridesTypeName() const
    {
        return base::mpArchetype ? TPropCast<TEnumPropertyBase>(base::mpArchetype)->OverridesTypeName() : mOverrideTypeName;
    }

    void SetOverrideTypeName(bool Override)
    {
        if (base::mpArchetype)
        {
            TEnumPropertyBase* pArchetype = TPropCast<TEnumPropertyBase>(base::RootArchetype());
            pArchetype->SetOverrideTypeName(Override);
        }
        else
        {
            if (mOverrideTypeName != Override)
            {
                mOverrideTypeName = Override;
                base::MarkDirty();
            }
        }
    }
};

using CChoiceProperty = TEnumPropertyBase<EPropertyType::Choice>;
using CEnumProperty = TEnumPropertyBase<EPropertyType::Enum>;

// Specialization of TPropCast to allow interchangeable casting, as both types are the same thing
template<>
inline CEnumProperty* TPropCast(IProperty* pProperty)
{
    if (pProperty)
    {
        EPropertyType InType = pProperty->Type();

        if (InType == EPropertyType::Enum || InType == EPropertyType::Choice)
        {
            return static_cast<CEnumProperty*>(pProperty);
        }
    }

    return nullptr;
}

template<>
inline CChoiceProperty* TPropCast(IProperty* pProperty)
{
    if (pProperty)
    {
        EPropertyType InType = pProperty->Type();

        if (InType == EPropertyType::Enum || InType == EPropertyType::Choice)
        {
            return static_cast<CChoiceProperty*>(pProperty);
        }
    }

    return nullptr;
}

#endif // CENUMPROPERTY_H
