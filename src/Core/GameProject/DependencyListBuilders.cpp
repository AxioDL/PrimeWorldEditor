#include "DependencyListBuilders.h"

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

bool CCharacterUsageMap::IsAnimationUsed(const CAssetID& rkID, CSetAnimationDependency *pAnim) const
{
    const auto Find = mUsageMap.find(rkID);
    if (Find == mUsageMap.end())
        return false;

    const std::vector<bool>& rkUsageList = Find->second;

    for (uint32 iChar = 0; iChar < rkUsageList.size(); iChar++)
    {
        if (rkUsageList[iChar] && pAnim->IsUsedByCharacter(iChar))
            return true;
    }

    return false;
}

void CCharacterUsageMap::FindUsagesForAsset(CResourceEntry *pEntry)
{
    Clear();
    ParseDependencyNode(pEntry->Dependencies());
}

void CCharacterUsageMap::FindUsagesForArea(CWorld *pWorld, CResourceEntry *pEntry)
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

void CCharacterUsageMap::FindUsagesForArea(CWorld *pWorld, size_t AreaIndex)
{
    // We only need to search forward from this area to other areas that both use the same character(s) + have duplicates enabled
    Clear();

    for (size_t iArea = AreaIndex; iArea < pWorld->NumAreas(); iArea++)
    {
        if (!mIsInitialArea && mStillLookingIDs.empty())
            break;

        mCurrentAreaAllowsDupes = pWorld->DoesAreaAllowPakDuplicates(iArea);

        const CAssetID AreaID = pWorld->AreaResourceID(iArea);
        CResourceEntry *pEntry = mpStore->FindEntry(AreaID);
        ASSERT(pEntry && pEntry->ResourceType() == EResourceType::Area);

        ParseDependencyNode(pEntry->Dependencies());
        mIsInitialArea = false;
    }
}

void CCharacterUsageMap::FindUsagesForLayer(CResourceEntry *pAreaEntry, uint32 LayerIndex)
{
    Clear();
    mLayerIndex = LayerIndex;

    CAreaDependencyTree *pTree = static_cast<CAreaDependencyTree*>(pAreaEntry->Dependencies());
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

        for (size_t iChar = 0; iChar < pSet->NumCharacters(); iChar++)
        {
            const bool Used = usedList.size() > iChar && usedList[iChar];
            const TString CharName = pSet->Character(iChar)->Name;
            debugf("%s : Char %zu : %s : %s", *ID.ToString(), iChar, *CharName, (Used ? "USED" : "UNUSED"));
        }
    }
}

// ************ PROTECTED ************
void CCharacterUsageMap::ParseDependencyNode(IDependencyNode *pNode)
{
    if (!pNode)
        return;

    const EDependencyNodeType Type = pNode->Type();

    if (Type == EDependencyNodeType::CharacterProperty)
    {
        auto *pDep = static_cast<CCharPropertyDependency*>(pNode);
        const CAssetID ResID = pDep->ID();
        const auto Find = mUsageMap.find(ResID);

        if (!mIsInitialArea && mStillLookingIDs.find(ResID) == mStillLookingIDs.cend())
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
        const uint32 UsedChar = pDep->UsedChar();

        if (rUsageList.size() <= UsedChar)
            rUsageList.resize(UsedChar + 1, false);

        rUsageList[UsedChar] = true;
    }
    // Parse dependencies of the referenced resource if it's a type that can reference animsets
    else if (Type == EDependencyNodeType::Resource || Type == EDependencyNodeType::ScriptProperty)
    {
        auto* pDep = static_cast<CResourceDependency*>(pNode);
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
    for (size_t iRes = 0; iRes < mpkPackage->NumNamedResources(); iRes++)
    {
        const SNamedResource& rkRes = mpkPackage->NamedResourceByIndex(iRes);
        CResourceEntry *pEntry = mpStore->FindEntry(rkRes.ID);
        if (!pEntry)
            continue;

        if (rkRes.Name.EndsWith("NODEPEND") || rkRes.Type == "CSNG")
        {
            rOut.push_back(rkRes.ID);
            continue;
        }

        mIsUniversalAreaAsset = mUniversalAreaAssets.find(rkRes.ID) != mUniversalAreaAssets.cend();

        if (rkRes.Type == "MLVL")
        {
            mpWorld = static_cast<CWorld*>(pEntry->Load());
            ASSERT(mpWorld);
        }
        else
        {
            mCharacterUsageMap.FindUsagesForAsset(pEntry);
        }

        AddDependency(nullptr, rkRes.ID, rOut);
        mpWorld = nullptr;
    }
}

void CPackageDependencyListBuilder::AddDependency(CResourceEntry *pCurEntry, const CAssetID& rkID, std::list<CAssetID>& rOut)
{
    if (pCurEntry && pCurEntry->ResourceType() == EResourceType::DependencyGroup)
        return;

    CResourceEntry *pEntry = mpStore->FindEntry(rkID);
    if (!pEntry)
        return;

    EResourceType ResType = pEntry->ResourceType();

    // Is this entry valid?
    bool IsValid =  ResType != EResourceType::Midi &&
                   (ResType != EResourceType::AudioGroup || mGame >= EGame::EchoesDemo) &&
                   (ResType != EResourceType::World || !pCurEntry) &&
                   (ResType != EResourceType::Area || !pCurEntry || pCurEntry->ResourceType() == EResourceType::World);

    if (!IsValid)
        return;

    if ((mCurrentAreaHasDuplicates && mAreaUsedAssets.find(rkID) != mAreaUsedAssets.end()) ||
        (!mCurrentAreaHasDuplicates && mPackageUsedAssets.find(rkID) != mPackageUsedAssets.end()) ||
        (!mIsUniversalAreaAsset && mUniversalAreaAssets.find(rkID) != mUniversalAreaAssets.end()))
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
    CDependencyTree *pTree = pEntry->Dependencies();
    EvaluateDependencyNode(pEntry, pTree, rOut);
    rOut.push_back(rkID);

    // Revert current animset ID
    if (ResType == EResourceType::AnimSet)
        mCurrentAnimSetID = CAssetID::InvalidID(mGame);
    // Revert duplicate flag
    else if (ResType == EResourceType::Area)
        mCurrentAreaHasDuplicates = false;
}

void CPackageDependencyListBuilder::EvaluateDependencyNode(CResourceEntry *pCurEntry, IDependencyNode *pNode, std::list<CAssetID>& rOut)
{
    if (!pNode)
        return;

    const EDependencyNodeType Type = pNode->Type();
    bool ParseChildren = false;

    // Straight resource dependencies should just be added to the tree directly
    if (Type == EDependencyNodeType::Resource || Type == EDependencyNodeType::ScriptProperty || Type == EDependencyNodeType::CharacterProperty)
    {
        const auto *pDep = static_cast<CResourceDependency*>(pNode);
        AddDependency(pCurEntry, pDep->ID(), rOut);
    }
    // Anim events should be added if either they apply to characters, or their character index is used
    else if (Type == EDependencyNodeType::AnimEvent)
    {
        const auto *pDep = static_cast<CAnimEventDependency*>(pNode);
        const uint32 CharIndex = pDep->CharIndex();

        if (CharIndex == UINT32_MAX || mCharacterUsageMap.IsCharacterUsed(mCurrentAnimSetID, CharIndex))
            AddDependency(pCurEntry, pDep->ID(), rOut);
    }
    // Set characters should only be added if their character index is used
    else if (Type == EDependencyNodeType::SetCharacter)
    {
        const auto *pChar = static_cast<CSetCharacterDependency*>(pNode);
        ParseChildren = mCharacterUsageMap.IsCharacterUsed(mCurrentAnimSetID, pChar->CharSetIndex()) || mIsPlayerActor;
    }
    // Set animations should only be added if they're being used by at least one used character
    else if (Type == EDependencyNodeType::SetAnimation)
    {
        auto *pAnim = static_cast<CSetAnimationDependency*>(pNode);
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
            const uint32 ObjType = static_cast<CScriptInstanceDependency*>(pNode)->ObjectType();
            mIsPlayerActor = (ObjType == 0x4C || ObjType == FOURCC('PLAC'));
        }

        for (size_t iChild = 0; iChild < pNode->NumChildren(); iChild++)
            EvaluateDependencyNode(pCurEntry, pNode->ChildByIndex(iChild), rOut);

        if (Type == EDependencyNodeType::ScriptInstance)
            mIsPlayerActor = false;
    }
}

void CPackageDependencyListBuilder::FindUniversalAreaAssets()
{
    CGameProject *pProject = mpStore->Project();
    CPackage *pPackage = pProject->FindPackage("UniverseArea");

    if (pPackage)
    {
        // Iterate over all the package contents, keep track of all universal area assets
        for (size_t ResIdx = 0; ResIdx < pPackage->NumNamedResources(); ResIdx++)
        {
            const SNamedResource& rkRes = pPackage->NamedResourceByIndex(ResIdx);

            if (rkRes.ID.IsValid())
            {
                mUniversalAreaAssets.insert(rkRes.ID);

                // For the universal area world, load it into memory to make sure we can exclude the area/map IDs
                if (rkRes.Type == "MLVL")
                {
                    CWorld *pUniverseWorld = gpResourceStore->LoadResource<CWorld>(rkRes.ID);

                    if (pUniverseWorld)
                    {
                        // Area IDs
                        for (size_t AreaIdx = 0; AreaIdx < pUniverseWorld->NumAreas(); AreaIdx++)
                        {
                            const CAssetID AreaID = pUniverseWorld->AreaResourceID(AreaIdx);

                            if (AreaID.IsValid())
                                mUniversalAreaAssets.insert(AreaID);
                        }

                        // Map IDs
                        auto *pMapWorld = static_cast<CDependencyGroup*>(pUniverseWorld->MapWorld());

                        if (pMapWorld)
                        {
                            for (size_t DepIdx = 0; DepIdx < pMapWorld->NumDependencies(); DepIdx++)
                            {
                                const CAssetID DepID = pMapWorld->DependencyByIndex(DepIdx);

                                if (DepID.IsValid())
                                    mUniversalAreaAssets.insert(DepID);
                            }
                        }
                    }
                }
            }
        }
    }
}

// ************ CAreaDependencyListBuilder ************
void CAreaDependencyListBuilder::BuildDependencyList(std::list<CAssetID>& rAssetsOut, std::list<uint32>& rLayerOffsetsOut, std::set<CAssetID> *pAudioGroupsOut)
{
    CAreaDependencyTree *pTree = static_cast<CAreaDependencyTree*>(mpAreaEntry->Dependencies());

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
                mIsPlayerActor = (pInst->ObjectType() == 0x4C || pInst->ObjectType() == FOURCC('PLAC'));

                for (size_t iDep = 0; iDep < pInst->NumChildren(); iDep++)
                {
                    const auto* pDep = static_cast<const CPropertyDependency*>(pInst->ChildByIndex(iDep));

                    // For MP3, exclude the CMDL/CSKR properties for the suit assets - only include default character assets
                    if (mGame == EGame::Corruption && mIsPlayerActor)
                    {
                        TString PropID = pDep->PropertyID();

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

void CAreaDependencyListBuilder::AddDependency(const CAssetID& rkID, std::list<CAssetID>& rOut, std::set<CAssetID> *pAudioGroupsOut)
{
    CResourceEntry *pEntry = mpStore->FindEntry(rkID);
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

    if (mBaseUsedAssets.find(rkID) != mBaseUsedAssets.end() || mLayerUsedAssets.find(rkID) != mLayerUsedAssets.end())
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

void CAreaDependencyListBuilder::EvaluateDependencyNode(CResourceEntry *pCurEntry, IDependencyNode *pNode, std::list<CAssetID>& rOut, std::set<CAssetID> *pAudioGroupsOut)
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
        const uint32 CharIndex = pDep->CharIndex();

        if (CharIndex == UINT32_MAX || mCharacterUsageMap.IsCharacterUsed(mCurrentAnimSetID, CharIndex))
            AddDependency(pDep->ID(), rOut, pAudioGroupsOut);
    }
    else if (Type == EDependencyNodeType::SetCharacter)
    {
        // Note: For MP1/2 PlayerActor, always treat as if Empty Suit is the only used one
        const uint32 kEmptySuitIndex = (mGame >= EGame::EchoesDemo ? 3 : 5);

        const auto *pChar = static_cast<const CSetCharacterDependency*>(pNode);
        const uint32 SetIndex = pChar->CharSetIndex();
        ParseChildren = mCharacterUsageMap.IsCharacterUsed(mCurrentAnimSetID, pChar->CharSetIndex()) || (mIsPlayerActor && SetIndex == kEmptySuitIndex);
    }
    else if (Type == EDependencyNodeType::SetAnimation)
    {
        auto *pAnim = static_cast<CSetAnimationDependency*>(pNode);
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
    CResourceEntry *pEntry = mpResourceEntry->ResourceStore()->FindEntry(kID);
    if (!pEntry)
        return;

    const EResourceType ResType = pEntry->ResourceType();

    if (mUsedAssets.find(kID) != mUsedAssets.cend())
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

void CAssetDependencyListBuilder::EvaluateDependencyNode(CResourceEntry* pCurEntry, IDependencyNode* pNode, std::vector<CAssetID>& Out)
{
    if (!pNode)
        return;

    const EDependencyNodeType Type = pNode->Type();
    bool ParseChildren = false;

    if (Type == EDependencyNodeType::Resource || Type == EDependencyNodeType::ScriptProperty || Type == EDependencyNodeType::CharacterProperty)
    {
        const auto* pDep = static_cast<CResourceDependency*>(pNode);
        AddDependency(pDep->ID(), Out);
    }
    else if (Type == EDependencyNodeType::AnimEvent)
    {
        const auto* pDep = static_cast<CAnimEventDependency*>(pNode);
        const uint32 CharIndex = pDep->CharIndex();

        if (CharIndex == UINT32_MAX || mCharacterUsageMap.IsCharacterUsed(mCurrentAnimSetID, CharIndex))
            AddDependency(pDep->ID(), Out);
    }
    else if (Type == EDependencyNodeType::SetCharacter)
    {
        const auto* pChar = static_cast<CSetCharacterDependency*>(pNode);
        ParseChildren = mCharacterUsageMap.IsCharacterUsed(mCurrentAnimSetID, pChar->CharSetIndex());
    }
    else if (Type == EDependencyNodeType::SetAnimation)
    {
        auto* pAnim = static_cast<CSetAnimationDependency*>(pNode);
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
