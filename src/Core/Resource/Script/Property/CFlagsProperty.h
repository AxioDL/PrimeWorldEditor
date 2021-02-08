#ifndef CFLAGSPROPERTY_H
#define CFLAGSPROPERTY_H

#include "IProperty.h"

class CFlagsProperty : public TSerializeableTypedProperty<uint32, EPropertyType::Flags>
{
    friend class IProperty;

    struct SBitFlag
    {
        TString Name;
        uint32 Mask = 0;

        SBitFlag() = default;
        SBitFlag(const TString& rkInName, uint32 InMask)
            : Name(rkInName), Mask(InMask)
        {}

        bool operator==(const SBitFlag& other) const
        {
            return Name == other.Name && Mask == other.Mask;
        }

        bool operator!=(const SBitFlag& other) const
        {
            return !operator==(other);
        }

        void Serialize(IArchive& rArc)
        {
            rArc << SerialParameter("Name", Name, SH_Attribute)
                 << SerialParameter("Mask", Mask, SH_Attribute | SH_HexDisplay);
        }
    };
    std::vector<SBitFlag> mBitFlags;
    uint32 mAllFlags = 0;

    explicit CFlagsProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {}

public:
    uint32 NumFlags() const
    {
        return mBitFlags.size();
    }

    TString FlagName(uint32 Idx) const
    {
        ASSERT(Idx >= 0 && Idx < mBitFlags.size());
        return mBitFlags[Idx].Name;
    }

    uint32 FlagMask(uint32 Idx) const
    {
        ASSERT(Idx >= 0 && Idx < mBitFlags.size());
        return mBitFlags[Idx].Mask;
    }

    void Serialize(IArchive& rArc) override;
    void PostInitialize() override;
    void SerializeValue(void* pData, IArchive& rArc) const override;
    void InitFromArchetype(IProperty* pOther) override;
    TString ValueAsString(void* pData) const override;

    /**
     * Checks whether there are any unrecognized bits toggled on in the property value.
     * Returns the mask of any invalid bits. If all bits are valid, returns 0.
     */
    uint32 HasValidValue(void* pPropertyData) const;
};

#endif // CFLAGSPROPERTY_H
