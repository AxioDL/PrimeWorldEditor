#include "CDependencyTree.h"
#include "Core/Resource/Script/CMasterTemplate.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/Resource/Script/CScriptObject.h"

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

void CDependencyTree::Read(IInputStream& rFile, EIDLength IDLength)
{
    mRootID = CAssetID(rFile, IDLength);

    u32 NumDepends = rFile.ReadLong();
    mChildren.reserve(NumDepends);

    for (u32 iDep = 0; iDep < NumDepends; iDep++)
    {
        CResourceDependency *pDepend = new CResourceDependency;
        pDepend->Read(rFile, IDLength);
        mChildren.push_back(pDepend);
    }
}

void CDependencyTree::Write(IOutputStream& rFile) const
{
    mRootID.Write(rFile);
    rFile.WriteLong( mChildren.size() );

    for (u32 iDep = 0; iDep < mChildren.size(); iDep++)
        mChildren[iDep]->Write(rFile);
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
    mChildren.push_back(pDepend);
}

// ************ CResourceDependency ************
EDependencyNodeType CResourceDependency::Type() const
{
    return eDNT_ResourceDependency;
}

void CResourceDependency::Read(IInputStream& rFile, EIDLength IDLength)
{
    mID = CAssetID(rFile, IDLength);
}

void CResourceDependency::Write(IOutputStream& rFile) const
{
    mID.Write(rFile);
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

void CPropertyDependency::Read(IInputStream& rFile, EIDLength IDLength)
{
    mIDString = rFile.ReadString();
    CResourceDependency::Read(rFile, IDLength);
}

void CPropertyDependency::Write(IOutputStream& rFile) const
{
    rFile.WriteString(mIDString.ToStdString());
    CResourceDependency::Write(rFile);
}

// ************ CCharacterPropertyDependency ************
EDependencyNodeType CCharPropertyDependency::Type() const
{
    return eDNT_CharacterProperty;
}

void CCharPropertyDependency::Read(IInputStream& rFile, EIDLength IDLength)
{
    CPropertyDependency::Read(rFile, IDLength);
    mUsedChar = rFile.ReadLong();
}

void CCharPropertyDependency::Write(IOutputStream& rFile) const
{
    CPropertyDependency::Write(rFile);
    rFile.WriteLong(mUsedChar);
}

// ************ CScriptInstanceDependency ************
EDependencyNodeType CScriptInstanceDependency::Type() const
{
    return eDNT_ScriptInstance;
}

void CScriptInstanceDependency::Read(IInputStream& rFile, EIDLength IDLength)
{
    mObjectType = rFile.ReadLong();
    u32 NumProperties = rFile.ReadLong();
    mChildren.reserve(NumProperties);

    for (u32 iProp = 0; iProp < NumProperties; iProp++)
    {
        bool IsCharacter = rFile.ReadBool();
        CPropertyDependency *pProp = (IsCharacter ? new CCharPropertyDependency() : new CPropertyDependency());
        pProp->Read(rFile, IDLength);
        mChildren.push_back(pProp);
    }
}

void CScriptInstanceDependency::Write(IOutputStream& rFile) const
{
    rFile.WriteLong(mObjectType);
    rFile.WriteLong(mChildren.size());

    for (u32 iProp = 0; iProp < mChildren.size(); iProp++)
    {
        CPropertyDependency *pProp = static_cast<CPropertyDependency*>(mChildren[iProp]);
        rFile.WriteBool( pProp->Type() == eDNT_CharacterProperty );
        pProp->Write(rFile);
    }
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

        else if (Type == eFileProperty)
        {
            CAssetID ID = static_cast<TFileProperty*>(pProp)->Get().ID();

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

void CAnimSetDependencyTree::Write(IOutputStream& rFile) const
{
    CDependencyTree::Write(rFile);
    rFile.WriteLong(mCharacterOffsets.size());

    for (u32 iChar = 0; iChar < mCharacterOffsets.size(); iChar++)
        rFile.WriteLong( mCharacterOffsets[iChar] );
}

void CAnimSetDependencyTree::AddCharacter(const SSetCharacter *pkChar, const std::set<CAssetID>& rkBaseUsedSet)
{
    mCharacterOffsets.push_back( NumChildren() );
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

// ************ CAreaDependencyTree ************
EDependencyNodeType CAreaDependencyTree::Type() const
{
    return eDNT_Area;
}

void CAreaDependencyTree::Read(IInputStream& rFile, EIDLength IDLength)
{
    mRootID = CAssetID(rFile, IDLength);

    // Base dependency list contains non-script dependencies (world geometry textures + PATH/PTLA/EGMC)
    u32 NumBaseDependencies = rFile.ReadLong();
    mChildren.reserve(NumBaseDependencies);

    for (u32 iDep = 0; iDep < NumBaseDependencies; iDep++)
    {
        CResourceDependency *pDep = new CResourceDependency;
        pDep->Read(rFile, IDLength);
        mChildren.push_back(pDep);
    }

    u32 NumScriptInstances = rFile.ReadLong();
    mChildren.reserve(mChildren.size() + NumScriptInstances);

    for (u32 iInst = 0; iInst < NumScriptInstances; iInst++)
    {
        CScriptInstanceDependency *pInst = new CScriptInstanceDependency;
        pInst->Read(rFile, IDLength);
        mChildren.push_back(pInst);
    }

    u32 NumLayers = rFile.ReadLong();
    mLayerOffsets.reserve(NumLayers);

    for (u32 iLyr = 0; iLyr < NumLayers; iLyr++)
        mLayerOffsets.push_back( rFile.ReadLong() );
}

void CAreaDependencyTree::Write(IOutputStream& rFile) const
{
    mRootID.Write(rFile);

    u32 NumBaseDependencies = (mLayerOffsets.empty() ? mChildren.size() : mLayerOffsets.front());
    rFile.WriteLong(NumBaseDependencies);

    for (u32 iDep = 0; iDep < NumBaseDependencies; iDep++)
        mChildren[iDep]->Write(rFile);

    rFile.WriteLong(mChildren.size() - NumBaseDependencies);

    for (u32 iDep = NumBaseDependencies; iDep < mChildren.size(); iDep++)
        mChildren[iDep]->Write(rFile);

    rFile.WriteLong(mLayerOffsets.size());

    for (u32 iLyr = 0; iLyr < mLayerOffsets.size(); iLyr++)
        rFile.WriteLong(mLayerOffsets[iLyr]);
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
