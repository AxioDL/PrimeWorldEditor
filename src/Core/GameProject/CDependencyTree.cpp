#include "CDependencyTree.h"
#include "Core/GameProject/CGameProject.h"
#include "Core/Resource/Animation/CAnimSet.h"
#include "Core/Resource/Script/CGameTemplate.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/Resource/Script/CScriptObject.h"
#include "Core/Resource/Script/NGameList.h"
#include <algorithm>
#include <array>

// ************ IDependencyNode ************
IDependencyNode::~IDependencyNode() = default;

bool IDependencyNode::HasDependency(const CAssetID& id) const
{
    return std::any_of(mChildren.cbegin(), mChildren.cend(),
                       [&id](const auto& entry) { return entry->HasDependency(id); });
}

void IDependencyNode::GetAllResourceReferences(std::set<CAssetID>& rOutSet) const
{
    for (const auto& child : mChildren)
        child->GetAllResourceReferences(rOutSet);
}

void IDependencyNode::ParseProperties(CResourceEntry* pParentEntry, CStructProperty* pProperties, void* pData)
{
    // Recursive function for parsing dependencies in properties
    for (size_t PropertyIdx = 0; PropertyIdx < pProperties->NumChildren(); PropertyIdx++)
    {
        IProperty* pProp = pProperties->ChildByIndex(PropertyIdx);
        const EPropertyType Type = pProp->Type();

        // Technically we aren't parsing array children, but it's not really worth refactoring this function
        // to support it when there aren't any array properties that contain any asset references anyway...
        if (Type == EPropertyType::Struct)
        {
            ParseProperties(pParentEntry, TPropCast<CStructProperty>(pProp), pData);
        }
        else if (Type == EPropertyType::Sound)
        {
            uint32 SoundID = TPropCast<CSoundProperty>(pProp)->Value(pData);

            if (SoundID != UINT32_MAX)
            {
                CGameProject* pProj = pParentEntry->Project();
                SSoundInfo Info = pProj->AudioManager()->GetSoundInfo(SoundID);

                if (Info.pAudioGroup)
                {
                    mChildren.push_back(std::make_unique<CPropertyDependency>(pProp->IDString(true), Info.pAudioGroup->ID()));
                }
            }
        }
        else if (Type == EPropertyType::Asset)
        {
            CAssetID ID = TPropCast<CAssetProperty>(pProp)->Value(pData);

            if (ID.IsValid())
            {
                mChildren.push_back(std::make_unique<CPropertyDependency>(pProp->IDString(true), ID));
            }
        }
        else if (Type == EPropertyType::AnimationSet)
        {
            CAnimationParameters Params = TPropCast<CAnimationSetProperty>(pProp)->Value(pData);
            CAssetID ID = Params.ID();

            if (ID.IsValid())
            {
                // Character sets are removed starting in MP3, so we only need char property dependencies in Echoes and earlier
                if (pProperties->Game() <= EGame::Echoes)
                {
                    mChildren.push_back(std::make_unique<CCharPropertyDependency>(pProp->IDString(true), ID, Params.CharacterIndex()));
                }
                else
                {
                    mChildren.push_back(std::make_unique<CPropertyDependency>(pProp->IDString(true), ID));
                }
            }
        }
    }
}

// Serialization constructor
IDependencyNode* IDependencyNode::ArchiveConstructor(EDependencyNodeType Type)
{
    switch (Type)
    {
    case EDependencyNodeType::DependencyTree:       return new CDependencyTree;
    case EDependencyNodeType::Resource:             return new CResourceDependency;
    case EDependencyNodeType::ScriptInstance:       return new CScriptInstanceDependency;
    case EDependencyNodeType::ScriptProperty:       return new CPropertyDependency;
    case EDependencyNodeType::CharacterProperty:    return new CCharPropertyDependency;
    case EDependencyNodeType::SetCharacter:         return new CSetCharacterDependency;
    case EDependencyNodeType::SetAnimation:         return new CSetAnimationDependency;
    case EDependencyNodeType::AnimEvent:            return new CAnimEventDependency;
    case EDependencyNodeType::Area:                 return new CAreaDependencyTree;
    default:                                        ASSERT(false); return nullptr;
    }
}

// ************ CDependencyTree ************
EDependencyNodeType CDependencyTree::Type() const
{
    return EDependencyNodeType::DependencyTree;
}

void CDependencyTree::Serialize(IArchive& rArc)
{
    rArc << SerialParameter("Children", mChildren);
}

void CDependencyTree::AddChild(std::unique_ptr<IDependencyNode>&& pNode)
{
    ASSERT(pNode);
    mChildren.push_back(std::move(pNode));
}

void CDependencyTree::AddDependency(const CAssetID& rkID, bool AvoidDuplicates /*= true*/)
{
    if (!rkID.IsValid() || (AvoidDuplicates && HasDependency(rkID)))
        return;

    mChildren.push_back(std::make_unique<CResourceDependency>(rkID));
}

void CDependencyTree::AddDependency(CResource *pRes, bool AvoidDuplicates /*= true*/)
{
    if (!pRes)
        return;

    AddDependency(pRes->ID(), AvoidDuplicates);
}

void CDependencyTree::AddCharacterDependency(const CAnimationParameters& rkAnimParams)
{
    // This is for formats other than MREA that use AnimationParameters (such as SCAN).
    CAnimSet *pSet = rkAnimParams.AnimSet();
    if (!pSet || rkAnimParams.CharacterIndex() == UINT32_MAX)
        return;

    mChildren.push_back(std::make_unique<CCharPropertyDependency>("NULL", pSet->ID(), rkAnimParams.CharacterIndex()));
}

// ************ CResourceDependency ************
EDependencyNodeType CResourceDependency::Type() const
{
    return EDependencyNodeType::Resource;
}

void CResourceDependency::Serialize(IArchive& rArc)
{
    rArc << SerialParameter("ID", mID);
}

void CResourceDependency::GetAllResourceReferences(std::set<CAssetID>& rOutSet) const
{
    rOutSet.insert(mID);
}

bool CResourceDependency::HasDependency(const CAssetID& rkID) const
{
    return (mID == rkID);
}

// ************ CPropertyDependency ************
EDependencyNodeType CPropertyDependency::Type() const
{
    return EDependencyNodeType::ScriptProperty;
}

void CPropertyDependency::Serialize(IArchive& rArc)
{
    rArc << SerialParameter("PropertyID", mIDString);
    CResourceDependency::Serialize(rArc);
}

// ************ CCharacterPropertyDependency ************
EDependencyNodeType CCharPropertyDependency::Type() const
{
    return EDependencyNodeType::CharacterProperty;
}

void CCharPropertyDependency::Serialize(IArchive& rArc)
{
    CPropertyDependency::Serialize(rArc);
    rArc << SerialParameter("CharIndex", mUsedChar);
}

// ************ CScriptInstanceDependency ************
EDependencyNodeType CScriptInstanceDependency::Type() const
{
    return EDependencyNodeType::ScriptInstance;
}

void CScriptInstanceDependency::Serialize(IArchive& rArc)
{
    rArc << SerialParameter("ObjectType", mObjectType)
         << SerialParameter("Properties", mChildren);
}

// Static
std::unique_ptr<CScriptInstanceDependency> CScriptInstanceDependency::BuildTree(CScriptObject *pInstance)
{
    auto pInst = std::make_unique<CScriptInstanceDependency>();
    pInst->mObjectType = pInstance->ObjectTypeID();
    pInst->ParseProperties(pInstance->Area()->Entry(), pInstance->Template()->Properties(), pInstance->PropertyData());
    return pInst;
}

// ************ CSetCharacterDependency ************
EDependencyNodeType CSetCharacterDependency::Type() const
{
    return EDependencyNodeType::SetCharacter;
}

void CSetCharacterDependency::Serialize(IArchive& rArc)
{
    rArc << SerialParameter("CharSetIndex", mCharSetIndex)
         << SerialParameter("Children", mChildren);
}

std::unique_ptr<CSetCharacterDependency> CSetCharacterDependency::BuildTree(const SSetCharacter& rkChar)
{
    auto pTree = std::make_unique<CSetCharacterDependency>(rkChar.ID);
    pTree->AddDependency(rkChar.pModel);
    pTree->AddDependency(rkChar.pSkeleton);
    pTree->AddDependency(rkChar.pSkin);
    pTree->AddDependency(rkChar.AnimDataID);
    pTree->AddDependency(rkChar.CollisionPrimitivesID);

    const std::array<const std::vector<CAssetID>*, 5> particleVectors{
        &rkChar.GenericParticles, &rkChar.ElectricParticles,
        &rkChar.SwooshParticles, &rkChar.SpawnParticles,
        &rkChar.EffectParticles
    };

    for (const auto& vec : particleVectors)
    {
        for (const auto& dependency : *vec)
            pTree->AddDependency(dependency);
    }

    for (const SOverlayModel& overlay : rkChar.OverlayModels)
    {
        pTree->AddDependency(overlay.ModelID);
        pTree->AddDependency(overlay.SkinID);
    }

    pTree->AddDependency(rkChar.SpatialPrimitives);
    return pTree;
}

// ************ CSetAnimationDependency ************
EDependencyNodeType CSetAnimationDependency::Type() const
{
    return EDependencyNodeType::SetAnimation;
}

void CSetAnimationDependency::Serialize(IArchive& rArc)
{
    rArc << SerialParameter("CharacterIndices", mCharacterIndices)
         << SerialParameter("Children", mChildren);
}

std::unique_ptr<CSetAnimationDependency> CSetAnimationDependency::BuildTree(const CAnimSet *pkOwnerSet, uint32 AnimIndex)
{
    auto pTree = std::make_unique<CSetAnimationDependency>();
    const SAnimation *pkAnim = pkOwnerSet->Animation(AnimIndex);

    // Find relevant character indices
    for (uint32 iChar = 0; iChar < pkOwnerSet->NumCharacters(); iChar++)
    {
        const SSetCharacter *pkChar = pkOwnerSet->Character(iChar);

        if (pkChar->UsedAnimationIndices.find(AnimIndex) != pkChar->UsedAnimationIndices.end())
            pTree->mCharacterIndices.insert(iChar);
    }

    // Add primitive dependencies. In MP2 animation event data is not a standalone resource.
    std::set<CAnimPrimitive> UsedPrimitives;
    pkAnim->pMetaAnim->GetUniquePrimitives(UsedPrimitives);

    for (const CAnimPrimitive& prim : UsedPrimitives)
    {
        pTree->AddDependency(prim.Animation());

        if (pkOwnerSet->Game() >= EGame::EchoesDemo)
        {
            CAnimEventData *pEvents = pkOwnerSet->AnimationEventData(prim.ID());
            ASSERT(pEvents && !pEvents->Entry());
            pEvents->AddDependenciesToTree(pTree.get());
        }
    }

    return pTree;
}

// ************ CAnimEventDependency ************
EDependencyNodeType CAnimEventDependency::Type() const
{
    return EDependencyNodeType::AnimEvent;
}

void CAnimEventDependency::Serialize(IArchive& rArc)
{
    CResourceDependency::Serialize(rArc);
    rArc << SerialParameter("CharacterIndex", mCharIndex);
}

// ************ CAreaDependencyTree ************
EDependencyNodeType CAreaDependencyTree::Type() const
{
    return EDependencyNodeType::Area;
}

void CAreaDependencyTree::Serialize(IArchive& rArc)
{
    CDependencyTree::Serialize(rArc);
    rArc << SerialParameter("LayerOffsets", mLayerOffsets);
}

void CAreaDependencyTree::AddScriptLayer(CScriptLayer *pLayer, const std::vector<CAssetID>& rkExtraDeps)
{
    if (!pLayer)
        return;

    mLayerOffsets.push_back(mChildren.size());
    std::set<CAssetID> UsedIDs;

    for (size_t iInst = 0; iInst < pLayer->NumInstances(); iInst++)
    {
        auto pTree = CScriptInstanceDependency::BuildTree(pLayer->InstanceByIndex(iInst));
        ASSERT(pTree != nullptr);

        // Note: MP2+ need to track all instances (not just instances with dependencies) to be able to build the layer module list
        if (pTree->NumChildren() > 0 || pLayer->Area()->Game() >= EGame::EchoesDemo)
        {
            pTree->GetAllResourceReferences(UsedIDs);
            mChildren.push_back(std::move(pTree));
        }
    }

    for (const auto& dep : rkExtraDeps)
        AddDependency(dep);
}

void CAreaDependencyTree::GetModuleDependencies(EGame Game, std::vector<TString>& rModuleDepsOut, std::vector<uint32>& rModuleLayerOffsetsOut) const
{
    CGameTemplate *pGame = NGameList::GetGameTemplate(Game);

    // Output module list will be split per-script layer
    // The output offset list contains two offsets per layer - start index and end index
    for (size_t iLayer = 0; iLayer < mLayerOffsets.size(); iLayer++)
    {
        const size_t StartIdx = mLayerOffsets[iLayer];
        const size_t EndIdx = (iLayer == mLayerOffsets.size() - 1 ? mChildren.size() : mLayerOffsets[iLayer + 1]);

        const auto ModuleStartIdx = static_cast<uint32>(rModuleDepsOut.size());
        rModuleLayerOffsetsOut.push_back(ModuleStartIdx);

        // Keep track of which types we've already checked on this layer to speed things up a little...
        std::set<uint32> UsedObjectTypes;

        for (size_t iInst = StartIdx; iInst < EndIdx; iInst++)
        {
            const auto& pNode = mChildren[iInst];
            if (pNode->Type() != EDependencyNodeType::ScriptInstance)
                continue;

            const auto *pInst = static_cast<CScriptInstanceDependency*>(pNode.get());
            const uint32 ObjType = pInst->ObjectType();

            if (UsedObjectTypes.find(ObjType) == UsedObjectTypes.end())
            {
                // Get the module list for this object type and check whether any of them are new before adding them to the output list
                const CScriptTemplate *pTemplate = pGame->TemplateByID(ObjType);
                const std::vector<TString>& rkModules = pTemplate->RequiredModules();

                for (const auto& ModuleName : rkModules)
                {
                    bool NewModule = true;

                    for (uint32 iUsed = ModuleStartIdx; iUsed < rModuleDepsOut.size(); iUsed++)
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

        rModuleLayerOffsetsOut.push_back(static_cast<uint32>(rModuleDepsOut.size()));
    }
}
