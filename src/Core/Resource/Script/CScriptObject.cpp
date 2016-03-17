#include "CScriptObject.h"
#include "CScriptLayer.h"
#include "CMasterTemplate.h"
#include "Core/Resource/CAnimSet.h"

CScriptObject::CScriptObject(u32 InstanceID, CGameArea *pArea, CScriptLayer *pLayer, CScriptTemplate *pTemplate)
    : mpTemplate(pTemplate)
    , mpArea(pArea)
    , mpLayer(pLayer)
    , mVersion(0)
    , mInstanceID(InstanceID)
    , mpDisplayModel(nullptr)
    , mpCollision(nullptr)
    , mHasInGameModel(false)
    , mIsCheckingNearVisibleActivation(false)
{
    mpTemplate->AddObject(this);
    mpProperties = (CPropertyStruct*) pTemplate->BaseStruct()->InstantiateProperty(this, nullptr);
}

CScriptObject::~CScriptObject()
{
    if (mpProperties) delete mpProperties;
    mpTemplate->RemoveObject(this);

    // Note: Incoming links will be deleted by the sender.
    for (u32 iLink = 0; iLink < mOutLinks.size(); iLink++)
        delete mOutLinks[iLink];
}

// ************ DATA MANIPULATION ************
 void CScriptObject::EvaluateProperties()
{
    mpInstanceName = mpTemplate->FindInstanceName(mpProperties);
    mpPosition = mpTemplate->FindPosition(mpProperties);
    mpRotation = mpTemplate->FindRotation(mpProperties);
    mpScale = mpTemplate->FindScale(mpProperties);
    mpActive = mpTemplate->FindActive(mpProperties);
    mpLightParameters = mpTemplate->FindLightParameters(mpProperties);
    mHasInGameModel = mpTemplate->HasInGameModel(mpProperties);
    EvaluateDisplayModel();
    EvaluateBillboard();
    EvaluateCollisionModel();
    EvaluateVolume();
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

void CScriptObject::EvaluateVolume()
{
    mVolumeShape = mpTemplate->VolumeShape(this);
    mVolumeScale = mpTemplate->VolumeScale(this);
}

bool CScriptObject::IsEditorProperty(IProperty *pProp)
{
    return ( (pProp == mpInstanceName) ||
             (pProp == mpPosition) ||
             (pProp == mpRotation) ||
             (pProp == mpScale) ||
             (pProp == mpActive) ||
             (pProp->Parent() == mpLightParameters)
           );
}

void CScriptObject::SetLayer(CScriptLayer *pLayer, u32 NewLayerIndex)
{
    if (pLayer != mpLayer)
    {
        if (mpLayer) mpLayer->RemoveInstance(this);
        mpLayer = pLayer;
        if (mpLayer) mpLayer->AddInstance(this, NewLayerIndex);
    }
}

u32 CScriptObject::LayerIndex() const
{
    if (!mpLayer) return -1;

    for (u32 iInst = 0; iInst < mpLayer->NumInstances(); iInst++)
    {
        if (mpLayer->InstanceByIndex(iInst) == this)
            return iInst;
    }

    return -1;
}

bool CScriptObject::HasNearVisibleActivation() const
{
    /* This function is used to check whether an inactive DKCR object should render in game mode. DKCR deactivates a lot of
     * decorative actors when the player isn't close to them as an optimization. This means a lot of them are inactive by
     * default but should render in game mode anyway. To get around this, we'll check the links to find out whether this
     * instance has a "Near Visible" activation, which is typically done via a trigger that activates the object on
     * InternalState04/05/06 (usually through a relay). */
    std::list<CScriptObject*> Relays;
    bool IsRelay = (ObjectTypeID() == 0x53524C59);

    if (mIsCheckingNearVisibleActivation) return false;
    mIsCheckingNearVisibleActivation = true;

    for (u32 iLink = 0; iLink < mInLinks.size(); iLink++)
    {
        CLink *pLink = mInLinks[iLink];

        // Check for trigger activation
        if (pLink->State() == 0x49533034 || pLink->State() == 0x49533035 || pLink->State() == 0x49533036) // "IS04", "IS05", or "IS06"
        {
            if ( (!IsRelay && pLink->Message() == 0x41435456) || // "ACTV"
                 (IsRelay  && pLink->Message() == 0x4143544E) )  // "ACTN"
            {
                CScriptObject *pObj = pLink->Sender();

                if (pObj->ObjectTypeID() == 0x54524752) // "TRGR"
                {
                    mIsCheckingNearVisibleActivation = false;
                    return true;
                }
            }
        }

        // Check for relay activation
        else if (pLink->State() == 0x524C4159) // "RLAY"
        {
            if ( (!IsRelay && pLink->Message() == 0x41435456) || // "ACTV"
                 (IsRelay  && pLink->Message() == 0x4143544E) )  // "ACTN"
            {
                CScriptObject *pObj = pLink->Sender();

                if (pObj->ObjectTypeID() == 0x53524C59) // "SRLY"
                    Relays.push_back(pObj);
            }
        }
    }

    // Check whether any of the relays have a near visible activation
    for (auto it = Relays.begin(); it != Relays.end(); it++)
    {
        if ((*it)->HasNearVisibleActivation())
        {
            mIsCheckingNearVisibleActivation = false;
            return true;
        }
    }

    mIsCheckingNearVisibleActivation = false;
    return false;
}

// ************ GETTERS ************
IProperty* CScriptObject::PropertyByIndex(u32 index) const
{
    return mpProperties->PropertyByIndex(index);
}

IProperty* CScriptObject::PropertyByIDString(const TString& str) const
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

u32 CScriptObject::Version() const
{
    return mVersion;
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

u32 CScriptObject::NumLinks(ELinkType Type) const
{
    return (Type == eIncoming ? mInLinks.size() : mOutLinks.size());
}

CLink* CScriptObject::Link(ELinkType Type, u32 Index) const
{
    return (Type == eIncoming ? mInLinks[Index] : mOutLinks[Index]);
}

void CScriptObject::AddLink(ELinkType Type, CLink *pLink, u32 Index /*= -1*/)
{
    std::vector<CLink*> *pLinkVec = (Type == eIncoming ? &mInLinks : &mOutLinks);

    if (Index == -1 || Index == pLinkVec->size())
        pLinkVec->push_back(pLink);
    else
    {
        auto it = pLinkVec->begin();
        std::advance(it, Index);
        pLinkVec->insert(it, pLink);
    }
}

void CScriptObject::RemoveLink(ELinkType Type, CLink *pLink)
{
    std::vector<CLink*> *pLinkVec = (Type == eIncoming ? &mInLinks : &mOutLinks);

    for (auto it = pLinkVec->begin(); it != pLinkVec->end(); it++)
    {
        if (*it == pLink)
        {
            pLinkVec->erase(it);
            break;
        }
    }
}

void CScriptObject::BreakAllLinks()
{
    for (auto it = mInLinks.begin(); it != mInLinks.end(); it++)
    {
        CLink *pLink = *it;
        pLink->Sender()->RemoveLink(eOutgoing, pLink);
        delete pLink;
    }

    for (auto it = mOutLinks.begin(); it != mOutLinks.end(); it++)
    {
        CLink *pLink = *it;
        pLink->Receiver()->RemoveLink(eIncoming, pLink);
        delete pLink;
    }

    mInLinks.clear();
    mOutLinks.clear();
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

float CScriptObject::VolumeScale() const
{
    return mVolumeScale;
}
