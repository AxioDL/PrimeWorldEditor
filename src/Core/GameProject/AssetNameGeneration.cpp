#include "AssetNameGeneration.h"
#include "CGameProject.h"
#include "CResourceIterator.h"
#include "Core/Resource/CAudioMacro.h"
#include "Core/Resource/CFont.h"
#include "Core/Resource/CScan.h"
#include "Core/Resource/CWorld.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include <Math/MathUtil.h>

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

void ApplyGeneratedName(CResourceEntry *pEntry, const TWideString& rkDir, const TWideString& rkName)
{
    ASSERT(pEntry != nullptr);
    TWideString SanitizedName = FileUtil::SanitizeName(rkName, false);
    TWideString SanitizedDir = FileUtil::SanitizePath(rkDir, true);
    if (SanitizedName.IsEmpty()) return;

    // trying to keep these as consistent with Retro's naming scheme as possible, and
    // for some reason in MP3 they started using all lowercase folder names...
    if (pEntry->Game() >= eCorruptionProto)
        SanitizedDir = SanitizedDir.ToLower();

    CVirtualDirectory *pNewDir = pEntry->ResourceStore()->GetVirtualDirectory(SanitizedDir, false, true);
    if (pEntry->Directory() == pNewDir && pEntry->Name() == SanitizedName) return;

    TWideString Name = SanitizedName;
    int AppendNum = 0;

    while (pNewDir->FindChildResource(Name, pEntry->ResourceType()) != nullptr)
    {
        Name = TWideString::Format(L"%s_%d", *SanitizedName, AppendNum);
        AppendNum++;
    }

    bool Success = pEntry->Move(SanitizedDir, Name);
    ASSERT(Success);
}

void GenerateAssetNames(CGameProject *pProj)
{
    CResourceStore *pStore = pProj->ResourceStore();

#if PROCESS_PACKAGES
    // Generate names for package named resources
    for (u32 iPkg = 0; iPkg < pProj->NumPackages(); iPkg++)
    {
        CPackage *pPkg = pProj->PackageByIndex(iPkg);

        for (u32 iRes = 0; iRes < pPkg->NumNamedResources(); iRes++)
        {
            const SNamedResource& rkRes = pPkg->NamedResourceByIndex(iRes);
            if (rkRes.Name.EndsWith("NODEPEND")) continue;

            // Some of Retro's paks reference assets that don't exist, so we need this check here.
            CResourceEntry *pRes = pStore->FindEntry(rkRes.ID);

            if (pRes)
                ApplyGeneratedName(pRes, pPkg->Name().ToUTF16(), rkRes.Name.ToUTF16());
        }
    }
#endif

#if PROCESS_WORLDS
    // Generate world/area names
    const TWideString kWorldsRoot = L"Worlds\\";

    for (TResourceIterator<eWorld> It(pStore); It; ++It)
    {
        // Set world name
        CWorld *pWorld = (CWorld*) It->Load();
        TWideString WorldName = L'!' + pWorld->Name().ToUTF16() + L"_Master";
        TWideString WorldDir = kWorldsRoot + WorldName + L'\\';

        TWideString WorldMasterName = L"!" + WorldName + L"_Master";
        TWideString WorldMasterDir = WorldDir + WorldMasterName + L'\\';
        ApplyGeneratedName(*It, WorldMasterDir, WorldMasterName);

        // Move world stuff
        const TWideString WorldNamesDir = L"Strings\\Worlds\\General\\";
        const TWideString AreaNamesDir = TWideString::Format(L"Strings\\Worlds\\%s\\", *WorldName);

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
            ApplyGeneratedName(pSkyEntry, WorldDir + L"sky\\cooked\\", WorldName + L"_" + L"sky");

            // Move sky textures
            for (u32 iSet = 0; iSet < pSkyModel->GetMatSetCount(); iSet++)
            {
                CMaterialSet *pSet = pSkyModel->GetMatSet(iSet);

                for (u32 iMat = 0; iMat < pSet->NumMaterials(); iMat++)
                {
                    CMaterial *pMat = pSet->MaterialByIndex(iMat);

                    for (u32 iPass = 0; iPass < pMat->PassCount(); iPass++)
                    {
                        CMaterialPass *pPass = pMat->Pass(iPass);

                        if (pPass->Texture())
                            ApplyGeneratedName(pPass->Texture()->Entry(), WorldDir + L"sky\\sourceimages\\", pPass->Texture()->Entry()->Name());
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
            ApplyGeneratedName(pDarkNameEntry, WorldNamesDir, WorldName + L"Dark");
        }

        // Areas
        for (u32 iArea = 0; iArea < pWorld->NumAreas(); iArea++)
        {
            // Determine area name
            TWideString AreaName = pWorld->AreaInternalName(iArea).ToUTF16();
            CAssetID AreaID = pWorld->AreaResourceID(iArea);

            if (AreaName.IsEmpty())
                AreaName = AreaID.ToString().ToUTF16();

            // Rename area stuff
            CResourceEntry *pAreaEntry = pStore->FindEntry(AreaID);
            ASSERT(pAreaEntry != nullptr);
            ApplyGeneratedName(pAreaEntry, WorldMasterDir, AreaName);

            CStringTable *pAreaNameTable = pWorld->AreaName(iArea);
            if (pAreaNameTable)
                ApplyGeneratedName(pAreaNameTable->Entry(), AreaNamesDir, AreaName);

            if (pMapWorld)
            {
                ASSERT(pMapWorld->Type() == eDependencyGroup);
                CDependencyGroup *pGroup = static_cast<CDependencyGroup*>(pMapWorld);
                CAssetID MapID = pGroup->DependencyByIndex(iArea);
                CResourceEntry *pMapEntry = pStore->FindEntry(MapID);
                ASSERT(pMapEntry != nullptr);

                ApplyGeneratedName(pMapEntry, WorldMasterDir, AreaName);
            }

#if PROCESS_AREAS
            // Move area dependencies
            TWideString AreaCookedDir = WorldDir + AreaName + L"\\cooked\\";
            CGameArea *pArea = (CGameArea*) pAreaEntry->Load();

            // Area lightmaps
            u32 LightmapNum = 0;
            CMaterialSet *pMaterials = pArea->Materials();

            for (u32 iMat = 0; iMat < pMaterials->NumMaterials(); iMat++)
            {
                CMaterial *pMat = pMaterials->MaterialByIndex(iMat);

                if (pMat->Options().HasFlag(CMaterial::eLightmap))
                {
                    CTexture *pLightmapTex = pMat->Pass(0)->Texture();
                    CResourceEntry *pTexEntry = pLightmapTex->Entry();
                    if (pTexEntry->IsCategorized()) continue;

                    TWideString TexName = TWideString::Format(L"%s_lit_lightmap%d", *AreaName, LightmapNum);
                    ApplyGeneratedName(pTexEntry, AreaCookedDir, TexName);
                    pTexEntry->SetHidden(true);
                    LightmapNum++;
                }
            }

            // Generate names from script instance names
            for (u32 iLyr = 0; iLyr < pArea->NumScriptLayers(); iLyr++)
            {
                CScriptLayer *pLayer = pArea->ScriptLayer(iLyr);

                for (u32 iInst = 0; iInst < pLayer->NumInstances(); iInst++)
                {
                    CScriptObject *pInst = pLayer->InstanceByIndex(iInst);

                    if (pInst->ObjectTypeID() == 0x42 || pInst->ObjectTypeID() == FOURCC('POIN'))
                    {
                        TString Name = pInst->InstanceName();

                        if (Name.StartsWith("POI_", false))
                        {
                            TIDString ScanIDString = (pProj->Game() <= ePrime ? "0x4:0x0" : "0xBDBEC295:0xB94E9BE7");
                            TAssetProperty *pScanProperty = TPropCast<TAssetProperty>(pInst->PropertyByIDString(ScanIDString));
                            ASSERT(pScanProperty); // Temporary assert to remind myself later to update this code when uncooked properties are added to the template

                            if (pScanProperty)
                            {
                                CAssetID ScanID = pScanProperty->Get();
                                CResourceEntry *pEntry = pStore->FindEntry(ScanID);

                                if (pEntry && !pEntry->IsNamed())
                                {
                                    TWideString ScanName = Name.ToUTF16().ChopFront(4);

                                    if (ScanName.EndsWith(L".SCAN", false))
                                        ScanName = ScanName.ChopBack(5);

                                    ApplyGeneratedName(pEntry, pEntry->DirectoryPath(), ScanName);

                                    CScan *pScan = (CScan*) pEntry->Load();
                                    if (pScan && pScan->ScanText())
                                    {
                                        CResourceEntry *pStringEntry = pScan->ScanText()->Entry();
                                        ApplyGeneratedName(pStringEntry, pStringEntry->DirectoryPath(), ScanName);
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
                            u32 StringPropID = (pProj->Game() <= ePrime ? 0x4 : 0x9182250C);
                            TAssetProperty *pStringProperty = TPropCast<TAssetProperty>(pInst->Properties()->PropertyByID(StringPropID));
                            ASSERT(pStringProperty); // Temporary assert to remind myself later to update this code when uncooked properties are added to the template

                            if (pStringProperty)
                            {
                                CAssetID StringID = pStringProperty->Get();
                                CResourceEntry *pEntry = pStore->FindEntry(StringID);

                                if (pEntry && !pEntry->IsNamed())
                                {
                                    TWideString StringName = Name.ToUTF16().ChopBack(5);

                                    if (StringName.StartsWith(L"HUDMemo - "))
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
                        u32 ModelPropID = (pProj->Game() <= ePrime ? (pInst->ObjectTypeID() == 0x0 ? 0xA : 0x6) : 0xC27FFA8F);
                        TAssetProperty *pModelProperty = TPropCast<TAssetProperty>(pInst->Properties()->PropertyByID(ModelPropID));
                        ASSERT(pModelProperty); // Temporary assert to remind myself later to update this code when uncooked properties are added to the template

                        if (pModelProperty)
                        {
                            CAssetID ModelID = pModelProperty->Get();
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
    for (TResourceIterator<eModel> It(pStore); It; ++It)
    {
        CModel *pModel = (CModel*) It->Load();
        u32 LightmapNum = 0;

        for (u32 iSet = 0; iSet < pModel->GetMatSetCount(); iSet++)
        {
            CMaterialSet *pSet = pModel->GetMatSet(iSet);

            for (u32 iMat = 0; iMat < pSet->NumMaterials(); iMat++)
            {
                CMaterial *pMat = pSet->MaterialByIndex(iMat);

                if (pMat->Options().HasFlag(CMaterial::eLightmap))
                {
                    CTexture *pLightmapTex = pMat->Pass(0)->Texture();
                    CResourceEntry *pTexEntry = pLightmapTex->Entry();
                    if (pTexEntry->IsNamed() || pTexEntry->IsCategorized()) continue;

                    TWideString TexName = TWideString::Format(L"%s_lightmap%d", *It->Name(), LightmapNum);
                    ApplyGeneratedName(pTexEntry, pModel->Entry()->DirectoryPath(), TexName);
                    pTexEntry->SetHidden(true);
                    LightmapNum++;
                }
            }
        }

        pStore->DestroyUnreferencedResources();
    }
#endif

#if PROCESS_AUDIO_GROUPS
    // Generate Audio Group names
    const TWideString kAudioGrpDir = L"Audio\\";

    for (TResourceIterator<eAudioGroup> It(pStore); It; ++It)
    {
        CAudioGroup *pGroup = (CAudioGroup*) It->Load();
        TWideString GroupName = pGroup->GroupName().ToUTF16();
        ApplyGeneratedName(*It, kAudioGrpDir, GroupName);
    }
#endif

#if PROCESS_AUDIO_MACROS
    // Process audio macro/sample names
    const TWideString kSfxDir = L"Audio\\Uncategorized\\";

    for (TResourceIterator<eAudioMacro> It(pStore); It; ++It)
    {
        CAudioMacro *pMacro = (CAudioMacro*) It->Load();
        TWideString MacroName = pMacro->MacroName().ToUTF16();
        ApplyGeneratedName(*It, kSfxDir, MacroName);

        for (u32 iSamp = 0; iSamp < pMacro->NumSamples(); iSamp++)
        {
            CAssetID SampleID = pMacro->SampleByIndex(iSamp);
            CResourceEntry *pSample = pStore->FindEntry(SampleID);

            if (pSample && !pSample->IsNamed())
                ApplyGeneratedName(pSample, kSfxDir, TWideString::Format(L"%s_sample%d", *MacroName, iSamp));
        }
    }
#endif

#if PROCESS_ANIM_CHAR_SETS
    // Generate animation format names
    for (TResourceIterator<eAnimSet> It(pStore); It; ++It)
    {
        TWideString SetDir = It->DirectoryPath();
        TWideString NewSetName;
        CAnimSet *pSet = (CAnimSet*) It->Load();

        for (u32 iChar = 0; iChar < pSet->NumCharacters(); iChar++)
        {
            const SSetCharacter *pkChar = pSet->Character(iChar);

            TWideString CharName = pkChar->Name.ToUTF16();
            if (iChar == 0) NewSetName = CharName;

            if (pkChar->pModel)     ApplyGeneratedName(pkChar->pModel->Entry(), SetDir, CharName);
            if (pkChar->pSkeleton)  ApplyGeneratedName(pkChar->pSkeleton->Entry(), SetDir, CharName);
            if (pkChar->pSkin)      ApplyGeneratedName(pkChar->pSkin->Entry(), SetDir, CharName);

            if (pkChar->IceModel.IsValid() || pkChar->IceSkin.IsValid())
            {
                TWideString IceName = TWideString::Format(L"%s_frozen", *CharName);

                if (pkChar->IceModel.IsValid())
                {
                    CResourceEntry *pIceModelEntry = pStore->FindEntry(pkChar->IceModel);
                    ApplyGeneratedName(pIceModelEntry, SetDir, IceName);
                }
                if (pkChar->IceSkin.IsValid())
                {
                    CResourceEntry *pIceSkinEntry = pStore->FindEntry(pkChar->IceSkin);
                    ApplyGeneratedName(pIceSkinEntry, SetDir, IceName);
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
                ApplyGeneratedName(pAnim->Entry(), SetDir, rkPrim.Name().ToUTF16());
                CAnimEventData *pEvents = pAnim->EventData();

                if (pEvents)
                    ApplyGeneratedName(pEvents->Entry(), SetDir, rkPrim.Name().ToUTF16());
            }
        }
    }
#endif

#if PROCESS_STRINGS
    // Generate string names
    const TWideString kStringsDir = L"Strings\\Uncategorized\\";

    for (TResourceIterator<eStringTable> It(pStore); It; ++It)
    {
        if (It->IsNamed()) continue;
        CStringTable *pString = (CStringTable*) It->Load();
        TWideString String;

        for (u32 iStr = 0; iStr < pString->NumStrings() && String.IsEmpty(); iStr++)
            String = CStringTable::StripFormatting( pString->String("ENGL", iStr) ).Trimmed();

        if (!String.IsEmpty())
        {
            TWideString Name = String.SubString(0, Math::Min<u32>(String.Size(), 50)).Trimmed();
            Name.Replace(L"\n", L" ");

            while (Name.EndsWith(L".") || TWideString::IsWhitespace(Name.Back()))
                Name = Name.ChopBack(1);

            ApplyGeneratedName(pString->Entry(), kStringsDir, Name);
        }
    }
#endif

#if PROCESS_SCANS
    // Generate scan names
    for (TResourceIterator<eScan> It(pStore); It; ++It)
    {
        if (It->IsNamed()) continue;
        CScan *pScan = (CScan*) It->Load();
        TWideString ScanName;

        if (pProj->Game() >= eEchoesDemo)
        {
            CAssetID DisplayAsset = pScan->LogbookDisplayAssetID();
            CResourceEntry *pEntry = pStore->FindEntry(DisplayAsset);
            if (pEntry && pEntry->IsNamed()) ScanName = pEntry->Name();
        }

        if (ScanName.IsEmpty())
        {
            CStringTable *pString = pScan->ScanText();
            if (pString) ScanName = pString->Entry()->Name();
        }

        ApplyGeneratedName(pScan->Entry(), It->DirectoryPath(), ScanName);

        if (!ScanName.IsEmpty() && pProj->Game() <= ePrime)
        {
            CAssetID FrameID = pScan->GuiFrame();
            CResourceEntry *pEntry = pStore->FindEntry(FrameID);
            if (pEntry) ApplyGeneratedName(pEntry, pEntry->DirectoryPath(), L"ScanFrame");

            for (u32 iImg = 0; iImg < 4; iImg++)
            {
                CAssetID ImageID = pScan->ScanImage(iImg);
                CResourceEntry *pImgEntry = pStore->FindEntry(ImageID);
                if (pImgEntry) ApplyGeneratedName(pImgEntry, pImgEntry->DirectoryPath(), TWideString::Format(L"%s_Image%d", *ScanName, iImg));
            }
        }
    }
#endif

#if PROCESS_FONTS
    // Generate font names
    for (TResourceIterator<eFont> It(pStore); It; ++It)
    {
        CFont *pFont = (CFont*) It->Load();

        if (pFont)
        {
            ApplyGeneratedName(pFont->Entry(), pFont->Entry()->DirectoryPath(), pFont->FontName().ToUTF16());

            CTexture *pFontTex = pFont->Texture();

            if (pFontTex)
                ApplyGeneratedName(pFontTex->Entry(), pFont->Entry()->DirectoryPath(), pFont->Entry()->Name() + L"_tex");
        }
    }
#endif

    pStore->ConditionalSaveStore();
}
