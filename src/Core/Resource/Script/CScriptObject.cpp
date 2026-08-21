#include "Core/Resource/Script/CScriptObject.h"

#include <Common/FileIO/CVectorOutStream.h>
#include <Common/Serialization/CBasicBinaryReader.h>
#include <Common/Serialization/CBasicBinaryWriter.h>
#include "Core/NRangeUtils.h"
#include "Core/Resource/Animation/CAnimSet.h"
#include "Core/Resource/Area/CGameArea.h"
#include "Core/Resource/Collision/CCollisionMeshGroup.h"
#include "Core/Resource/Script/CGameTemplate.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/Resource/Script/CScriptTemplate.h"

#include <algorithm>

CScriptObject::CScriptObject(CInstanceID ID, CGameArea *pArea, CScriptLayer *pLayer, CScriptTemplate *pTemplate)
    : mpTemplate(pTemplate)
    , mpArea(pArea)
    , mpLayer(pLayer)
    , mInstanceID(ID)
{
    mpTemplate->AddObject(this);

    // Init properties
    CStructProperty* pProperties = pTemplate->Properties();
    uint32_t PropertiesSize = pProperties->DataSize();

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

    CVectorOutStream DataStream(std::endian::native);
    CBasicBinaryWriter DataWriter(&DataStream, Version);
    Template()->Properties()->SerializeValue(pObject->PropertyData(), DataWriter);

    CBasicBinaryReader DataReader(DataStream.Data(), DataStream.Size(), Version, std::endian::native);
    Template()->Properties()->SerializeValue(PropertyData(), DataReader);
}

 void CScriptObject::EvaluateProperties()
{
    EvaluateDisplayAsset();
    EvaluateCollisionModel();
    EvaluateVolume();
}

void CScriptObject::EvaluateDisplayAsset()
{
    mpDisplayAsset = mpTemplate->FindDisplayAsset(mpArea->Entry()->ResourceStore(), PropertyData(), mActiveCharIndex,
                                                  mActiveAnimIndex, mHasInGameModel);
}

void CScriptObject::EvaluateCollisionModel()
{
    mpCollision = mpTemplate->FindCollision(mpArea->Entry()->ResourceStore(), PropertyData());
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

void CScriptObject::SetLayer(CScriptLayer *pLayer, uint32_t NewLayerIndex)
{
    ASSERT(pLayer != nullptr);

    if (pLayer != mpLayer)
    {
        if (mpLayer) mpLayer->RemoveInstance(this);
        mpLayer = pLayer;
        mpLayer->AddInstance(this, NewLayerIndex);
    }
}

uint32_t CScriptObject::LayerIndex() const
{
    if (!mpLayer)
        return UINT32_MAX;

    for (const auto [idx, instance] : Utils::enumerate(mpLayer->Instances()))
    {
        if (instance == this)
            return uint32_t(idx);
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
        const CFourCC State(pLink->State());
        const CFourCC Message(pLink->Message());

        // Check for trigger activation
        if (State == CFourCC("IS04") || State == CFourCC("IS05") || State == CFourCC("IS06"))
        {
            if ((!IsRelay && Message == CFourCC("ACTV")) ||
                (IsRelay  && Message == CFourCC("ACTN")))
            {
                CScriptObject* pObj = pLink->Sender();

                if (pObj->ObjectTypeID() == CFourCC("TRGR").ToU32())
                {
                    mIsCheckingNearVisibleActivation = false;
                    return true;
                }
            }
        }
        // Check for relay activation
        else if (State == CFourCC("RLAY"))
        {
            if ((!IsRelay && Message == CFourCC("ACTV")) ||
                (IsRelay  && Message == CFourCC("ACTN")))
            {
                CScriptObject* pObj = pLink->Sender();

                if (pObj->ObjectTypeID() == CFourCC("SRLY").ToU32())
                    Relays.push_back(pObj);
            }
        }
    }

    // Check whether any of the relays have a near visible activation
    if (std::ranges::any_of(Relays, &CScriptObject::HasNearVisibleActivation))
    {
        mIsCheckingNearVisibleActivation = false;
        return true;
    }

    mIsCheckingNearVisibleActivation = false;
    return false;
}

void CScriptObject::AddLink(ELinkType Type, CLink *pLink, uint32_t Index)
{
    auto& LinkVec = (Type == ELinkType::Incoming ? mInLinks : mOutLinks);

    if (Index == UINT32_MAX || Index == LinkVec.size())
    {
        LinkVec.push_back(pLink);
    }
    else
    {
        auto it = LinkVec.begin();
        std::advance(it, Index);
        LinkVec.insert(it, pLink);
    }
}

void CScriptObject::RemoveLink(ELinkType Type, CLink* pLink)
{
    auto& LinkVec = (Type == ELinkType::Incoming ? mInLinks : mOutLinks);

    const auto it = std::ranges::find(LinkVec, pLink);
    if (it == LinkVec.end())
        return;

    LinkVec.erase(it);
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

CGameTemplate* CScriptObject::GameTemplate() const
{
    return mpTemplate->GameTemplate();
}

uint32_t CScriptObject::ObjectTypeID() const
{
    return mpTemplate->ObjectID();
}
