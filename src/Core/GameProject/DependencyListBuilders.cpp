#include "Core/GameProject/DependencyListBuilders.h"

#include "Core/NRangeUtils.h"
#include "Core/Resource/CDependencyGroup.h"
#include <Common/Log.h>

// ************ CCharacterUsageMap ************
bool CCharacterUsageMap::IsCharacterUsed(const CAssetID& rkID, size_t CharacterIndex) const
{
    if (mpStore->Game() >= EGame::CorruptionProto)
        return true;

    const auto Find = mUsageMap.find(rkID);
    if (Find == mUsageMap.cend())
        return false;

    const std::vector<bool>& rkUsageList = Find->second;
    if (CharacterIndex >= rkUsageList.size())
        return false;

    return rkUsageList[CharacterIndex];
}

bool CCharacterUsageMap::IsAnimationUsed(const CAssetID& rkID, const CSetAnimationDependency* pAnim) const
{
    const auto Find = mUsageMap.find(rkID);
    if (Find == mUsageMap.end())
        return false;

    const std::vector<bool>& rkUsageList = Find->second;

    for (uint32_t iChar = 0; iChar < rkUsageList.size(); iChar++)
    {
        if (rkUsageList[iChar] && pAnim->IsUsedByCharacter(iChar))
            return true;
    }

    return false;
}

void CCharacterUsageMap::FindUsagesForAsset(const CResourceEntry* pEntry)
{
    Clear();
    ParseDependencyNode(pEntry->Dependencies());
}

void CCharacterUsageMap::FindUsagesForArea(const CWorld* pWorld, const CResourceEntry* pEntry)
{
    ASSERT(pEntry->ResourceType() == EResourceType::Area);

    for (size_t iArea = 0; iArea < pWorld->NumAreas(); iArea++)
    {
        if (pWorld->AreaResourceID(iArea) == pEntry->ID())
        {
            FindUsagesForArea(pWorld, iArea);
            return;
        }
    }
}

void CCharacterUsageMap::FindUsagesForArea(const CWorld* pWorld, size_t AreaIndex)
{
    // We only need to search forward from this area to other areas that both use the same character(s) + have duplicates enabled
    Clear();

    for (size_t iArea = AreaIndex; iArea < pWorld->NumAreas(); iArea++)
    {
        if (!mIsInitialArea && mStillLookingIDs.empty())
            break;

        mCurrentAreaAllowsDupes = pWorld->DoesAreaAllowPakDuplicates(iArea);

        const CAssetID& AreaID = pWorld->AreaResourceID(iArea);
        CResourceEntry *pEntry = mpStore->FindEntry(AreaID);
        ASSERT(pEntry && pEntry->ResourceType() == EResourceType::Area);

        ParseDependencyNode(pEntry->Dependencies());
        mIsInitialArea = false;
    }
}

void CCharacterUsageMap::FindUsagesForLayer(const CResourceEntry* pAreaEntry, uint32_t LayerIndex)
{
    Clear();
    mLayerIndex = LayerIndex;

    const auto* pTree = static_cast<const CAreaDependencyTree*>(pAreaEntry->Dependencies());
    ASSERT(pTree->Type() == EDependencyNodeType::Area);

    // Only examine dependencies of the particular layer specified by the caller
    const bool IsLastLayer = mLayerIndex == pTree->NumScriptLayers() - 1;
    const size_t StartIdx = pTree->ScriptLayerOffset(mLayerIndex);
    const size_t EndIdx = IsLastLayer ? pTree->NumChildren() : pTree->ScriptLayerOffset(mLayerIndex + 1);

    for (size_t iInst = StartIdx; iInst < EndIdx; iInst++)
        ParseDependencyNode(pTree->ChildByIndex(iInst));
}

void CCharacterUsageMap::Clear()
{
    mUsageMap.clear();
    mStillLookingIDs.clear();
    mLayerIndex = UINT32_MAX;
    mIsInitialArea = true;
}

#include "Core/Resource/Animation/CAnimSet.h"

void CCharacterUsageMap::DebugPrintContents()
{
    for (const auto& [ID, usedList] : mUsageMap)
    {
        const auto* pSet = mpStore->LoadResource<CAnimSet>(ID);

        for (auto&& [idx, character] : Utils::enumerate(pSet->Characters()))
        {
            const bool Used = usedList.size() > idx && usedList[idx];
            NLog::Debug("{} : Char {} : {} : {}", ID.ToString(), idx, character.Name, (Used ? "USED" : "UNUSED"));
        }
    }
}

// ************ PROTECTED ************
void CCharacterUsageMap::ParseDependencyNode(const IDependencyNode* pNode)
{
    if (!pNode)
        return;

    const EDependencyNodeType Type = pNode->Type();

    if (Type == EDependencyNodeType::CharacterProperty)
    {
        const auto* pDep = static_cast<const CCharPropertyDependency*>(pNode);
        const CAssetID& ResID = pDep->ID();
        const auto Find = mUsageMap.find(ResID);

        if (!mIsInitialArea && !mStillLookingIDs.contains(ResID))
            return;

        if (Find != mUsageMap.cend())
        {
            if (!mIsInitialArea && mCurrentAreaAllowsDupes)
            {
                mStillLookingIDs.erase(mStillLookingIDs.find(ResID));
                return;
            }
        }
        else
        {
            if (!mIsInitialArea)
                return;

            mUsageMap.insert_or_assign(ResID, std::vector<bool>());
            mStillLookingIDs.insert(ResID);
        }

        std::vector<bool>& rUsageList = mUsageMap[ResID];
        const uint32_t UsedChar = pDep->UsedChar();

        if (rUsageList.size() <= UsedChar)
            rUsageList.resize(UsedChar + 1, false);

        rUsageList[UsedChar] = true;
    }
    // Parse dependencies of the referenced resource if it's a type that can reference animsets
    else if (Type == EDependencyNodeType::Resource || Type == EDependencyNodeType::ScriptProperty)
    {
        const auto* pDep = static_cast<const CResourceDependency*>(pNode);
        CResourceEntry* pEntry = mpStore->FindEntry(pDep->ID());

        if (pEntry != nullptr && pEntry->ResourceType() == EResourceType::Scan)
        {
            ParseDependencyNode(pEntry->Dependencies());
        }
    }
    else // Look for sub-dependencies of the current node
    {
        for (size_t iChild = 0; iChild < pNode->NumChildren(); iChild++)
            ParseDependencyNode(pNode->ChildByIndex(iChild));
    }
}

// ************ CPackageDependencyListBuilder ************
void CPackageDependencyListBuilder::BuildDependencyList(bool AllowDuplicates, std::list<CAssetID>& rOut)
{
    mEnableDuplicates = AllowDuplicates;
    FindUniversalAreaAssets();

    // Iterate over all resources and parse their dependencies
    for (const auto& res : mpkPackage->NamedResources())
    {
        auto* pEntry = mpStore->FindEntry(res.ID);
        if (!pEntry)
            continue;

        if (res.Name.EndsWith("NODEPEND") || res.Type == CFourCC("CSNG"))
        {
            rOut.push_back(res.ID);
            continue;
        }

        mIsUniversalAreaAsset = mUniversalAreaAssets.contains(res.ID);

        if (res.Type == CFourCC("MLVL"))
        {
            mpWorld = static_cast<CWorld*>(pEntry->Load());
            ASSERT(mpWorld);
        }
        else
        {
            mCharacterUsageMap.FindUsagesForAsset(pEntry);
        }

        AddDependency(nullptr, res.ID, rOut);
        mpWorld = nullptr;
    }
}

void CPackageDependencyListBuilder::AddDependency(const CResourceEntry* pCurEntry, const CAssetID& rkID, std::list<CAssetID>& rOut)
{
    if (pCurEntry && pCurEntry->ResourceType() == EResourceType::DependencyGroup)
        return;

    const CResourceEntry* pEntry = mpStore->FindEntry(rkID);
    if (!pEntry)
        return;

    const EResourceType ResType = pEntry->ResourceType();

    // Is this entry valid?
    const bool IsValid =  ResType != EResourceType::Midi &&
                         (ResType != EResourceType::AudioGroup || mGame >= EGame::EchoesDemo) &&
                         (ResType != EResourceType::World || !pCurEntry) &&
                         (ResType != EResourceType::Area || !pCurEntry || pCurEntry->ResourceType() == EResourceType::World);

    if (!IsValid)
        return;

    if ((mCurrentAreaHasDuplicates && mAreaUsedAssets.contains(rkID)) ||
        (!mCurrentAreaHasDuplicates && mPackageUsedAssets.contains(rkID)) ||
        (!mIsUniversalAreaAsset && mUniversalAreaAssets.contains(rkID)))
    {
        return;
    }

    // Entry is valid, parse its sub-dependencies
    mPackageUsedAssets.insert(rkID);
    mAreaUsedAssets.insert(rkID);

    // New area - toggle duplicates and find character usages
    if (ResType == EResourceType::Area)
    {
        if (mGame <= EGame::Echoes)
            mCharacterUsageMap.FindUsagesForArea(mpWorld, pEntry);

        mAreaUsedAssets.clear();
        mCurrentAreaHasDuplicates = false;

        if (mEnableDuplicates)
        {
            for (size_t iArea = 0; iArea < mpWorld->NumAreas(); iArea++)
            {
                if (mpWorld->AreaResourceID(iArea) == rkID)
                {
                    mCurrentAreaHasDuplicates = mpWorld->DoesAreaAllowPakDuplicates(iArea);
                    break;
                }
            }
        }
    }
    // Animset - keep track of the current animset ID
    else if (ResType == EResourceType::AnimSet)
    {
        mCurrentAnimSetID = rkID;
    }

    // Evaluate dependencies of this entry
    const CDependencyTree* pTree = pEntry->Dependencies();
    EvaluateDependencyNode(pEntry, pTree, rOut);
    rOut.push_back(rkID);

    // Revert current animset ID
    if (ResType == EResourceType::AnimSet)
        mCurrentAnimSetID = CAssetID::InvalidID(mGame);
    // Revert duplicate flag
    else if (ResType == EResourceType::Area)
        mCurrentAreaHasDuplicates = false;
}

void CPackageDependencyListBuilder::EvaluateDependencyNode(const CResourceEntry* pCurEntry, const IDependencyNode* pNode, std::list<CAssetID>& rOut)
{
    if (!pNode)
        return;

    const EDependencyNodeType Type = pNode->Type();
    bool ParseChildren = false;

    // Straight resource dependencies should just be added to the tree directly
    if (Type == EDependencyNodeType::Resource || Type == EDependencyNodeType::ScriptProperty || Type == EDependencyNodeType::CharacterProperty)
    {
        const auto* pDep = static_cast<const CResourceDependency*>(pNode);
        AddDependency(pCurEntry, pDep->ID(), rOut);
    }
    // Anim events should be added if either they apply to characters, or their character index is used
    else if (Type == EDependencyNodeType::AnimEvent)
    {
        const auto* pDep = static_cast<const CAnimEventDependency*>(pNode);
        const uint32_t CharIndex = pDep->CharIndex();

        if (CharIndex == UINT32_MAX || mCharacterUsageMap.IsCharacterUsed(mCurrentAnimSetID, CharIndex))
            AddDependency(pCurEntry, pDep->ID(), rOut);
    }
    // Set characters should only be added if their character index is used
    else if (Type == EDependencyNodeType::SetCharacter)
    {
        const auto* pChar = static_cast<const CSetCharacterDependency*>(pNode);
        ParseChildren = mCharacterUsageMap.IsCharacterUsed(mCurrentAnimSetID, pChar->CharSetIndex()) || mIsPlayerActor;
    }
    // Set animations should only be added if they're being used by at least one used character
    else if (Type == EDependencyNodeType::SetAnimation)
    {
        const auto* pAnim = static_cast<const CSetAnimationDependency*>(pNode);
        ParseChildren = mCharacterUsageMap.IsAnimationUsed(mCurrentAnimSetID, pAnim) || (mIsPlayerActor && pAnim->IsUsedByAnyCharacter());
    }
    else
    {
        ParseChildren = true;
    }

    // Analyze this node's children
    if (ParseChildren)
    {
        if (Type == EDependencyNodeType::ScriptInstance)
        {
            const uint32_t ObjType = static_cast<const CScriptInstanceDependency*>(pNode)->ObjectType();
            mIsPlayerActor = (ObjType == 0x4C || ObjType == CFourCC("PLAC").ToU32());
        }

        for (size_t iChild = 0; iChild < pNode->NumChildren(); iChild++)
            EvaluateDependencyNode(pCurEntry, pNode->ChildByIndex(iChild), rOut);

        if (Type == EDependencyNodeType::ScriptInstance)
            mIsPlayerActor = false;
    }
}

void CPackageDependencyListBuilder::FindUniversalAreaAssets()
{
    const CGameProject* pProject = mpStore->Project();
    const CPackage* pPackage = pProject->FindPackage("UniverseArea");

    if (!pPackage)
        return;

    // Iterate over all the package contents, keep track of all universal area assets
    for (const auto& res : pPackage->NamedResources())
    {
        if (!res.ID.IsValid())
            continue;

        mUniversalAreaAssets.insert(res.ID);

        // For the universal area world, load it into memory to make sure we can exclude the area/map IDs
        if (res.Type != CFourCC("MLVL"))
            continue;

        const auto* pUniverseWorld = mpStore->LoadResource<CWorld>(res.ID);
        if (!pUniverseWorld)
            continue;

        // Area IDs
        for (size_t AreaIdx = 0; AreaIdx < pUniverseWorld->NumAreas(); AreaIdx++)
        {
            const CAssetID& AreaID = pUniverseWorld->AreaResourceID(AreaIdx);

            if (AreaID.IsValid())
                mUniversalAreaAssets.insert(AreaID);
        }

        // Map IDs
        const auto* pMapWorld = static_cast<const CDependencyGroup*>(pUniverseWorld->MapWorld());
        if (!pMapWorld)
            continue;

        for (const auto& DepID : pMapWorld->Dependencies())
        {
            if (DepID.IsValid())
                mUniversalAreaAssets.insert(DepID);
        }
    }
}

// ************ CAreaDependencyListBuilder ************
void CAreaDependencyListBuilder::BuildDependencyList(std::list<CAssetID>& rAssetsOut, std::list<uint32_t>& rLayerOffsetsOut, std::set<CAssetID>* pAudioGroupsOut)
{
    const auto* pTree = static_cast<CAreaDependencyTree*>(mpAreaEntry->Dependencies());

    // Fill area base used assets set (don't actually add to list yet)
    const size_t BaseEndIndex = pTree->NumScriptLayers() > 0 ? pTree->ScriptLayerOffset(0) : pTree->NumChildren();

    for (size_t iDep = 0; iDep < BaseEndIndex; iDep++)
    {
        const auto* pRes = static_cast<const CResourceDependency*>(pTree->ChildByIndex(iDep));
        ASSERT(pRes->Type() == EDependencyNodeType::Resource);
        mBaseUsedAssets.insert(pRes->ID());
    }

    // Get dependencies of each layer
    for (size_t iLyr = 0; iLyr < pTree->NumScriptLayers(); iLyr++)
    {
        mLayerUsedAssets.clear();
        mCharacterUsageMap.FindUsagesForLayer(mpAreaEntry, iLyr);
        rLayerOffsetsOut.push_back(rAssetsOut.size());

        const bool IsLastLayer = iLyr == pTree->NumScriptLayers() - 1;
        const size_t StartIdx = pTree->ScriptLayerOffset(iLyr);
        const size_t EndIdx = IsLastLayer ? pTree->NumChildren() : pTree->ScriptLayerOffset(iLyr + 1);

        for (size_t iChild = StartIdx; iChild < EndIdx; iChild++)
        {
            const IDependencyNode *pNode = pTree->ChildByIndex(iChild);

            if (pNode->Type() == EDependencyNodeType::ScriptInstance)
            {
                const auto* pInst = static_cast<const CScriptInstanceDependency*>(pNode);
                mIsPlayerActor = (pInst->ObjectType() == 0x4C || pInst->ObjectType() == CFourCC("PLAC").ToU32());

                for (size_t iDep = 0; iDep < pInst->NumChildren(); iDep++)
                {
                    const auto* pDep = static_cast<const CPropertyDependency*>(pInst->ChildByIndex(iDep));

                    // For MP3, exclude the CMDL/CSKR properties for the suit assets - only include default character assets
                    if (mGame == EGame::Corruption && mIsPlayerActor)
                    {
                        const TString& PropID = pDep->PropertyID();

                        if (PropID == "0x846397A8" || PropID == "0x685A4C01" ||
                            PropID == "0x9834ECC9" || PropID == "0x188B8960" ||
                            PropID == "0x134A81E3" || PropID == "0x4ABF030C" ||
                            PropID == "0x9BF030DC" || PropID == "0x981263D3" ||
                            PropID == "0x8A8D5AA5" || PropID == "0xE4734608" ||
                            PropID == "0x3376814D" || PropID == "0x797CA77E" ||
                            PropID == "0x0EBEC440" || PropID == "0xBC0952D8" ||
                            PropID == "0xA8778E57" || PropID == "0x1CB10DBE")
                        {
                            continue;
                        }
                    }

                    AddDependency(pDep->ID(), rAssetsOut, pAudioGroupsOut);
                }
            }
            else if (pNode->Type() == EDependencyNodeType::Resource)
            {
                const auto* pResDep = static_cast<const CResourceDependency*>(pNode);
                AddDependency(pResDep->ID(), rAssetsOut, pAudioGroupsOut);
            }
            else
            {
                ASSERT(false); // unhandled case!
            }
        }
    }

    // Add base assets
    mBaseUsedAssets.clear();
    mLayerUsedAssets.clear();
    rLayerOffsetsOut.push_back(rAssetsOut.size());

    for (size_t iDep = 0; iDep < BaseEndIndex; iDep++)
    {
        const auto* pDep = static_cast<const CResourceDependency*>(pTree->ChildByIndex(iDep));
        AddDependency(pDep->ID(), rAssetsOut, pAudioGroupsOut);
    }
}

void CAreaDependencyListBuilder::AddDependency(const CAssetID& rkID, std::list<CAssetID>& rOut, std::set<CAssetID>* pAudioGroupsOut)
{
    const CResourceEntry* pEntry = mpStore->FindEntry(rkID);
    if (!pEntry)
        return;

    const EResourceType ResType = pEntry->ResourceType();

    // If this is an audio group, for MP1, save it in the output set. For MP2, treat audio groups as a normal dependency.
    if (mGame <= EGame::Prime && ResType == EResourceType::AudioGroup)
    {
        if (pAudioGroupsOut)
            pAudioGroupsOut->insert(rkID);
        return;
    }

    // If this is an audio stream, skip
    if (ResType == EResourceType::StreamedAudio)
        return;

    // Check to ensure this is a valid/new dependency
    if (ResType == EResourceType::World || ResType == EResourceType::Area)
        return;

    if (mBaseUsedAssets.contains(rkID) || mLayerUsedAssets.contains(rkID))
        return;

    // Dependency is valid! Evaluate the node tree (except for SCAN and DGRP)
    if (ResType != EResourceType::Scan && ResType != EResourceType::DependencyGroup)
    {
        if (ResType == EResourceType::AnimSet)
        {
            ASSERT(!mCurrentAnimSetID.IsValid());
            mCurrentAnimSetID = pEntry->ID();
        }

        EvaluateDependencyNode(pEntry, pEntry->Dependencies(), rOut, pAudioGroupsOut);

        if (ResType == EResourceType::AnimSet)
        {
            ASSERT(mCurrentAnimSetID.IsValid());
            mCurrentAnimSetID = CAssetID::InvalidID(mGame);
        }
    }

    // Don't add CSNGs to the output dependency list (we parse them because we need their AGSC dependencies in the output AudioGroup set)
    if (ResType != EResourceType::Midi)
    {
        rOut.push_back(rkID);
        mLayerUsedAssets.insert(rkID);
    }
}

void CAreaDependencyListBuilder::EvaluateDependencyNode(const CResourceEntry* pCurEntry, const IDependencyNode* pNode,
                                                        std::list<CAssetID>& rOut, std::set<CAssetID>* pAudioGroupsOut)
{
    if (!pNode)
        return;

    const EDependencyNodeType Type = pNode->Type();
    bool ParseChildren = false;

    if (Type == EDependencyNodeType::Resource || Type == EDependencyNodeType::ScriptProperty || Type == EDependencyNodeType::CharacterProperty)
    {
        const auto* pDep = static_cast<const CResourceDependency*>(pNode);
        AddDependency(pDep->ID(), rOut, pAudioGroupsOut);
    }
    else if (Type == EDependencyNodeType::AnimEvent)
    {
        const auto* pDep = static_cast<const CAnimEventDependency*>(pNode);
        const uint32_t CharIndex = pDep->CharIndex();

        if (CharIndex == UINT32_MAX || mCharacterUsageMap.IsCharacterUsed(mCurrentAnimSetID, CharIndex))
            AddDependency(pDep->ID(), rOut, pAudioGroupsOut);
    }
    else if (Type == EDependencyNodeType::SetCharacter)
    {
        // Note: For MP1/2 PlayerActor, always treat as if Empty Suit is the only used one
        const uint32_t kEmptySuitIndex = (mGame >= EGame::EchoesDemo ? 3 : 5);

        const auto* pChar = static_cast<const CSetCharacterDependency*>(pNode);
        const uint32_t SetIndex = pChar->CharSetIndex();
        ParseChildren = mCharacterUsageMap.IsCharacterUsed(mCurrentAnimSetID, pChar->CharSetIndex()) || (mIsPlayerActor && SetIndex == kEmptySuitIndex);
    }
    else if (Type == EDependencyNodeType::SetAnimation)
    {
        const auto* pAnim = static_cast<const CSetAnimationDependency*>(pNode);
        ParseChildren = mCharacterUsageMap.IsAnimationUsed(mCurrentAnimSetID, pAnim) || (mIsPlayerActor && pAnim->IsUsedByAnyCharacter());
    }
    else
    {
        ParseChildren = true;
    }

    if (ParseChildren)
    {
        for (size_t iChild = 0; iChild < pNode->NumChildren(); iChild++)
            EvaluateDependencyNode(pCurEntry, pNode->ChildByIndex(iChild), rOut, pAudioGroupsOut);
    }
}

// ************ CAssetDependencyListBuilder ************
void CAssetDependencyListBuilder::BuildDependencyList(std::vector<CAssetID>& OutAssets)
{
    mCharacterUsageMap.FindUsagesForAsset(mpResourceEntry);
    EvaluateDependencyNode(mpResourceEntry, mpResourceEntry->Dependencies(), OutAssets);
}

void CAssetDependencyListBuilder::AddDependency(const CAssetID& kID, std::vector<CAssetID>& Out)
{
    const CResourceEntry *pEntry = mpResourceEntry->ResourceStore()->FindEntry(kID);
    if (!pEntry)
        return;

    const EResourceType ResType = pEntry->ResourceType();

    if (mUsedAssets.contains(kID))
        return;

    // Dependency is valid! Evaluate the node tree
    if (ResType == EResourceType::AnimSet)
    {
        ASSERT(!mCurrentAnimSetID.IsValid());
        mCurrentAnimSetID = pEntry->ID();
    }

    EvaluateDependencyNode(pEntry, pEntry->Dependencies(), Out);

    if (ResType == EResourceType::AnimSet)
    {
        ASSERT(mCurrentAnimSetID.IsValid());
        mCurrentAnimSetID = CAssetID::InvalidID(mpResourceEntry->Game());
    }

    Out.push_back(kID);
    mUsedAssets.insert(kID);
}

void CAssetDependencyListBuilder::EvaluateDependencyNode(const CResourceEntry* pCurEntry, const IDependencyNode* pNode, std::vector<CAssetID>& Out)
{
    if (!pNode)
        return;

    const EDependencyNodeType Type = pNode->Type();
    bool ParseChildren = false;

    if (Type == EDependencyNodeType::Resource || Type == EDependencyNodeType::ScriptProperty || Type == EDependencyNodeType::CharacterProperty)
    {
        const auto* pDep = static_cast<const CResourceDependency*>(pNode);
        AddDependency(pDep->ID(), Out);
    }
    else if (Type == EDependencyNodeType::AnimEvent)
    {
        const auto* pDep = static_cast<const CAnimEventDependency*>(pNode);
        const uint32_t CharIndex = pDep->CharIndex();

        if (CharIndex == UINT32_MAX || mCharacterUsageMap.IsCharacterUsed(mCurrentAnimSetID, CharIndex))
            AddDependency(pDep->ID(), Out);
    }
    else if (Type == EDependencyNodeType::SetCharacter)
    {
        const auto* pChar = static_cast<const CSetCharacterDependency*>(pNode);
        ParseChildren = mCharacterUsageMap.IsCharacterUsed(mCurrentAnimSetID, pChar->CharSetIndex());
    }
    else if (Type == EDependencyNodeType::SetAnimation)
    {
        const auto* pAnim = static_cast<const CSetAnimationDependency*>(pNode);
        ParseChildren = mCharacterUsageMap.IsAnimationUsed(mCurrentAnimSetID, pAnim);
    }
    else
    {
        ParseChildren = true;
    }

    if (ParseChildren)
    {
        for (size_t iChild = 0; iChild < pNode->NumChildren(); iChild++)
            EvaluateDependencyNode(pCurEntry, pNode->ChildByIndex(iChild), Out);
    }
}
