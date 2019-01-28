#ifndef CTWEAKDATA_H
#define CTWEAKDATA_H

#include "Core/Resource/CResource.h"
#include "Core/Resource/Script/CScriptTemplate.h"
#include "Core/Resource/Script/Property/TPropertyRef.h"

/** Tweak data assets for MP1 */
class CTweakData : public CResource
{
    DECLARE_RESOURCE_TYPE(Tweaks)

    /** Script template specifying tweak data layout */
    CScriptTemplate* mpTemplate;

    /** Tweak ID for MP2+ */
    uint mTweakID;

    /** Tweak data */
    std::vector<uint8> mTweakData;

public:
    CTweakData(CScriptTemplate* pTemplate, uint TweakID, CResourceEntry* pEntry = 0)
        : mpTemplate(pTemplate)
        , mTweakID(TweakID)
        , CResource(pEntry)
    {
        CStructProperty* pProperties = pTemplate->Properties();
        mTweakData.resize(pProperties->DataSize());
        pProperties->Construct(mTweakData.data());
    }

    TString TweakName()
    {
        if (Entry() != nullptr)
        {
            return Entry()->Name();
        }
        else
        {
            IProperty* pNameProperty = mpTemplate->Properties()->ChildByID(0x7FDA1466);
            return CStringRef(mTweakData.data(), pNameProperty);
        }
    }

    inline CScriptTemplate* TweakTemplate() const
    {
        return mpTemplate;
    }

    inline uint32 TweakID() const
    {
        return mTweakID;
    }

    inline CStructRef TweakData() const
    {
        return CStructRef((void*) mTweakData.data(), mpTemplate->Properties());
    }
};

#endif // CTWEAKDATA_H
