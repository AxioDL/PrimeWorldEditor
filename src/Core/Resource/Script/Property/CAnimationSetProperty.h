#ifndef CANIMATIONSETPROPERTY_H
#define CANIMATIONSETPROPERTY_H

#include "../IPropertyNew.h"

class CAnimationSetProperty : public TSerializeableTypedProperty< CAnimationParameters, EPropertyTypeNew::AnimationSet >
{
    friend class IPropertyNew;

protected:
    CAnimationSetProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {
        mDefaultValue.SetGame(Game);
    }

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Value(pData).Serialize(Arc);
    }

    virtual const char* HashableTypeName() const
    {
        return (Game() <= eEchoes ? "AnimationSet" : "CharacterAnimationSet");
    }

    virtual CAnimationParameters GetSerializationDefaultValue()
    {
        return CAnimationParameters( Game() );
    }
};

#endif // CANIMATIONSETPROPERTY_H
