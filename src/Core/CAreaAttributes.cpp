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
    case eEchoesDemo:
    case eEchoes:
    case eCorruptionProto:
    case eCorruption:
        return static_cast<TBoolProperty*>(pBaseStruct->PropertyByIndex(1))->Get();
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
        return (CModel*) static_cast<TFileProperty*>(pBaseStruct->PropertyByIndex(7))->Get().RawPointer();
    case eEchoesDemo:
    case eEchoes:
    case eCorruptionProto:
    case eCorruption:
        return (CModel*) static_cast<TFileProperty*>(pBaseStruct->PropertyByID(0xD208C9FA))->Get().RawPointer();
    default:
        return nullptr;
    }
}
