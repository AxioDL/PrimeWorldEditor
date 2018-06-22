#include "CAreaAttributes.h"
#include "Core/Resource/Script/CMasterTemplate.h"
#include "Core/Resource/Script/CScriptLayer.h"

CAreaAttributes::CAreaAttributes(CScriptObject *pObj)
{
    SetObject(pObj);
}

CAreaAttributes::~CAreaAttributes()
{
}

void CAreaAttributes::SetObject(CScriptObject *pObj)
{
    CScriptTemplate* pTemplate = pObj->Template();
    CStructPropertyNew* pProperties = pTemplate->Properties();

    mpObject = pObj;
    mGame = pTemplate->MasterTemplate()->Game();
    mNeedSky = CBoolRef(pObj, pProperties->ChildByIndex(1));

    if (mGame == ePrime)
        mOverrideSky = CAssetRef(pObj, pProperties->ChildByIndex(7));
    else if (mGame > ePrime)
        mOverrideSky = CAssetRef(pObj, pProperties->ChildByID(0xD208C9FA));
}

bool CAreaAttributes::IsLayerEnabled() const
{
    return mpObject->Layer()->IsActive();
}

bool CAreaAttributes::IsSkyEnabled() const
{
    return mNeedSky.IsValid() ? mNeedSky.Get() : false;
}

CModel* CAreaAttributes::SkyModel() const
{
    return mOverrideSky.IsValid() ? gpResourceStore->LoadResource<CModel>(mOverrideSky.Get()) : nullptr;
}
