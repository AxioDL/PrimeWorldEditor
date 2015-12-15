#include "CScriptObject.h"
#include "CMasterTemplate.h"
#include "Core/Resource/CAnimSet.h"

CScriptObject::CScriptObject(CGameArea *pArea, CScriptLayer *pLayer, CScriptTemplate *pTemplate)
{
    mpTemplate = pTemplate;
    mpArea = pArea;
    mpLayer = pLayer;
    mpProperties = nullptr;
    mpTemplate->AddObject(this);
    mpDisplayModel = nullptr;
    mpBillboard = nullptr;
    mpCollision = nullptr;
    mHasInGameModel = false;
}

CScriptObject::~CScriptObject()
{
    if (mpProperties) delete mpProperties;
    mpTemplate->RemoveObject(this);
}

// ************ DATA MANIPULATION ************
void CScriptObject::CopyFromTemplate(CScriptTemplate *pTemp, u32 propCount)
{
    CStructTemplate *pBaseStruct = pTemp->BaseStructByCount(propCount);
    delete mpProperties;
    mpProperties = CPropertyStruct::CopyFromTemplate(pBaseStruct);
}

void CScriptObject::EvaluateProperties()
{
    mpInstanceName = mpTemplate->FindInstanceName(mpProperties);
    mpPosition = mpTemplate->FindPosition(mpProperties);
    mpRotation = mpTemplate->FindRotation(mpProperties);
    mpScale = mpTemplate->FindScale(mpProperties);
    mpActive = mpTemplate->FindActive(mpProperties);
    mpLightParameters = mpTemplate->FindLightParameters(mpProperties);
    mHasInGameModel = mpTemplate->HasInGameModel(mpProperties);
    mVolumeShape = mpTemplate->VolumeShape(this);
    EvaluateDisplayModel();
    EvaluateBillboard();
    EvaluateCollisionModel();
}

void CScriptObject::EvaluateDisplayModel()
{
    mpDisplayModel = mpTemplate->FindDisplayModel(mpProperties);
}

void CScriptObject::EvaluateBillboard()
{
    mpBillboard = mpTemplate->FindBillboardTexture(mpProperties);
}

void CScriptObject::EvaluateCollisionModel()
{
    mpCollision = mpTemplate->FindCollision(mpProperties);
}

// ************ GETTERS ************
CPropertyBase* CScriptObject::PropertyByIndex(u32 index) const
{
    return mpProperties->PropertyByIndex(index);
}

CPropertyBase* CScriptObject::PropertyByIDString(const TString& str) const
{
    return mpProperties->PropertyByIDString(str);
}

CScriptTemplate* CScriptObject::Template() const
{
    return mpTemplate;
}

CMasterTemplate* CScriptObject::MasterTemplate() const
{
    return mpTemplate->MasterTemplate();
}

CGameArea* CScriptObject::Area() const
{
    return mpArea;
}

CScriptLayer* CScriptObject::Layer() const
{
    return mpLayer;
}

CPropertyStruct* CScriptObject::Properties() const
{
    return mpProperties;
}

u32 CScriptObject::NumProperties() const
{
    return mpProperties->Count();
}

u32 CScriptObject::ObjectTypeID() const
{
    return mpTemplate->ObjectID();
}

u32 CScriptObject::InstanceID() const
{
    return mInstanceID;
}

u32 CScriptObject::NumInLinks() const
{
    return mInConnections.size();
}

u32 CScriptObject::NumOutLinks() const
{
    return mOutConnections.size();
}

const SLink& CScriptObject::InLink(u32 index) const
{
    return mInConnections[index];
}

const SLink& CScriptObject::OutLink(u32 index) const
{
    return mOutConnections[index];
}

TString CScriptObject::InstanceName() const
{
    if (mpInstanceName)
        return mpInstanceName->Get();
    else
        return "";
}

CVector3f CScriptObject::Position() const
{
    if (mpPosition)
        return mpPosition->Get();
    else
        return CVector3f::skZero;
}

CVector3f CScriptObject::Rotation() const
{
    if (mpRotation)
        return mpRotation->Get();
    else
        return CVector3f::skZero;
}

CVector3f CScriptObject::Scale() const
{
    if (mpScale)
        return mpScale->Get();
    else
        return CVector3f::skOne;
}

bool CScriptObject::IsActive() const
{
    if (mpActive)
        return mpActive->Get();
    else
        return false;
}

bool CScriptObject::HasInGameModel() const
{
    return mHasInGameModel;
}

void CScriptObject::SetPosition(const CVector3f& newPos)
{
    if (mpPosition) mpPosition->Set(newPos);
}

void CScriptObject::SetRotation(const CVector3f& newRot)
{
    if (mpRotation) mpRotation->Set(newRot);
}

void CScriptObject::SetScale(const CVector3f& newScale)
{
    if (mpScale) mpScale->Set(newScale);
}

void CScriptObject::SetName(const TString& newName)
{
    if (mpInstanceName) mpInstanceName->Set(newName);
}

void CScriptObject::SetActive(bool isActive)
{
    if (mpActive) mpActive->Set(isActive);
}

CPropertyStruct* CScriptObject::LightParameters() const
{
    return mpLightParameters;
}

CModel* CScriptObject::GetDisplayModel() const
{
    return mpDisplayModel;
}

CTexture* CScriptObject::GetBillboard() const
{
    return mpBillboard;
}

CCollisionMeshGroup* CScriptObject::GetCollision() const
{
    return mpCollision;
}

EVolumeShape CScriptObject::VolumeShape() const
{
    return mVolumeShape;
}
