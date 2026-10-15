#ifndef CANIMATIONPROPERTY_H
#define CANIMATIONPROPERTY_H

#include "Core/Resource/Script/Property/IProperty.h"
#include <fmt/format.h>

class CAnimationProperty : public TSerializeableTypedProperty<uint32_t, EPropertyType::Animation>
{
    friend class IProperty;

protected:
    explicit CAnimationProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {}

public:
    void SerializeValue(void* pData, IArchive& rArc) override
    {
        rArc.SerializePrimitive(ValueRef(pData), SH_HexDisplay);
    }

    TString ValueAsString(const void* pData) const override
    {
        // Animations in DKCR are an ID hash, in every other game they're an index
        if (Game() != EGame::DKCReturns)
        {
            if (Value(pData) == UINT32_MAX)
                return "-1";
            return fmt::format("{:d}", Value(pData));
        }
        else
        {
            return fmt::format("0x{:08X}", Value(pData));
        }
    }

    const char* HashableTypeName() const override
    {
        if (Game() < EGame::CorruptionProto)
            return "animation";
        else
            return "characteranimation";
    }
};

#endif // CANIMATIONPROPERTY_H
