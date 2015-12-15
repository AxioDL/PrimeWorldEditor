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
    mpObj = pObj;
    mGame = pObj->Template()->MasterTemplate()->GetGame();
}

bool CAreaAttributes::IsLayerEnabled()
{
    return mpObj->Layer()->IsActive();
}

bool CAreaAttributes::IsSkyEnabled()
{
    CPropertyStruct *pBaseStruct = mpObj->Properties();

    switch (mGame)
    {
    case ePrime:
    case eEchoes:
    case eCorruption:
        return static_cast<CBoolProperty*>(pBaseStruct->PropertyByIndex(1))->Get();
    default:
        return false;
    }
}

CModel* CAreaAttributes::SkyModel()
{
    CPropertyStruct *pBaseStruct = mpObj->Properties();

    switch (mGame)
    {
    case ePrime:
    case eEchoes:
        return (CModel*) static_cast<CFileProperty*>(pBaseStruct->PropertyByIndex(7))->Get().RawPointer();
    case eCorruption:
        return (CModel*) static_cast<CFileProperty*>(pBaseStruct->PropertyByIndex(8))->Get().RawPointer();
    default:
        return nullptr;
    }
}
