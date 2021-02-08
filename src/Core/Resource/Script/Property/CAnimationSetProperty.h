#ifndef CANIMATIONSETPROPERTY_H
#define CANIMATIONSETPROPERTY_H

#include "IProperty.h"
#include "Core/Resource/Animation/CAnimationParameters.h"

class CAnimationSetProperty : public TSerializeableTypedProperty< CAnimationParameters, EPropertyType::AnimationSet >
{
    friend class IProperty;

protected:
    explicit CAnimationSetProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {
        mDefaultValue.SetGame(Game);
    }

public:
    void SerializeValue(void* pData, IArchive& Arc) const override
    {
        ValueRef(pData).Serialize(Arc);
    }

    const char* HashableTypeName() const override
    {
        return Game() <= EGame::Echoes ? "AnimationSet" : "CharacterAnimationSet";
    }

    CAnimationParameters GetSerializationDefaultValue() override
    {
        return CAnimationParameters(Game());
    }
};

#endif // CANIMATIONSETPROPERTY_H
