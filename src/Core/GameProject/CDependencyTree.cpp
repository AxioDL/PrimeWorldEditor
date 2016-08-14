#include "CDependencyTree.h"
#include "Core/Resource/Script/CMasterTemplate.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/Resource/Script/CScriptObject.h"

// ************ CResourceDependency ************
EDependencyNodeType CResourceDependency::Type() const
{
    return eDNT_ResourceDependency;
}

void CResourceDependency::Read(IInputStream& rFile, EIDLength IDLength)
{
    mID = CAssetID(rFile, IDLength);
}

void CResourceDependency::Write(IOutputStream& rFile, EIDLength IDLength) const
{
    if (IDLength == e32Bit)
        rFile.WriteLong(mID.ToLong());
    else
        rFile.WriteLongLong(mID.ToLongLong());
}

// ************ CAnimSetDependency ************
EDependencyNodeType CAnimSetDependency::Type() const
{
    return eDNT_AnimSet;
}

void CAnimSetDependency::Read(IInputStream& rFile, EIDLength IDLength)
{
    CResourceDependency::Read(rFile, IDLength);
    mUsedChar = rFile.ReadLong();
}

void CAnimSetDependency::Write(IOutputStream& rFile, EIDLength IDLength) const
{
    CResourceDependency::Write(rFile, IDLength);
    rFile.WriteLong(mUsedChar);
}

// Static
CAnimSetDependency* CAnimSetDependency::BuildDependency(TCharacterProperty *pProp)
{
    ASSERT(pProp && pProp->Type() == eCharacterProperty && pProp->Instance()->Area()->Game() <= eEchoes);

    CAnimationParameters Params = pProp->Get();
    if (!Params.ID().IsValid()) return nullptr;

    CAnimSetDependency *pDepend = new CAnimSetDependency;
    pDepend->SetID(Params.ID());
    pDepend->SetUsedChar(Params.CharacterIndex());
    return pDepend;
}

// ************ CDependencyTree ************
CDependencyTree::~CDependencyTree()
{
    for (u32 iRef = 0; iRef < mReferencedResources.size(); iRef++)
        delete mReferencedResources[iRef];
}

EDependencyNodeType CDependencyTree::Type() const
{
    return eDNT_Root;
}

void CDependencyTree::Read(IInputStream& rFile, EIDLength IDLength)
{
    mID = CAssetID(rFile, IDLength);

    u32 NumDepends = rFile.ReadLong();
    mReferencedResources.reserve(NumDepends);

    for (u32 iDep = 0; iDep < NumDepends; iDep++)
    {
        CResourceDependency *pDepend = new CResourceDependency;
        pDepend->Read(rFile, IDLength);
        mReferencedResources.push_back(pDepend);
    }
}

void CDependencyTree::Write(IOutputStream& rFile, EIDLength IDLength) const
{
    if (IDLength == e32Bit)
        rFile.WriteLong(mID.ToLong());
    else
        rFile.WriteLongLong(mID.ToLongLong());

    rFile.WriteLong( mReferencedResources.size() );

    for (u32 iDep = 0; iDep < mReferencedResources.size(); iDep++)
        mReferencedResources[iDep]->Write(rFile, IDLength);
}

u32 CDependencyTree::NumDependencies() const
{
    return mReferencedResources.size();
}

bool CDependencyTree::HasDependency(const CAssetID& rkID)
{
    for (u32 iDep = 0; iDep < mReferencedResources.size(); iDep++)
    {
        if (mReferencedResources[iDep]->ID() == rkID)
            return true;
    }

    return false;
}

CAssetID CDependencyTree::DependencyByIndex(u32 Index) const
{
    ASSERT(Index >= 0 && Index < mReferencedResources.size());
    return mReferencedResources[Index]->ID();
}

void CDependencyTree::AddDependency(CResource *pRes, bool AvoidDuplicates /*= true*/)
{
    if (!pRes) return;
    AddDependency(pRes->ID(), AvoidDuplicates);
}

void CDependencyTree::AddDependency(const CAssetID& rkID, bool AvoidDuplicates /*= true*/)
{
    if (!rkID.IsValid() || (AvoidDuplicates && HasDependency(rkID))) return;
    CResourceDependency *pDepend = new CResourceDependency(rkID);
    mReferencedResources.push_back(pDepend);
}

// ************ CAnimSetDependencyTree ************
EDependencyNodeType CAnimSetDependencyTree::Type() const
{
    return eDNT_AnimSet;
}

void CAnimSetDependencyTree::Read(IInputStream& rFile, EIDLength IDLength)
{
    CDependencyTree::Read(rFile, IDLength);
    u32 NumChars = rFile.ReadLong();
    mCharacterOffsets.reserve(NumChars);

    for (u32 iChar = 0; iChar < NumChars; iChar++)
        mCharacterOffsets.push_back( rFile.ReadLong() );
}

void CAnimSetDependencyTree::Write(IOutputStream& rFile, EIDLength IDLength) const
{
    CDependencyTree::Write(rFile, IDLength);
    rFile.WriteLong(mCharacterOffsets.size());

    for (u32 iChar = 0; iChar < mCharacterOffsets.size(); iChar++)
        rFile.WriteLong( mCharacterOffsets[iChar] );
}

void CAnimSetDependencyTree::AddCharacter(const SSetCharacter *pkChar, const std::set<CAssetID>& rkBaseUsedSet)
{
    mCharacterOffsets.push_back( NumDependencies() );
    if (!pkChar) return;

    std::set<CAssetID> UsedSet = rkBaseUsedSet;
    AddCharDependency(pkChar->pModel, UsedSet);
    AddCharDependency(pkChar->pSkeleton, UsedSet);
    AddCharDependency(pkChar->pSkin, UsedSet);

    const std::vector<CAssetID> *pkParticleVectors[5] = {
        &pkChar->GenericParticles, &pkChar->ElectricParticles,
        &pkChar->SwooshParticles, &pkChar->SpawnParticles,
        &pkChar->EffectParticles
    };

    for (u32 iVec = 0; iVec < 5; iVec++)
    {
        for (u32 iPart = 0; iPart < pkParticleVectors[iVec]->size(); iPart++)
            AddCharDependency(pkParticleVectors[iVec]->at(iPart), UsedSet);
    }

    AddCharDependency(pkChar->IceModel, UsedSet);
    AddCharDependency(pkChar->IceSkin, UsedSet);
}

void CAnimSetDependencyTree::AddCharDependency(const CAssetID& rkID, std::set<CAssetID>& rUsedSet)
{
    if (rkID.IsValid() && rUsedSet.find(rkID) == rUsedSet.end())
    {
        rUsedSet.insert(rkID);
        AddDependency(rkID, false);
    }
}

void CAnimSetDependencyTree::AddCharDependency(CResource *pRes, std::set<CAssetID>& rUsedSet)
{
    if (!pRes) return;
    AddCharDependency(pRes->ID(), rUsedSet);
}

// ************ CScriptInstanceDependencyTree ************
CScriptInstanceDependencyTree::~CScriptInstanceDependencyTree()
{
    for (u32 iDep = 0; iDep < mDependencies.size(); iDep++)
        delete mDependencies[iDep];
}

EDependencyNodeType CScriptInstanceDependencyTree::Type() const
{
    return eDNT_ScriptInstance;
}

void CScriptInstanceDependencyTree::Read(IInputStream& rFile, EIDLength IDLength)
{
    mObjectType = rFile.ReadLong();
    u32 NumDepends = rFile.ReadLong();
    mDependencies.reserve(NumDepends);

    for (u32 iDep = 0; iDep < NumDepends; iDep++)
    {
        CAssetID ID(rFile, IDLength);
        CResourceEntry *pEntry = gpResourceStore->FindEntry(ID);

        if (pEntry && pEntry->ResourceType() == eAnimSet && pEntry->Game() <= eEchoes)
        {
            CAnimSetDependency *pSet = new CAnimSetDependency();
            pSet->SetID(ID);
            pSet->SetUsedChar( rFile.ReadLong() );
            mDependencies.push_back(pSet);
        }

        else
        {
            CResourceDependency *pRes = new CResourceDependency(ID);
            mDependencies.push_back(pRes);
        }
    }
}

void CScriptInstanceDependencyTree::Write(IOutputStream& rFile, EIDLength IDLength) const
{
    rFile.WriteLong(mObjectType);
    rFile.WriteLong(mDependencies.size());

    for (u32 iDep = 0; iDep < mDependencies.size(); iDep++)
        mDependencies[iDep]->Write(rFile, IDLength);
}

bool CScriptInstanceDependencyTree::HasDependency(const CAssetID& rkID)
{
    if (!rkID.IsValid()) return false;

    for (u32 iDep = 0; iDep < mDependencies.size(); iDep++)
    {
        CResourceDependency *pDep = mDependencies[iDep];
        if (pDep->ID() == rkID) return true;
    }

    return false;
}

CAssetID CScriptInstanceDependencyTree::DependencyByIndex(u32 Index) const
{
    ASSERT(Index >= 0 && Index < mDependencies.size());
    return mDependencies[Index]->ID();
}

// Static
CScriptInstanceDependencyTree* CScriptInstanceDependencyTree::BuildTree(CScriptObject *pInstance)
{
    CScriptInstanceDependencyTree *pTree = new CScriptInstanceDependencyTree();
    pTree->mObjectType = pInstance->ObjectTypeID();
    ParseStructDependencies(pTree, pInstance->Properties());
    return pTree;
}

void CScriptInstanceDependencyTree::ParseStructDependencies(CScriptInstanceDependencyTree *pTree, CPropertyStruct *pStruct)
{
    for (u32 iProp = 0; iProp < pStruct->Count(); iProp++)
    {
        IProperty *pProp = pStruct->PropertyByIndex(iProp);

        if (pProp->Type() == eStructProperty || pProp->Type() == eArrayProperty)
            ParseStructDependencies(pTree, static_cast<CPropertyStruct*>(pProp));

        else if (pProp->Type() == eFileProperty)
        {
            CAssetID ID = static_cast<TFileProperty*>(pProp)->Get().ID();

            if (ID.IsValid() && !pTree->HasDependency(ID))
            {
                CResourceDependency *pDep = new CResourceDependency(ID);
                pTree->mDependencies.push_back(pDep);
            }
        }

        else if (pProp->Type() == eCharacterProperty)
        {
            TCharacterProperty *pChar = static_cast<TCharacterProperty*>(pProp);
            CAssetID ID = pChar->Get().ID();

            if (ID.IsValid() && !pTree->HasDependency(ID))
                pTree->mDependencies.push_back( CAnimSetDependency::BuildDependency(pChar) );
        }
    }
}

// ************ CAreaDependencyTree ************
CAreaDependencyTree::~CAreaDependencyTree()
{
    for (u32 iInst = 0; iInst < mScriptInstances.size(); iInst++)
        delete mScriptInstances[iInst];
}

EDependencyNodeType CAreaDependencyTree::Type() const
{
    return eDNT_Area;
}

void CAreaDependencyTree::Read(IInputStream& rFile, EIDLength IDLength)
{
    // Base dependency list contains non-script dependencies (world geometry textures + PATH/PTLA/EGMC)
    CDependencyTree::Read(rFile, IDLength);
    u32 NumScriptInstances = rFile.ReadLong();
    mScriptInstances.reserve(NumScriptInstances);

    for (u32 iInst = 0; iInst < NumScriptInstances; iInst++)
    {
        CScriptInstanceDependencyTree *pInst = new CScriptInstanceDependencyTree;
        pInst->Read(rFile, IDLength);
        mScriptInstances.push_back(pInst);
    }

    u32 NumLayers = rFile.ReadLong();
    mLayerOffsets.reserve(NumLayers);

    for (u32 iLyr = 0; iLyr < NumLayers; iLyr++)
        mLayerOffsets.push_back( rFile.ReadLong() );
}

void CAreaDependencyTree::Write(IOutputStream& rFile, EIDLength IDLength) const
{
    CDependencyTree::Write(rFile, IDLength);
    rFile.WriteLong(mScriptInstances.size());

    for (u32 iInst = 0; iInst < mScriptInstances.size(); iInst++)
        mScriptInstances[iInst]->Write(rFile, IDLength);

    rFile.WriteLong(mLayerOffsets.size());

    for (u32 iLyr = 0; iLyr < mLayerOffsets.size(); iLyr++)
        rFile.WriteLong(mLayerOffsets[iLyr]);
}

void CAreaDependencyTree::AddScriptLayer(CScriptLayer *pLayer)
{
    if (!pLayer) return;
    mLayerOffsets.push_back(mScriptInstances.size());

    for (u32 iInst = 0; iInst < pLayer->NumInstances(); iInst++)
    {
        CScriptInstanceDependencyTree *pTree = CScriptInstanceDependencyTree::BuildTree( pLayer->InstanceByIndex(iInst) );
        ASSERT(pTree != nullptr);

        if (pTree->NumDependencies() > 0)
            mScriptInstances.push_back(pTree);
        else
            delete pTree;
    }
}

CScriptInstanceDependencyTree* CAreaDependencyTree::ScriptInstanceByIndex(u32 Index) const
{
    ASSERT(Index >= 0 && Index < mScriptInstances.size());
    return mScriptInstances[Index];
}

void CAreaDependencyTree::GetModuleDependencies(EGame Game, std::vector<TString>& rModuleDepsOut, std::vector<u32>& rModuleLayerOffsetsOut) const
{
    CMasterTemplate *pMaster = CMasterTemplate::MasterForGame(Game);

    // Output module list will be split per-script layer
    // The output offset list contains two offsets per layer - start index and end index
    for (u32 iLayer = 0; iLayer < mLayerOffsets.size(); iLayer++)
    {
        u32 StartIdx = mLayerOffsets[iLayer];
        u32 EndIdx = (iLayer == mLayerOffsets.size() - 1 ? mScriptInstances.size() : mLayerOffsets[iLayer + 1]);

        u32 ModuleStartIdx = rModuleDepsOut.size();
        rModuleLayerOffsetsOut.push_back(ModuleStartIdx);

        // Keep track of which types we've already checked on this layer to speed things up a little...
        std::set<u32> UsedObjectTypes;

        for (u32 iInst = StartIdx; iInst < EndIdx; iInst++)
        {
            CScriptInstanceDependencyTree *pInst = mScriptInstances[iInst];
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
