#include "CScriptObject.h"
#include "CScriptLayer.h"
#include "CGameTemplate.h"
#include "Core/Resource/Animation/CAnimSet.h"

CScriptObject::CScriptObject(uint32 InstanceID, CGameArea *pArea, CScriptLayer *pLayer, CScriptTemplate *pTemplate)
    : mpTemplate(pTemplate)
    , mpArea(pArea)
    , mpLayer(pLayer)
    , mInstanceID(InstanceID)
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
    for (auto* link : mOutLinks)
        delete link;
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

bool CScriptObject::IsEditorProperty(const IProperty *pProp) const
{
    return pProp == mInstanceName.Property() ||
           pProp == mPosition.Property() ||
           pProp == mRotation.Property() ||
           pProp == mScale.Property() ||
           pProp == mActive.Property() ||
           pProp == mLightParameters.Property() ||
           pProp->Parent() == mPosition.Property() ||
           pProp->Parent() == mRotation.Property() ||
           pProp->Parent() == mScale.Property() ||
           pProp->Parent() == mLightParameters.Property();
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
    if (!mpLayer)
        return UINT32_MAX;

    for (uint32 iInst = 0; iInst < mpLayer->NumInstances(); iInst++)
    {
        if (mpLayer->InstanceByIndex(iInst) == this)
            return iInst;
    }

    return UINT32_MAX;
}

bool CScriptObject::HasNearVisibleActivation() const
{
    /* This function is used to check whether an inactive DKCR object should render in game mode. DKCR deactivates a lot of
     * decorative actors when the player isn't close to them as an optimization. This means a lot of them are inactive by
     * default but should render in game mode anyway. To get around this, we'll check the links to find out whether this
     * instance has a "Near Visible" activation, which is typically done via a trigger that activates the object on
     * InternalState04/05/06 (usually through a relay). */
    std::list<CScriptObject*> Relays;
    const bool IsRelay = ObjectTypeID() == 0x53524C59;

    if (mIsCheckingNearVisibleActivation)
        return false;

    mIsCheckingNearVisibleActivation = true;

    for (const auto* pLink : mInLinks)
    {
        // Check for trigger activation
        if (pLink->State() == FOURCC('IS04') || pLink->State() == FOURCC('IS05') || pLink->State() == FOURCC('IS06'))
        {
            if ((!IsRelay && pLink->Message() == FOURCC('ACTV')) ||
                (IsRelay  && pLink->Message() == FOURCC('ACTN')))
            {
                CScriptObject *pObj = pLink->Sender();

                if (pObj->ObjectTypeID() == FOURCC('TRGR'))
                {
                    mIsCheckingNearVisibleActivation = false;
                    return true;
                }
            }
        }

        // Check for relay activation
        else if (pLink->State() == FOURCC('RLAY'))
        {
            if ( (!IsRelay && pLink->Message() == FOURCC('ACTV')) ||
                 (IsRelay  && pLink->Message() == FOURCC('ACTN')) )
            {
                CScriptObject *pObj = pLink->Sender();

                if (pObj->ObjectTypeID() == FOURCC('SRLY'))
                    Relays.push_back(pObj);
            }
        }
    }

    // Check whether any of the relays have a near visible activation
    const bool nearVisible = std::any_of(Relays.cbegin(), Relays.cend(),
                                         [](const auto* relay) { return relay->HasNearVisibleActivation(); });
    if (nearVisible)
    {
        mIsCheckingNearVisibleActivation = false;
        return true;
    }

    mIsCheckingNearVisibleActivation = false;
    return false;
}

void CScriptObject::AddLink(ELinkType Type, CLink *pLink, uint32 Index)
{
    std::vector<CLink*> *pLinkVec = (Type == ELinkType::Incoming ? &mInLinks : &mOutLinks);

    if (Index == UINT32_MAX || Index == pLinkVec->size())
    {
        pLinkVec->push_back(pLink);
    }
    else
    {
        auto it = pLinkVec->begin();
        std::advance(it, Index);
        pLinkVec->insert(it, pLink);
    }
}

void CScriptObject::RemoveLink(ELinkType Type, CLink *pLink)
{
    std::vector<CLink*> *pLinkVec = (Type == ELinkType::Incoming ? &mInLinks : &mOutLinks);

    for (auto it = pLinkVec->begin(); it != pLinkVec->end(); ++it)
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
    for (auto* link : mInLinks)
    {
        if (CScriptObject* sender = link->Sender())
            sender->RemoveLink(ELinkType::Outgoing, link);

        delete link;
    }

    for (auto* link : mOutLinks)
    {
        if (CScriptObject* receiver = link->Receiver())
            receiver->RemoveLink(ELinkType::Incoming, link);

        delete link;
    }

    mInLinks.clear();
    mOutLinks.clear();
}
