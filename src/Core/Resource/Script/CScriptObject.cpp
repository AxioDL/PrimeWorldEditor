#include "CScriptObject.h"
#include "CScriptLayer.h"
#include "CGameTemplate.h"
#include "Core/Resource/Animation/CAnimSet.h"

CScriptObject::CScriptObject(uint32 InstanceID, CGameArea *pArea, CScriptLayer *pLayer, CScriptTemplate *pTemplate)
    : mpTemplate(pTemplate)
    , mpArea(pArea)
    , mpLayer(pLayer)
    , mVersion(0)
    , mInstanceID(InstanceID)
    , mHasInGameModel(false)
    , mIsCheckingNearVisibleActivation(false)
{
    mpTemplate->AddObject(this);

    // Init properties
    CStructProperty* pProperties = pTemplate->Properties();
    uint32 PropertiesSize = pProperties->DataSize();

    mPropertyData.resize( PropertiesSize );
    void* pData = mPropertyData.data();
    pProperties->Construct( pData );

    mInstanceName = CStringRef(pData, pTemplate->NameProperty());
    mPosition = CVectorRef(pData, pTemplate->PositionProperty());
    mRotation = CVectorRef(pData, pTemplate->RotationProperty());
    mScale = CVectorRef(pData, pTemplate->ScaleProperty());
    mActive = CBoolRef(pData, pTemplate->ActiveProperty());
    mLightParameters = CStructRef(pData, pTemplate->LightParametersProperty());
}

CScriptObject::~CScriptObject()
{
    if (!mPropertyData.empty())
    {
        mpTemplate->Properties()->Destruct( mPropertyData.data() );
        mPropertyData.clear();
    }

    mpTemplate->RemoveObject(this);

    // Note: Incoming links will be deleted by the sender.
    for (uint32 iLink = 0; iLink < mOutLinks.size(); iLink++)
        delete mOutLinks[iLink];
}

// ************ DATA MANIPULATION ************
void CScriptObject::CopyProperties(CScriptObject* pObject)
{
    ASSERT(pObject->Template() == Template());
    CSerialVersion Version(0, IArchive::skCurrentArchiveVersion, Template()->Game());

    CVectorOutStream DataStream;
    CBasicBinaryWriter DataWriter(&DataStream, Version);
    Template()->Properties()->SerializeValue( pObject->PropertyData(), DataWriter );

    CBasicBinaryReader DataReader(DataStream.Data(), DataStream.Size(), Version);
    Template()->Properties()->SerializeValue( PropertyData(), DataReader );
}

 void CScriptObject::EvaluateProperties()
{
    EvaluateDisplayAsset();
    EvaluateCollisionModel();
    EvaluateVolume();
}

void CScriptObject::EvaluateDisplayAsset()
{
    mpDisplayAsset = mpTemplate->FindDisplayAsset(PropertyData(), mActiveCharIndex, mActiveAnimIndex, mHasInGameModel);
}

void CScriptObject::EvaluateCollisionModel()
{
    mpCollision = mpTemplate->FindCollision(PropertyData());
}

void CScriptObject::EvaluateVolume()
{
    mVolumeShape = mpTemplate->VolumeShape(this);
    mVolumeScale = mpTemplate->VolumeScale(this);
}

bool CScriptObject::IsEditorProperty(IProperty *pProp)
{
    return ( (pProp == mInstanceName.Property()) ||
             (pProp == mPosition.Property()) ||
             (pProp == mRotation.Property()) ||
             (pProp == mScale.Property()) ||
             (pProp == mActive.Property()) ||
             (pProp == mLightParameters.Property()) ||
             (pProp->Parent() == mPosition.Property()) ||
             (pProp->Parent() == mRotation.Property()) ||
             (pProp->Parent() == mScale.Property()) ||
             (pProp->Parent() == mLightParameters.Property())
           );
}

void CScriptObject::SetLayer(CScriptLayer *pLayer, uint32 NewLayerIndex)
{
    ASSERT(pLayer != nullptr);

    if (pLayer != mpLayer)
    {
        if (mpLayer) mpLayer->RemoveInstance(this);
        mpLayer = pLayer;
        mpLayer->AddInstance(this, NewLayerIndex);
    }
}

uint32 CScriptObject::LayerIndex() const
{
    if (!mpLayer) return -1;

    for (uint32 iInst = 0; iInst < mpLayer->NumInstances(); iInst++)
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

    for (uint32 iLink = 0; iLink < mInLinks.size(); iLink++)
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

void CScriptObject::AddLink(ELinkType Type, CLink *pLink, uint32 Index /*= -1*/)
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
        CScriptObject *pSender = pLink->Sender();
        if (pSender) pSender->RemoveLink(eOutgoing, pLink);
        delete pLink;
    }

    for (auto it = mOutLinks.begin(); it != mOutLinks.end(); it++)
    {
        CLink *pLink = *it;
        CScriptObject *pReceiver = pLink->Receiver();
        if (pReceiver) pReceiver->RemoveLink(eIncoming, pLink);
        delete pLink;
    }

    mInLinks.clear();
    mOutLinks.clear();
}
