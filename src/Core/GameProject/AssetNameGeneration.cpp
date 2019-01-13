#include "AssetNameGeneration.h"
#include "CGameProject.h"
#include "CResourceIterator.h"
#include "Core/Resource/CAudioMacro.h"
#include "Core/Resource/CFont.h"
#include "Core/Resource/CWorld.h"
#include "Core/Resource/Animation/CAnimSet.h"
#include "Core/Resource/Scan/CScan.h"
#include "Core/Resource/Scan/SScanParametersMP1.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include <Common/Math/MathUtil.h>

#define REVERT_AUTO_NAMES 1
#define PROCESS_PACKAGES 1
#define PROCESS_WORLDS 1
#define PROCESS_AREAS 1
#define PROCESS_MODELS 1
#define PROCESS_AUDIO_GROUPS 1
#define PROCESS_AUDIO_MACROS 1
#define PROCESS_ANIM_CHAR_SETS 1
#define PROCESS_STRINGS 1
#define PROCESS_SCANS 1
#define PROCESS_FONTS 1

void ApplyGeneratedName(CResourceEntry *pEntry, const TString& rkDir, const TString& rkName)
{
    ASSERT(pEntry != nullptr);

    // Don't overwrite hand-picked names and directories with auto-generated ones
    bool HasCustomDir = !pEntry->HasFlag(EResEntryFlag::AutoResDir);
    bool HasCustomName = !pEntry->HasFlag(EResEntryFlag::AutoResName);
    if (HasCustomDir && HasCustomName) return;

    // Determine final directory to use
    CVirtualDirectory *pNewDir = nullptr;

    if (HasCustomDir)
    {
        pNewDir = pEntry->Directory();
    }
    else
    {
        TString SanitizedDir = FileUtil::SanitizePath(rkDir, true);

        // trying to keep these as consistent with Retro's naming scheme as possible, and
        // for some reason in MP3 they started using all lowercase folder names...
        if (pEntry->Game() >= EGame::CorruptionProto)
            SanitizedDir = SanitizedDir.ToLower();

        pNewDir = pEntry->ResourceStore()->GetVirtualDirectory(SanitizedDir, true);
    }

    // Determine final name to use
    TString NewName;

    if (HasCustomName)
    {
        NewName = pEntry->Name();
    }
    else
    {
        TString SanitizedName = FileUtil::SanitizeName(rkName, false);
        if (SanitizedName.IsEmpty()) return;

        // Find an unused variant of this name
        NewName = SanitizedName;
        int AppendNum = 0;

        while (CResourceEntry *pConflict = pNewDir->FindChildResource(NewName, pEntry->ResourceType()))
        {
            if (pConflict == pEntry)
                return;

            NewName = TString::Format("%s_%d", *SanitizedName, AppendNum);
            AppendNum++;
        }
    }

    // Check if we're actually moving anything
    if (pEntry->Directory() == pNewDir && pEntry->Name() == NewName) return;

    // Perform the move
    bool Success = pEntry->MoveAndRename(pNewDir->FullPath(), NewName, true, true);
    ASSERT(Success);
}

void GenerateAssetNames(CGameProject *pProj)
{
    debugf("*** Generating Asset Names ***");
    CResourceStore *pStore = pProj->ResourceStore();

#if REVERT_AUTO_NAMES
    // Revert all auto-generated asset names back to default to prevent name conflicts resulting in inconsistent results.
    debugf("Reverting auto-generated names");

    for (CResourceIterator It(pStore); It; ++It)
    {
        bool HasCustomDir = !It->HasFlag(EResEntryFlag::AutoResDir);
        bool HasCustomName = !It->HasFlag(EResEntryFlag::AutoResName);
        if (HasCustomDir && HasCustomName) continue;

        TString NewDir = (HasCustomDir ? It->DirectoryPath() : pStore->DefaultResourceDirPath());
        TString NewName = (HasCustomName ? It->Name() : It->ID().ToString());
        It->MoveAndRename(NewDir, NewName, true, true);
    }
#endif

#if PROCESS_PACKAGES
    // Generate names for package named resources
    debugf("Processing packages");

    for (uint32 iPkg = 0; iPkg < pProj->NumPackages(); iPkg++)
    {
        CPackage *pPkg = pProj->PackageByIndex(iPkg);

        for (uint32 iRes = 0; iRes < pPkg->NumNamedResources(); iRes++)
        {
            const SNamedResource& rkRes = pPkg->NamedResourceByIndex(iRes);
            if (rkRes.Name.EndsWith("NODEPEND")) continue;

            // Some of Retro's paks reference assets that don't exist, so we need this check here.
            CResourceEntry *pRes = pStore->FindEntry(rkRes.ID);

            if (pRes)
                ApplyGeneratedName(pRes, pPkg->Name(), rkRes.Name);
        }
    }
#endif

#if PROCESS_WORLDS
    // Generate world/area names
    debugf("Processing worlds");
    const TString kWorldsRoot = "Worlds/";

    for (TResourceIterator<EResourceType::World> It(pStore); It; ++It)
    {
        // Set world name
        TResPtr<CWorld> pWorld = It->Load();
        TString WorldName = pWorld->Name();
        TString WorldDir = kWorldsRoot + WorldName + '/';

        TString WorldMasterName = "!" + WorldName + "_Master";
        TString WorldMasterDir = WorldDir + WorldMasterName + '/';
        ApplyGeneratedName(*It, WorldMasterDir, WorldMasterName);

        // Move world stuff
        const TString WorldNamesDir = "Strings/Worlds/General/";
        const TString AreaNamesDir = TString::Format("Strings/Worlds/%s/", *WorldName);

        CModel *pSkyModel = pWorld->DefaultSkybox();
        CStringTable *pWorldNameTable = pWorld->NameString();
        CStringTable *pDarkWorldNameTable = pWorld->DarkNameString();
        CResource *pSaveWorld = pWorld->SaveWorld();
        CResource *pMapWorld = pWorld->MapWorld();

        if (pSaveWorld)
            ApplyGeneratedName(pSaveWorld->Entry(), WorldMasterDir, WorldMasterName);

        if (pMapWorld)
            ApplyGeneratedName(pMapWorld->Entry(), WorldMasterDir, WorldMasterName);

        if (pSkyModel && !pSkyModel->Entry()->IsCategorized())
        {
            // Move sky model
            CResourceEntry *pSkyEntry = pSkyModel->Entry();
            ApplyGeneratedName(pSkyEntry, WorldDir + "sky/cooked/", WorldName + "_sky");

            // Move sky textures
            for (uint32 iSet = 0; iSet < pSkyModel->GetMatSetCount(); iSet++)
            {
                CMaterialSet *pSet = pSkyModel->GetMatSet(iSet);

                for (uint32 iMat = 0; iMat < pSet->NumMaterials(); iMat++)
                {
                    CMaterial *pMat = pSet->MaterialByIndex(iMat);

                    for (uint32 iPass = 0; iPass < pMat->PassCount(); iPass++)
                    {
                        CMaterialPass *pPass = pMat->Pass(iPass);

                        if (pPass->Texture())
                            ApplyGeneratedName(pPass->Texture()->Entry(), WorldDir + "sky/sourceimages/", pPass->Texture()->Entry()->Name());
                    }
                }
            }
        }

        if (pWorldNameTable)
        {
            CResourceEntry *pNameEntry = pWorldNameTable->Entry();
            ApplyGeneratedName(pNameEntry, WorldNamesDir, WorldName);
        }

        if (pDarkWorldNameTable)
        {
            CResourceEntry *pDarkNameEntry = pDarkWorldNameTable->Entry();
            ApplyGeneratedName(pDarkNameEntry, WorldNamesDir, WorldName + "Dark");
        }

        // Areas
        for (uint32 iArea = 0; iArea < pWorld->NumAreas(); iArea++)
        {
            // Determine area name
            TString AreaName = pWorld->AreaInternalName(iArea);
            CAssetID AreaID = pWorld->AreaResourceID(iArea);

            if (AreaName.IsEmpty())
                AreaName = AreaID.ToString();

            // Rename area stuff
            CResourceEntry *pAreaEntry = pStore->FindEntry(AreaID);
            if (!pAreaEntry) continue; // Some DKCR worlds reference areas that don't exist
            ApplyGeneratedName(pAreaEntry, WorldMasterDir, AreaName);

            CStringTable *pAreaNameTable = pWorld->AreaName(iArea);
            if (pAreaNameTable)
                ApplyGeneratedName(pAreaNameTable->Entry(), AreaNamesDir, AreaName);

            if (pMapWorld)
            {
                CDependencyGroup *pGroup = dynamic_cast<CDependencyGroup*>(pMapWorld);
                ASSERT(pGroup != nullptr);

                CAssetID MapID = pGroup->DependencyByIndex(iArea);
                CResourceEntry *pMapEntry = pStore->FindEntry(MapID);
                ASSERT(pMapEntry != nullptr);

                ApplyGeneratedName(pMapEntry, WorldMasterDir, AreaName);
            }

#if PROCESS_AREAS
            // Move area dependencies
            TString AreaCookedDir = WorldDir + AreaName + "/cooked/";
            CGameArea *pArea = (CGameArea*) pAreaEntry->Load();

            // Area lightmaps
            uint32 LightmapNum = 0;
            CMaterialSet *pMaterials = pArea->Materials();

            for (uint32 iMat = 0; iMat < pMaterials->NumMaterials(); iMat++)
            {
                CMaterial *pMat = pMaterials->MaterialByIndex(iMat);
                bool FoundLightmap = false;

                for (uint32 iPass = 0; iPass < pMat->PassCount(); iPass++)
                {
                    CMaterialPass *pPass = pMat->Pass(iPass);

                    bool IsLightmap = ( (pArea->Game() <= EGame::Echoes && pMat->Options().HasFlag(EMaterialOption::Lightmap) && iPass == 0) ||
                                        (pArea->Game() >= EGame::CorruptionProto && pPass->Type() == "DIFF") );
                    bool IsBloomLightmap = (pArea->Game() >= EGame::CorruptionProto && pPass->Type() == "BLOL");

                    TString TexName;

                    if (IsLightmap)
                    {
                        TexName = TString::Format("%s_lit_lightmap%d", *AreaName, LightmapNum);
                    }
                    else if (IsBloomLightmap)
                    {
                        TexName = TString::Format("%s_lit_lightmap_bloom%d", *AreaName, LightmapNum);
                    }

                    if (!TexName.IsEmpty())
                    {
                        CTexture *pLightmapTex = pPass->Texture();
                        CResourceEntry *pTexEntry = pLightmapTex->Entry();
                        if (pTexEntry->IsCategorized()) continue;

                        ApplyGeneratedName(pTexEntry, AreaCookedDir, TexName);
                        pTexEntry->SetHidden(true);
                        FoundLightmap = true;
                    }
                }

                if (FoundLightmap)
                    LightmapNum++;
            }

            // Generate names from script instance names
            for (uint32 iLyr = 0; iLyr < pArea->NumScriptLayers(); iLyr++)
            {
                CScriptLayer *pLayer = pArea->ScriptLayer(iLyr);

                for (uint32 iInst = 0; iInst < pLayer->NumInstances(); iInst++)
                {
                    CScriptObject* pInst = pLayer->InstanceByIndex(iInst);
                    CStructProperty* pProperties = pInst->Template()->Properties();

                    if (pInst->ObjectTypeID() == 0x42 || pInst->ObjectTypeID() == FOURCC('POIN'))
                    {
                        TString Name = pInst->InstanceName();

                        if (Name.StartsWith("POI_", false))
                        {
                            TIDString ScanIDString = (pProj->Game() <= EGame::Prime ? "0x4:0x0" : "0xBDBEC295:0xB94E9BE7");
                            CAssetProperty *pScanProperty = TPropCast<CAssetProperty>(pProperties->ChildByIDString(ScanIDString));
                            ASSERT(pScanProperty); // Temporary assert to remind myself later to update this code when uncooked properties are added to the template

                            if (pScanProperty)
                            {
                                CAssetID ScanID = pScanProperty->Value(pInst->PropertyData());
                                CResourceEntry *pEntry = pStore->FindEntry(ScanID);

                                if (pEntry && !pEntry->IsNamed())
                                {
                                    TString ScanName = Name.ChopFront(4);

                                    if (ScanName.EndsWith(".SCAN", false))
                                        ScanName = ScanName.ChopBack(5);

                                    ApplyGeneratedName(pEntry, pEntry->DirectoryPath(), ScanName);

                                    CScan *pScan = (CScan*) pEntry->Load();
                                    if (pScan)
                                    {
                                        CAssetID StringID = pScan->ScanStringPropertyRef();
                                        CResourceEntry* pStringEntry = gpResourceStore->FindEntry(StringID);

                                        if (pStringEntry)
                                        {
                                            ApplyGeneratedName(pStringEntry, pStringEntry->DirectoryPath(), ScanName);
                                        }
                                    }
                                }
                            }
                        }
                    }

                    else if (pInst->ObjectTypeID() == 0x17 || pInst->ObjectTypeID() == FOURCC('MEMO'))
                    {
                        TString Name = pInst->InstanceName();

                        if (Name.EndsWith(".STRG", false))
                        {
                            uint32 StringPropID = (pProj->Game() <= EGame::Prime ? 0x4 : 0x9182250C);
                            CAssetProperty *pStringProperty = TPropCast<CAssetProperty>(pProperties->ChildByID(StringPropID));
                            ASSERT(pStringProperty); // Temporary assert to remind myself later to update this code when uncooked properties are added to the template

                            if (pStringProperty)
                            {
                                CAssetID StringID = pStringProperty->Value(pInst->PropertyData());
                                CResourceEntry *pEntry = pStore->FindEntry(StringID);

                                if (pEntry && !pEntry->IsNamed())
                                {
                                    TString StringName = Name.ChopBack(5);

                                    if (StringName.StartsWith("HUDMemo - "))
                                        StringName = StringName.ChopFront(10);

                                    ApplyGeneratedName(pEntry, pEntry->DirectoryPath(), StringName);
                                }
                            }
                        }
                    }

                    // Look for lightmapped models - these are going to be unique to this area
                    else if (pInst->ObjectTypeID() == 0x0 || pInst->ObjectTypeID() == FOURCC('ACTR') ||
                             pInst->ObjectTypeID() == 0x8 || pInst->ObjectTypeID() == FOURCC('PLAT'))
                    {
                        uint32 ModelPropID = (pProj->Game() <= EGame::Prime ? (pInst->ObjectTypeID() == 0x0 ? 0xA : 0x6) : 0xC27FFA8F);
                        CAssetProperty *pModelProperty = TPropCast<CAssetProperty>(pProperties->ChildByID(ModelPropID));
                        ASSERT(pModelProperty); // Temporary assert to remind myself later to update this code when uncooked properties are added to the template

                        if (pModelProperty)
                        {
                            CAssetID ModelID = pModelProperty->Value(pInst->PropertyData());
                            CResourceEntry *pEntry = pStore->FindEntry(ModelID);

                            if (pEntry && !pEntry->IsCategorized())
                            {
                                CModel *pModel = (CModel*) pEntry->Load();

                                if (pModel->IsLightmapped())
                                    ApplyGeneratedName(pEntry, AreaCookedDir, pEntry->Name());
                            }
                        }
                    }
                }
            }

            // Other area assets
            CResourceEntry *pPathEntry = pStore->FindEntry(pArea->PathID());
            CResourceEntry *pPoiMapEntry = pArea->PoiToWorldMap() ? pArea->PoiToWorldMap()->Entry() : nullptr;
            CResourceEntry *pPortalEntry = pStore->FindEntry(pArea->PortalAreaID());

            if (pPathEntry)
                ApplyGeneratedName(pPathEntry, WorldMasterDir, AreaName);

            if (pPoiMapEntry)
                ApplyGeneratedName(pPoiMapEntry, WorldMasterDir, AreaName);

            if (pPortalEntry)
                ApplyGeneratedName(pPortalEntry, WorldMasterDir, AreaName);

            pStore->DestroyUnreferencedResources();
#endif
        }
    }
#endif

#if PROCESS_MODELS
    // Generate Model Lightmap names
    debugf("Processing model lightmaps");

    for (TResourceIterator<EResourceType::Model> It(pStore); It; ++It)
    {
        CModel *pModel = (CModel*) It->Load();
        uint32 LightmapNum = 0;

        for (uint32 iSet = 0; iSet < pModel->GetMatSetCount(); iSet++)
        {
            CMaterialSet *pSet = pModel->GetMatSet(iSet);

            for (uint32 iMat = 0; iMat < pSet->NumMaterials(); iMat++)
            {
                CMaterial *pMat = pSet->MaterialByIndex(iMat);

                for (uint32 iPass = 0; iPass < pMat->PassCount(); iPass++)
                {
                    CMaterialPass *pPass = pMat->Pass(iPass);

                    bool IsLightmap = ( (pMat->Version() <= EGame::Echoes && pMat->Options().HasFlag(EMaterialOption::Lightmap) && iPass == 0) ||
                                        (pMat->Version() >= EGame::CorruptionProto && pPass->Type() == "DIFF") );

                    if (IsLightmap)
                    {
                        CTexture *pLightmapTex = pPass->Texture();
                        CResourceEntry *pTexEntry = pLightmapTex->Entry();
                        if (pTexEntry->IsNamed() || pTexEntry->IsCategorized()) continue;

                        TString TexName = TString::Format("%s_lightmap%d", *It->Name(), LightmapNum);
                        ApplyGeneratedName(pTexEntry, pModel->Entry()->DirectoryPath(), TexName);
                        pTexEntry->SetHidden(true);
                        LightmapNum++;
                    }
                }
            }
        }

        pStore->DestroyUnreferencedResources();
    }
#endif

#if PROCESS_AUDIO_GROUPS
    // Generate Audio Group names
    debugf("Processing audio groups");
    const TString kAudioGrpDir = "Audio/";

    for (TResourceIterator<EResourceType::AudioGroup> It(pStore); It; ++It)
    {
        CAudioGroup *pGroup = (CAudioGroup*) It->Load();
        TString GroupName = pGroup->GroupName();
        ApplyGeneratedName(*It, kAudioGrpDir, GroupName);
    }
#endif

#if PROCESS_AUDIO_MACROS
    // Process audio macro/sample names
    debugf("Processing audio macros");
    const TString kSfxDir = "Audio/Uncategorized/";

    for (TResourceIterator<EResourceType::AudioMacro> It(pStore); It; ++It)
    {
        CAudioMacro *pMacro = (CAudioMacro*) It->Load();
        TString MacroName = pMacro->MacroName();
        ApplyGeneratedName(*It, kSfxDir, MacroName);

        for (uint32 iSamp = 0; iSamp < pMacro->NumSamples(); iSamp++)
        {
            CAssetID SampleID = pMacro->SampleByIndex(iSamp);
            CResourceEntry *pSample = pStore->FindEntry(SampleID);

            if (pSample && !pSample->IsNamed())
            {
                TString SampleName;

                if (pMacro->NumSamples() == 1)
                    SampleName = MacroName;
                else
                    SampleName = TString::Format("%s_%d", *MacroName, iSamp);

                ApplyGeneratedName(pSample, kSfxDir, SampleName);
            }
        }
    }
#endif

#if PROCESS_ANIM_CHAR_SETS
    // Generate animation format names
    // Hacky syntax because animsets are under eAnimSet in MP1/2 and eCharacter in MP3/DKCR
    debugf("Processing animation data");
    CResourceIterator *pIter = (pProj->Game() <= EGame::Echoes ?
                                    (CResourceIterator*) new TResourceIterator<EResourceType::AnimSet> :
                                    (CResourceIterator*) new TResourceIterator<EResourceType::Character>);
    CResourceIterator& It = *pIter;

    for (; It; ++It)
    {
        TString SetDir = It->DirectoryPath();
        TString NewSetName;
        CAnimSet *pSet = (CAnimSet*) It->Load();

        for (uint32 iChar = 0; iChar < pSet->NumCharacters(); iChar++)
        {
            const SSetCharacter *pkChar = pSet->Character(iChar);

            TString CharName = pkChar->Name;
            if (iChar == 0) NewSetName = CharName;

            if (pkChar->pModel)     ApplyGeneratedName(pkChar->pModel->Entry(), SetDir, CharName);
            if (pkChar->pSkeleton)  ApplyGeneratedName(pkChar->pSkeleton->Entry(), SetDir, CharName);
            if (pkChar->pSkin)      ApplyGeneratedName(pkChar->pSkin->Entry(), SetDir, CharName);

            if (pProj->Game() >= EGame::CorruptionProto && pProj->Game() <= EGame::Corruption && pkChar->ID == 0)
            {
                CResourceEntry *pAnimDataEntry = gpResourceStore->FindEntry( pkChar->AnimDataID );

                if (pAnimDataEntry)
                {
                    TString AnimDataName = TString::Format("%s_animdata", *CharName);
                    ApplyGeneratedName(pAnimDataEntry, SetDir, AnimDataName);
                }
            }

            for (uint32 iOverlay = 0; iOverlay < pkChar->OverlayModels.size(); iOverlay++)
            {
                const SOverlayModel& rkOverlay = pkChar->OverlayModels[iOverlay];

                if (rkOverlay.ModelID.IsValid() || rkOverlay.SkinID.IsValid())
                {
                    TString TypeName = (
                                rkOverlay.Type == EOverlayType::Frozen ? "frozen" :
                                rkOverlay.Type == EOverlayType::Acid ? "acid" :
                                rkOverlay.Type == EOverlayType::Hypermode ? "hypermode" :
                                rkOverlay.Type == EOverlayType::XRay ? "xray" :
                                ""
                    );
                    ASSERT(TypeName != "");

                    TString OverlayName = TString::Format("%s_%s", *CharName, *TypeName);

                    if (rkOverlay.ModelID.IsValid())
                    {
                        CResourceEntry *pModelEntry = pStore->FindEntry(rkOverlay.ModelID);
                        ApplyGeneratedName(pModelEntry, SetDir, OverlayName);
                    }
                    if (rkOverlay.SkinID.IsValid())
                    {
                        CResourceEntry *pSkinEntry = pStore->FindEntry(rkOverlay.SkinID);
                        ApplyGeneratedName(pSkinEntry, SetDir, OverlayName);
                    }
                }
            }
        }

        if (!NewSetName.IsEmpty())
            ApplyGeneratedName(*It, SetDir, NewSetName);

        std::set<CAnimPrimitive> AnimPrimitives;
        pSet->GetUniquePrimitives(AnimPrimitives);

        for (auto It = AnimPrimitives.begin(); It != AnimPrimitives.end(); It++)
        {
            const CAnimPrimitive& rkPrim = *It;
            CAnimation *pAnim = rkPrim.Animation();

            if (pAnim)
            {
                ApplyGeneratedName(pAnim->Entry(), SetDir, rkPrim.Name());
                CAnimEventData *pEvents = pAnim->EventData();

                if (pEvents)
                    ApplyGeneratedName(pEvents->Entry(), SetDir, rkPrim.Name());
            }
        }
    }
    delete pIter;
#endif

#if PROCESS_STRINGS
    // Generate string names
    debugf("Processing strings");
    const TString kStringsDir = "Strings/Uncategorized/";

    for (TResourceIterator<EResourceType::StringTable> It(pStore); It; ++It)
    {
        if (It->IsNamed()) continue;
        CStringTable *pString = (CStringTable*) It->Load();
        TString String;

        for (uint32 iStr = 0; iStr < pString->NumStrings() && String.IsEmpty(); iStr++)
            String = CStringTable::StripFormatting( pString->GetString(ELanguage::English, iStr) ).Trimmed();

        if (!String.IsEmpty())
        {
            TString Name = String.SubString(0, Math::Min<uint32>(String.Size(), 50)).Trimmed();
            Name.Replace("\n", " ");

            while (Name.EndsWith(".") || TString::IsWhitespace(Name.Back()))
                Name = Name.ChopBack(1);

            ApplyGeneratedName(pString->Entry(), kStringsDir, Name);
        }
    }
#endif

#if PROCESS_SCANS
    // Generate scan names
    debugf("Processing scans");
    for (TResourceIterator<EResourceType::Scan> It(pStore); It; ++It)
    {
        if (It->IsNamed()) continue;
        CScan *pScan = (CScan*) It->Load();
        TString ScanName;

        if (ScanName.IsEmpty())
        {
            CAssetID StringID = pScan->ScanStringPropertyRef().Get();
            CStringTable *pString = (CStringTable*) gpResourceStore->LoadResource(StringID, EResourceType::StringTable);
            if (pString) ScanName = pString->Entry()->Name();
        }

        ApplyGeneratedName(pScan->Entry(), It->DirectoryPath(), ScanName);

        if (!ScanName.IsEmpty() && pProj->Game() <= EGame::Prime)
        {
            const SScanParametersMP1& kParms = *static_cast<SScanParametersMP1*>(pScan->ScanData().DataPointer());

            CResourceEntry *pEntry = pStore->FindEntry(kParms.GuiFrame);
            if (pEntry) ApplyGeneratedName(pEntry, pEntry->DirectoryPath(), "ScanFrame");

            for (uint32 iImg = 0; iImg < 4; iImg++)
            {
                CAssetID ImageID = kParms.ScanImages[iImg].Texture;
                CResourceEntry *pImgEntry = pStore->FindEntry(ImageID);
                if (pImgEntry) ApplyGeneratedName(pImgEntry, pImgEntry->DirectoryPath(), TString::Format("%s_Image%d", *ScanName, iImg));
            }
        }
    }
#endif

#if PROCESS_FONTS
    // Generate font names
    debugf("Processing fonts");
    for (TResourceIterator<EResourceType::Font> It(pStore); It; ++It)
    {
        CFont *pFont = (CFont*) It->Load();

        if (pFont)
        {
            ApplyGeneratedName(pFont->Entry(), pFont->Entry()->DirectoryPath(), pFont->FontName());

            CTexture *pFontTex = pFont->Texture();

            if (pFontTex)
                ApplyGeneratedName(pFontTex->Entry(), pFont->Entry()->DirectoryPath(), pFont->Entry()->Name() + "_tex");
        }
    }
#endif

    pStore->RootDirectory()->DeleteEmptySubdirectories();
    pStore->ConditionalSaveStore();
    debugf("*** Asset Name Generation FINISHED ***");
}
