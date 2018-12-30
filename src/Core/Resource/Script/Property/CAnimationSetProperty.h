#ifndef CANIMATIONSETPROPERTY_H
#define CANIMATIONSETPROPERTY_H

#include "IProperty.h"

class CAnimationSetProperty : public TSerializeableTypedProperty< CAnimationParameters, EPropertyType::AnimationSet >
{
    friend class IProperty;

protected:
    CAnimationSetProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {
        mDefaultValue.SetGame(Game);
    }

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        ValueRef(pData).Serialize(Arc);
    }

    virtual const char* HashableTypeName() const
    {
        return (Game() <= EGame::Echoes ? "AnimationSet" : "CharacterAnimationSet");
    }

    virtual CAnimationParameters GetSerializationDefaultValue()
    {
        return CAnimationParameters( Game() );
    }
};

#endif // CANIMATIONSETPROPERTY_H
