#ifndef CFLAGSPROPERTY_H
#define CFLAGSPROPERTY_H

#include "Core/Resource/Script/Property/IProperty.h"

class CFlagsProperty : public TSerializeableTypedProperty<uint32_t, EPropertyType::Flags>
{
    friend class IProperty;

    struct SBitFlag
    {
        TString Name;
        uint32_t Mask = 0;

        SBitFlag() = default;
        SBitFlag(TString InName, uint32_t InMask)
            : Name(std::move(InName)), Mask(InMask)
        {}

        bool operator==(const SBitFlag&) const = default;

        void Serialize(IArchive& rArc)
        {
            rArc << SerialParameter("Name", Name, SH_Attribute)
                 << SerialParameter("Mask", Mask, SH_Attribute | SH_HexDisplay);
        }
    };
    std::vector<SBitFlag> mBitFlags;
    uint32_t mAllFlags = 0;

    explicit CFlagsProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {}

public:
    uint32_t NumFlags() const
    {
        return mBitFlags.size();
    }

    TString FlagName(uint32_t Idx) const
    {
        ASSERT(Idx < mBitFlags.size());
        return mBitFlags[Idx].Name;
    }

    uint32_t FlagMask(uint32_t Idx) const
    {
        ASSERT(Idx < mBitFlags.size());
        return mBitFlags[Idx].Mask;
    }

    void Serialize(IArchive& rArc) override;
    void PostInitialize() override;
    void SerializeValue(void* pData, IArchive& rArc) override;
    void InitFromArchetype(IProperty* pOther) override;
    TString ValueAsString(const void* pData) const override;

    /**
     * Checks whether there are any unrecognized bits toggled on in the property value.
     * Returns the mask of any invalid bits. If all bits are valid, returns 0.
     */
    uint32_t HasValidValue(const void* pPropertyData) const;
};

#endif // CFLAGSPROPERTY_H
