#include "CDependencyTree.h"
#include "Core/GameProject/CGameProject.h"
#include "Core/Resource/Script/CMasterTemplate.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/Resource/Script/CScriptObject.h"

CDependencyNodeFactory gDependencyNodeFactory;

// ************ IDependencyNode ************
IDependencyNode::~IDependencyNode()
{
    for (u32 iChild = 0; iChild < mChildren.size(); iChild++)
        delete mChildren[iChild];
}

bool IDependencyNode::HasDependency(const CAssetID& rkID) const
{
    for (u32 iChild = 0; iChild < mChildren.size(); iChild++)
    {
        if (mChildren[iChild]->HasDependency(rkID))
            return true;
    }

    return false;
}

// ************ CDependencyTree ************
EDependencyNodeType CDependencyTree::Type() const
{
    return eDNT_DependencyTree;
}

void CDependencyTree::Serialize(IArchive& rArc)
{
    rArc << SERIAL("RootID", mRootID)
         << SERIAL_ABSTRACT_CONTAINER("Children", mChildren, "Child", &gDependencyNodeFactory);
}

void CDependencyTree::AddChild(IDependencyNode *pNode)
{
    ASSERT(pNode);
    mChildren.push_back(pNode);
}

void CDependencyTree::AddDependency(const CAssetID& rkID, bool AvoidDuplicates /*= true*/)
{
    if (!rkID.IsValid() || (AvoidDuplicates && HasDependency(rkID))) return;
    CResourceDependency *pDepend = new CResourceDependency(rkID);
    mChildren.push_back(pDepend);
}

void CDependencyTree::AddDependency(CResource *pRes, bool AvoidDuplicates /*= true*/)
{
    if (!pRes) return;
    AddDependency(pRes->ID(), AvoidDuplicates);
}

// ************ CResourceDependency ************
EDependencyNodeType CResourceDependency::Type() const
{
    return eDNT_ResourceDependency;
}

void CResourceDependency::Serialize(IArchive& rArc)
{
    rArc << SERIAL("ID", mID);
}

bool CResourceDependency::HasDependency(const CAssetID& rkID) const
{
    return (mID == rkID);
}

// ************ CPropertyDependency ************
EDependencyNodeType CPropertyDependency::Type() const
{
    return eDNT_ScriptProperty;
}

void CPropertyDependency::Serialize(IArchive& rArc)
{
    rArc << SERIAL("PropertyID", mIDString);
    CResourceDependency::Serialize(rArc);
}

// ************ CCharacterPropertyDependency ************
EDependencyNodeType CCharPropertyDependency::Type() const
{
    return eDNT_CharacterProperty;
}

void CCharPropertyDependency::Serialize(IArchive& rArc)
{
    CPropertyDependency::Serialize(rArc);
    rArc << SERIAL("CharIndex", mUsedChar);
}

// ************ CScriptInstanceDependency ************
EDependencyNodeType CScriptInstanceDependency::Type() const
{
    return eDNT_ScriptInstance;
}

void CScriptInstanceDependency::Serialize(IArchive& rArc)
{
    rArc << SERIAL("ObjectType", mObjectType)
         << SERIAL_ABSTRACT_CONTAINER("Properties", mChildren, "Property", &gDependencyNodeFactory);
}

// Static
CScriptInstanceDependency* CScriptInstanceDependency::BuildTree(CScriptObject *pInstance)
{
    CScriptInstanceDependency *pInst = new CScriptInstanceDependency();
    pInst->mObjectType = pInstance->ObjectTypeID();
    ParseStructDependencies(pInst, pInstance->Properties());
    return pInst;
}

void CScriptInstanceDependency::ParseStructDependencies(CScriptInstanceDependency *pInst, CPropertyStruct *pStruct)
{
    // Recursive function for parsing script dependencies and loading them into the script instance dependency
    for (u32 iProp = 0; iProp < pStruct->Count(); iProp++)
    {
        IProperty *pProp = pStruct->PropertyByIndex(iProp);
        EPropertyType Type = pProp->Type();

        if (Type == eStructProperty || Type == eArrayProperty)
            ParseStructDependencies(pInst, static_cast<CPropertyStruct*>(pProp));

        else if (Type == eSoundProperty)
        {
            u32 SoundID = static_cast<TSoundProperty*>(pProp)->Get();

            if (SoundID != -1)
            {
                SSoundInfo Info = CGameProject::ActiveProject()->AudioManager()->GetSoundInfo(SoundID);

                if (Info.pAudioGroup)
                {
                    CPropertyDependency *pDep = new CPropertyDependency(pProp->IDString(true), Info.pAudioGroup->ID());
                    pInst->mChildren.push_back(pDep);
                }
            }
        }

        else if (Type == eAssetProperty)
        {
            CAssetID ID = static_cast<TAssetProperty*>(pProp)->Get();

            if (ID.IsValid())
            {
                CPropertyDependency *pDep = new CPropertyDependency(pProp->IDString(true), ID);
                pInst->mChildren.push_back(pDep);
            }
        }

        else if (Type == eCharacterProperty)
        {
            TCharacterProperty *pChar = static_cast<TCharacterProperty*>(pProp);
            CAnimationParameters Params = pChar->Get();
            CAssetID ID = Params.ID();

            if (ID.IsValid())
            {
                // Character sets are removed starting in MP3, so we only need char property dependencies in Echoes and earlier
                if (pStruct->Instance()->Area()->Game() <= eEchoes)
                {
                    CCharPropertyDependency *pDep = new CCharPropertyDependency(pProp->IDString(true), ID, Params.CharacterIndex());
                    pInst->mChildren.push_back(pDep);
                }
                else
                {
                    CPropertyDependency *pDep = new CPropertyDependency(pProp->IDString(true), ID);
                    pInst->mChildren.push_back(pDep);
                }
            }
        }
    }
}

// ************ CSetCharacterDependency ************
EDependencyNodeType CSetCharacterDependency::Type() const
{
    return eDNT_SetCharacter;
}

void CSetCharacterDependency::Serialize(IArchive& rArc)
{
    rArc << SERIAL("CharSetIndex", mCharSetIndex)
         << SERIAL_ABSTRACT_CONTAINER("Children", mChildren, "Child", &gDependencyNodeFactory);
}

CSetCharacterDependency* CSetCharacterDependency::BuildTree(const CAnimSet *pkOwnerSet, u32 CharIndex)
{
    CSetCharacterDependency *pTree = new CSetCharacterDependency(CharIndex);
    const SSetCharacter *pkChar = pkOwnerSet->Character(CharIndex);

    if (pkChar)
    {
        pTree->AddDependency(pkChar->pModel);
        pTree->AddDependency(pkChar->pSkeleton);
        pTree->AddDependency(pkChar->pSkin);

        const std::vector<CAssetID> *pkParticleVectors[5] = {
            &pkChar->GenericParticles, &pkChar->ElectricParticles,
            &pkChar->SwooshParticles, &pkChar->SpawnParticles,
            &pkChar->EffectParticles
        };

        for (u32 iVec = 0; iVec < 5; iVec++)
        {
            for (u32 iPart = 0; iPart < pkParticleVectors[iVec]->size(); iPart++)
                pTree->AddDependency(pkParticleVectors[iVec]->at(iPart));
        }

        pTree->AddDependency(pkChar->IceModel);
        pTree->AddDependency(pkChar->IceSkin);
    }

    return pTree;
}

// ************ CSetAnimationDependency ************
EDependencyNodeType CSetAnimationDependency::Type() const
{
    return eDNT_SetAnimation;
}

void CSetAnimationDependency::Serialize(IArchive& rArc)
{
    rArc << SERIAL_CONTAINER("CharacterIndices", mCharacterIndices, "Index")
         << SERIAL_ABSTRACT_CONTAINER("Children", mChildren, "Child", &gDependencyNodeFactory);
}

CSetAnimationDependency* CSetAnimationDependency::BuildTree(const CAnimSet *pkOwnerSet, u32 AnimIndex)
{
    CSetAnimationDependency *pTree = new CSetAnimationDependency;
    const SAnimation *pkAnim = pkOwnerSet->Animation(AnimIndex);

    // Find relevant character indices
    for (u32 iChar = 0; iChar < pkOwnerSet->NumCharacters(); iChar++)
    {
        const SSetCharacter *pkChar = pkOwnerSet->Character(iChar);

        if ( pkChar->UsedAnimationIndices.find(AnimIndex) != pkChar->UsedAnimationIndices.end() )
            pTree->mCharacterIndices.insert(iChar);
    }

    // Add primitive dependencies. In MP2 animation event data is not a standalone resource.
    std::set<CAnimPrimitive> UsedPrimitives;
    pkAnim->pMetaAnim->GetUniquePrimitives(UsedPrimitives);

    for (auto Iter = UsedPrimitives.begin(); Iter != UsedPrimitives.end(); Iter++)
    {
        const CAnimPrimitive& rkPrim = *Iter;
        pTree->AddDependency(rkPrim.Animation());

        if (pkOwnerSet->Game() >= eEchoesDemo)
        {
            CAnimEventData *pEvents = pkOwnerSet->AnimationEventData(rkPrim.ID());
            ASSERT(pEvents && !pEvents->Entry());
            pEvents->AddDependenciesToTree(pTree);
        }
    }

    return pTree;
}

// ************ CAnimEventDependency ************
EDependencyNodeType CAnimEventDependency::Type() const
{
    return eDNT_AnimEvent;
}

void CAnimEventDependency::Serialize(IArchive& rArc)
{
    CResourceDependency::Serialize(rArc);
    rArc << SERIAL("CharacterIndex", mCharIndex);
}

// ************ CAreaDependencyTree ************
EDependencyNodeType CAreaDependencyTree::Type() const
{
    return eDNT_Area;
}

void CAreaDependencyTree::Serialize(IArchive& rArc)
{
    CDependencyTree::Serialize(rArc);
    rArc << SERIAL_CONTAINER("LayerOffsets", mLayerOffsets, "Offset");
}

void CAreaDependencyTree::AddScriptLayer(CScriptLayer *pLayer)
{
    if (!pLayer) return;
    mLayerOffsets.push_back(mChildren.size());

    for (u32 iInst = 0; iInst < pLayer->NumInstances(); iInst++)
    {
        CScriptInstanceDependency *pTree = CScriptInstanceDependency::BuildTree( pLayer->InstanceByIndex(iInst) );
        ASSERT(pTree != nullptr);

        if (pTree->NumChildren() > 0)
            mChildren.push_back(pTree);
        else
            delete pTree;
    }
}

void CAreaDependencyTree::GetModuleDependencies(EGame Game, std::vector<TString>& rModuleDepsOut, std::vector<u32>& rModuleLayerOffsetsOut) const
{
    CMasterTemplate *pMaster = CMasterTemplate::MasterForGame(Game);

    // Output module list will be split per-script layer
    // The output offset list contains two offsets per layer - start index and end index
    for (u32 iLayer = 0; iLayer < mLayerOffsets.size(); iLayer++)
    {
        u32 StartIdx = mLayerOffsets[iLayer];
        u32 EndIdx = (iLayer == mLayerOffsets.size() - 1 ? mChildren.size() : mLayerOffsets[iLayer + 1]);

        u32 ModuleStartIdx = rModuleDepsOut.size();
        rModuleLayerOffsetsOut.push_back(ModuleStartIdx);

        // Keep track of which types we've already checked on this layer to speed things up a little...
        std::set<u32> UsedObjectTypes;

        for (u32 iInst = StartIdx; iInst < EndIdx; iInst++)
        {
            CScriptInstanceDependency *pInst = static_cast<CScriptInstanceDependency*>(mChildren[iInst]);
            ASSERT(pInst->Type() == eDNT_ScriptInstance);
            u32 ObjType = pInst->ObjectType();

            if (UsedObjectTypes.find(ObjType) == UsedObjectTypes.end())
            {
                // Get the module list for this object type and check whether any of them are new before adding them to the output list
                CScriptTemplate *pTemplate = pMaster->TemplateByID(ObjType);
                const std::vector<TString>& rkModules = pTemplate->RequiredModules();

                for (u32 iMod = 0; iMod < rkModules.size(); iMod++)
                {
                    TString ModuleName = rkModules[iMod];
                    bool NewModule = true;

                    for (u32 iUsed = ModuleStartIdx; iUsed < rModuleDepsOut.size(); iUsed++)
                    {
                        if (rModuleDepsOut[iUsed] == ModuleName)
                        {
                            NewModule = false;
                            break;
                        }
                    }

                    if (NewModule)
                        rModuleDepsOut.push_back(ModuleName);
                }

                UsedObjectTypes.insert(ObjType);
            }
        }

        rModuleLayerOffsetsOut.push_back(rModuleDepsOut.size());
    }
}
