#include "Core/GameProject/AssetNameGeneration.h"

#include "Core/NRangeUtils.h"
#include "Core/GameProject/CGameProject.h"
#include "Core/GameProject/CPackage.h"
#include "Core/Resource/CAudioGroup.h"
#include "Core/Resource/CAudioMacro.h"
#include "Core/Resource/CDependencyGroup.h"
#include "Core/Resource/CFont.h"
#include "Core/Resource/CWorld.h"
#include "Core/Resource/Animation/CAnimSet.h"
#include "Core/Resource/Scan/CScan.h"
#include "Core/Resource/Scan/SScanParametersMP1.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/Resource/Script/CScriptTemplate.h"
#include <Common/FileUtil.h>
#include <Common/Log.h>

#include <algorithm>
#include <fmt/format.h>
#include <ranges>

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

static void ApplyGeneratedName(CResourceEntry* pEntry, const TString& rkDir, const TString& rkName)
{
    ASSERT(pEntry != nullptr);

    // Don't overwrite hand-picked names and directories with auto-generated ones
    const bool HasCustomDir = !pEntry->HasFlag(EResEntryFlag::AutoResDir);
    const bool HasCustomName = !pEntry->HasFlag(EResEntryFlag::AutoResName);
    if (HasCustomDir && HasCustomName)
        return;

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
        if (SanitizedName.IsEmpty())
            return;

        // Find an unused variant of this name
        NewName = SanitizedName;
        int AppendNum = 0;

        while (const CResourceEntry *pConflict = pNewDir->FindChildResource(NewName, pEntry->ResourceType()))
        {
            if (pConflict == pEntry)
                return;

            NewName = fmt::format("{}_{}", SanitizedName, AppendNum);
            AppendNum++;
        }
    }

    // Check if we're actually moving anything
    if (pEntry->Directory() == pNewDir && pEntry->Name() == NewName) return;

    // Perform the move
    [[maybe_unused]] const bool Success = pEntry->MoveAndRename(pNewDir->FullPath(), NewName, true, true);
    ASSERT(Success);
}

static void ProcessPackages(const CGameProject* proj, CResourceStore* store)
{
#if PROCESS_PACKAGES
    // Generate names for package named resources
    NLog::Debug("Processing packages");

    for (const auto& pkg : proj->Packages())
    {
        for (const auto& res : pkg->NamedResources())
        {
            if (res.Name.EndsWith("NODEPEND"))
                continue;

            // Some of Retro's paks reference assets that don't exist, so we need this check here.
            if (CResourceEntry* pRes = store->FindEntry(res.ID))
                ApplyGeneratedName(pRes, pkg->Name(), res.Name);
        }
    }
#endif
}

static void ProcessWorlds(const CGameProject* proj, CResourceStore* store)
{
#if PROCESS_WORLDS
    // Generate world/area names
    NLog::Debug("Processing worlds");
    const TString kWorldsRoot = "Worlds/";

    for (const auto& It : store->MakeTypedResourceView(EResourceType::World))
    {
        // Set world name
        TResPtr<CWorld> pWorld = It->Load();
        const TString& WorldName = pWorld->Name();
        const TString WorldDir = kWorldsRoot + WorldName + '/';

        const TString WorldMasterName = "!" + WorldName + "_Master";
        const TString WorldMasterDir = WorldDir + WorldMasterName + '/';
        ApplyGeneratedName(It.get(), WorldMasterDir, WorldMasterName);

        // Move world stuff
        const TString WorldNamesDir = "Strings/Worlds/General/";
        const TString AreaNamesDir = fmt::format("Strings/Worlds/{}/", WorldName);

        CModel* pSkyModel = pWorld->DefaultSkybox();
        CStringTable* pWorldNameTable = pWorld->NameString();
        CStringTable* pDarkWorldNameTable = pWorld->DarkNameString();
        CResource* pSaveWorld = pWorld->SaveWorld();
        CResource* pMapWorld = pWorld->MapWorld();

        if (pSaveWorld)
            ApplyGeneratedName(pSaveWorld->Entry(), WorldMasterDir, WorldMasterName);

        if (pMapWorld)
            ApplyGeneratedName(pMapWorld->Entry(), WorldMasterDir, WorldMasterName);

        if (pSkyModel && !pSkyModel->Entry()->IsCategorized())
        {
            // Move sky model
            CResourceEntry* pSkyEntry = pSkyModel->Entry();
            ApplyGeneratedName(pSkyEntry, WorldDir + "sky/cooked/", WorldName + "_sky");

            // Move sky textures
            for (size_t iSet = 0; iSet < pSkyModel->GetMatSetCount(); iSet++)
            {
                const CMaterialSet* pSet = pSkyModel->GetMatSet(iSet);

                for (size_t mat = 0; mat < pSet->NumMaterials(); mat++)
                {
                    const CMaterial* pMat = pSet->MaterialByIndex(mat, true);

                    for (const auto* pass : pMat->Passes())
                    {
                        if (pass->Texture())
                            ApplyGeneratedName(pass->Texture()->Entry(), WorldDir + "sky/sourceimages/", pass->Texture()->Entry()->Name());
                    }
                }
            }
        }

        if (pWorldNameTable)
        {
            CResourceEntry* pNameEntry = pWorldNameTable->Entry();
            ApplyGeneratedName(pNameEntry, WorldNamesDir, WorldName);
        }

        if (pDarkWorldNameTable)
        {
            CResourceEntry* pDarkNameEntry = pDarkWorldNameTable->Entry();
            ApplyGeneratedName(pDarkNameEntry, WorldNamesDir, WorldName + "Dark");
        }

        // Areas
        for (size_t iArea = 0; iArea < pWorld->NumAreas(); iArea++)
        {
            // Determine area name
            TString AreaName = pWorld->AreaInternalName(iArea);
            const CAssetID& AreaID = pWorld->AreaResourceID(iArea);

            if (AreaName.IsEmpty())
                AreaName = AreaID.ToString();

            // Rename area stuff
            CResourceEntry* pAreaEntry = store->FindEntry(AreaID);
            // Some DKCR worlds reference areas that don't exist
            if (!pAreaEntry)
                continue;
            ApplyGeneratedName(pAreaEntry, WorldMasterDir, AreaName);

            CStringTable* pAreaNameTable = pWorld->AreaName(iArea);
            if (pAreaNameTable)
                ApplyGeneratedName(pAreaNameTable->Entry(), AreaNamesDir, AreaName);

            if (pMapWorld)
            {
                const auto* pGroup = dynamic_cast<CDependencyGroup*>(pMapWorld);
                ASSERT(pGroup != nullptr);

                const CAssetID& MapID = pGroup->Dependencies()[iArea];
                CResourceEntry* pMapEntry = store->FindEntry(MapID);
                ASSERT(pMapEntry != nullptr);

                ApplyGeneratedName(pMapEntry, WorldMasterDir, AreaName);
            }

#if PROCESS_AREAS
            // Move area dependencies
            const TString AreaCookedDir = WorldDir + AreaName + "/cooked/";
            const auto* pArea = static_cast<CGameArea*>(pAreaEntry->Load());

            // Area lightmaps
            uint32_t LightmapNum = 0;
            const CMaterialSet* pMaterials = pArea->Materials();

            for (size_t iMat = 0; iMat < pMaterials->NumMaterials(); iMat++)
            {
                const CMaterial* pMat = pMaterials->MaterialByIndex(iMat, true);
                bool FoundLightmap = false;

                for (auto [idx, pass] : Utils::enumerate(pMat->Passes()))
                {
                    const bool IsLightmap = ((pArea->Game() <= EGame::Echoes && pMat->Options().HasFlag(EMaterialOption::Lightmap) && idx == 0) ||
                                             (pArea->Game() >= EGame::CorruptionProto && pass->Type() == CFourCC("DIFF")));
                    const bool IsBloomLightmap = (pArea->Game() >= EGame::CorruptionProto && pass->Type() == CFourCC("BLOL"));

                    TString TexName;

                    if (IsLightmap)
                    {
                        TexName = fmt::format("{}_lit_lightmap{}", AreaName, LightmapNum);
                    }
                    else if (IsBloomLightmap)
                    {
                        TexName = fmt::format("{}_lit_lightmap_bloom{}", AreaName, LightmapNum);
                    }

                    if (!TexName.IsEmpty())
                    {
                        CTexture* pLightmapTex = pass->Texture();
                        CResourceEntry* pTexEntry = pLightmapTex->Entry();
                        if (pTexEntry->IsCategorized())
                            continue;

                        ApplyGeneratedName(pTexEntry, AreaCookedDir, TexName);
                        pTexEntry->SetHidden(true);
                        FoundLightmap = true;
                    }
                }

                if (FoundLightmap)
                    LightmapNum++;
            }

            // Generate names from script instance names
            for (const auto* layer : pArea->ScriptLayers())
            {
                for (const auto* inst : layer->Instances())
                {
                    CStructProperty* pProperties = inst->Template()->Properties();

                    if (inst->ObjectTypeID() == 0x42 || inst->ObjectTypeID() == CFourCC("POIN").ToU32())
                    {
                        const TString Name = inst->InstanceName();

                        if (Name.StartsWith("POI_", false))
                        {
                            const TIDString ScanIDString = (proj->Game() <= EGame::Prime ? "0x4:0x0" : "0xBDBEC295:0xB94E9BE7");
                            const auto* pScanProperty = TPropCast<CAssetProperty>(pProperties->ChildByIDString(ScanIDString));
                            ASSERT(pScanProperty); // Temporary assert to remind myself later to update this code when uncooked properties are added to the template

                            if (pScanProperty)
                            {
                                const CAssetID ScanID = pScanProperty->Value(inst->PropertyData());
                                CResourceEntry* pEntry = store->FindEntry(ScanID);

                                if (pEntry && !pEntry->IsNamed())
                                {
                                    TString ScanName = Name.ChopFront(4);

                                    if (ScanName.EndsWith(".SCAN", false))
                                        ScanName = ScanName.ChopBack(5);

                                    ApplyGeneratedName(pEntry, pEntry->DirectoryPath(), ScanName);

                                    auto* pScan = static_cast<CScan*>(pEntry->Load());
                                    if (pScan)
                                    {
                                        const CAssetID StringID = pScan->ScanStringPropertyRef();
                                        CResourceEntry* pStringEntry = store->FindEntry(StringID);

                                        if (pStringEntry)
                                        {
                                            ApplyGeneratedName(pStringEntry, pStringEntry->DirectoryPath(), ScanName);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else if (inst->ObjectTypeID() == 0x17 || inst->ObjectTypeID() == CFourCC("MEMO").ToU32())
                    {
                        const TString Name = inst->InstanceName();

                        if (Name.EndsWith(".STRG", false))
                        {
                            const uint32_t StringPropID = (proj->Game() <= EGame::Prime ? 0x4 : 0x9182250C);
                            const auto* pStringProperty = TPropCast<CAssetProperty>(pProperties->ChildByID(StringPropID));
                            ASSERT(pStringProperty); // Temporary assert to remind myself later to update this code when uncooked properties are added to the template

                            if (pStringProperty)
                            {
                                const CAssetID StringID = pStringProperty->Value(inst->PropertyData());
                                CResourceEntry* pEntry = store->FindEntry(StringID);

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
                    else if (inst->ObjectTypeID() == 0x0 || inst->ObjectTypeID() == CFourCC("ACTR").ToU32() ||
                             inst->ObjectTypeID() == 0x8 || inst->ObjectTypeID() == CFourCC("PLAT").ToU32())
                    {
                        const uint32_t ModelPropID = (proj->Game() <= EGame::Prime ? (inst->ObjectTypeID() == 0x0 ? 0xA : 0x6) : 0xC27FFA8F);
                        const auto* pModelProperty = TPropCast<CAssetProperty>(pProperties->ChildByID(ModelPropID));
                        ASSERT(pModelProperty); // Temporary assert to remind myself later to update this code when uncooked properties are added to the template

                        if (pModelProperty)
                        {
                            const CAssetID ModelID = pModelProperty->Value(inst->PropertyData());
                            CResourceEntry* pEntry = store->FindEntry(ModelID);

                            if (pEntry && !pEntry->IsCategorized())
                            {
                                auto* pModel = static_cast<CModel*>(pEntry->Load());

                                if (pModel->IsLightmapped())
                                    ApplyGeneratedName(pEntry, AreaCookedDir, pEntry->Name());
                            }
                        }
                    }
                }
            }

            // Other area assets
            CResourceEntry* pPathEntry = store->FindEntry(pArea->PathID());
            CResourceEntry* pPoiMapEntry = pArea->PoiToWorldMap() ? pArea->PoiToWorldMap()->Entry() : nullptr;
            CResourceEntry* pPortalEntry = store->FindEntry(pArea->PortalAreaID());

            if (pPathEntry)
                ApplyGeneratedName(pPathEntry, WorldMasterDir, AreaName);

            if (pPoiMapEntry)
                ApplyGeneratedName(pPoiMapEntry, WorldMasterDir, AreaName);

            if (pPortalEntry)
                ApplyGeneratedName(pPortalEntry, WorldMasterDir, AreaName);

            store->DestroyUnreferencedResources();
#endif
        }
    }
#endif
}

static void ProcessModels(CResourceStore* store)
{
#if PROCESS_MODELS
    // Generate Model Lightmap names
    NLog::Debug("Processing model lightmaps");

    for (const auto& It : store->MakeTypedResourceView(EResourceType::Model))
    {
        auto* pModel = static_cast<CModel*>(It->Load());
        size_t LightmapNum = 0;

        for (size_t iSet = 0; iSet < pModel->GetMatSetCount(); iSet++)
        {
            const CMaterialSet* pSet = pModel->GetMatSet(iSet);

            for (size_t iMat = 0; iMat < pSet->NumMaterials(); iMat++)
            {
                const CMaterial* pMat = pSet->MaterialByIndex(iMat, true);

                for (auto [idx, pass] : Utils::enumerate(pMat->Passes()))
                {
                    const bool IsLightmap = (pMat->Version() <= EGame::Echoes && pMat->Options().HasFlag(EMaterialOption::Lightmap) && idx == 0) ||
                                            (pMat->Version() >= EGame::CorruptionProto && pass->Type() == CFourCC("DIFF"));

                    if (IsLightmap)
                    {
                        CTexture* pLightmapTex = pass->Texture();
                        CResourceEntry* pTexEntry = pLightmapTex->Entry();
                        if (pTexEntry->IsNamed() || pTexEntry->IsCategorized())
                            continue;

                        const TString TexName = fmt::format("{}_lightmap{}", It->Name(), LightmapNum);
                        ApplyGeneratedName(pTexEntry, pModel->Entry()->DirectoryPath(), TexName);
                        pTexEntry->SetHidden(true);
                        LightmapNum++;
                    }
                }
            }
        }

        store->DestroyUnreferencedResources();
    }
#endif
}

static void ProcessAudioGroups(CResourceStore* store)
{
#if PROCESS_AUDIO_GROUPS
    // Generate Audio Group names
    NLog::Debug("Processing audio groups");
    const TString kAudioGrpDir = "Audio/";

    for (const auto& entry : store->MakeTypedResourceView(EResourceType::AudioGroup))
    {
        const auto* pGroup = static_cast<const CAudioGroup*>(entry->Load());
        const TString& GroupName = pGroup->GroupName();
        ApplyGeneratedName(entry.get(), kAudioGrpDir, GroupName);
    }
#endif
}

static void ProcessAudioMacros(CResourceStore* store)
{
#if PROCESS_AUDIO_MACROS
    // Process audio macro/sample names
    NLog::Debug("Processing audio macros");
    const TString kSfxDir = "Audio/Uncategorized/";

    for (const auto& It : store->MakeTypedResourceView(EResourceType::AudioMacro))
    {
        const auto* pMacro = static_cast<const CAudioMacro*>(It->Load());
        const TString& MacroName = pMacro->MacroName();
        ApplyGeneratedName(It.get(), kSfxDir, MacroName);

        for (const auto [idx, SampleID] : std::views::enumerate(pMacro->Samples()))
        {
            auto* pSample = store->FindEntry(SampleID);
            if (pSample == nullptr || pSample->IsNamed())
                continue;

            TString SampleName;
            if (pMacro->NumSamples() == 1)
                SampleName = MacroName;
            else
                SampleName = fmt::format("{}_{}", MacroName, idx);

            ApplyGeneratedName(pSample, kSfxDir, SampleName);
        }
    }
#endif
}

static void ProcessAnimCharSets(const CGameProject* proj, CResourceStore* store)
{
#if PROCESS_ANIM_CHAR_SETS
    // Generate animation format names
    // Hacky syntax because animsets are under eAnimSet in MP1/2 and eCharacter in MP3/DKCR
    NLog::Debug("Processing animation data");
    auto View = proj->Game() <= EGame::Echoes ? store->MakeTypedResourceView(EResourceType::AnimSet)
                                              : store->MakeTypedResourceView(EResourceType::Character);
    for (const auto& It : View)
    {
        const TString SetDir = It->DirectoryPath();
        const auto* pSet = static_cast<CAnimSet*>(It->Load());
        TString NewSetName;

        for (auto&& [idx, character] : Utils::enumerate(pSet->Characters()))
        {
            const TString& CharName = character.Name;

            if (idx == 0)
                NewSetName = CharName;

            if (character.pModel)
                ApplyGeneratedName(character.pModel->Entry(), SetDir, CharName);
            if (character.pSkeleton)
                ApplyGeneratedName(character.pSkeleton->Entry(), SetDir, CharName);
            if (character.pSkin)
                ApplyGeneratedName(character.pSkin->Entry(), SetDir, CharName);

            if (proj->Game() >= EGame::CorruptionProto && proj->Game() <= EGame::Corruption && character.ID == 0)
            {
                CResourceEntry* pAnimDataEntry = store->FindEntry(character.AnimDataID);

                if (pAnimDataEntry)
                {
                    const TString AnimDataName = fmt::format("{}_animdata", CharName);
                    ApplyGeneratedName(pAnimDataEntry, SetDir, AnimDataName);
                }
            }

            for (const auto& rkOverlay : character.OverlayModels)
            {
                if (rkOverlay.ModelID.IsValid() || rkOverlay.SkinID.IsValid())
                {
                    const TString TypeName = (rkOverlay.Type == EOverlayType::Frozen    ? "frozen"
                                            : rkOverlay.Type == EOverlayType::Acid      ? "acid"
                                            : rkOverlay.Type == EOverlayType::Hypermode ? "hypermode"
                                            : rkOverlay.Type == EOverlayType::XRay      ? "xray"
                                            : "");
                    ASSERT(TypeName != "");

                    const TString OverlayName = fmt::format("{}_{}", CharName, TypeName);

                    if (rkOverlay.ModelID.IsValid())
                    {
                        CResourceEntry* pModelEntry = store->FindEntry(rkOverlay.ModelID);
                        ApplyGeneratedName(pModelEntry, SetDir, OverlayName);
                    }
                    if (rkOverlay.SkinID.IsValid())
                    {
                        CResourceEntry* pSkinEntry = store->FindEntry(rkOverlay.SkinID);
                        ApplyGeneratedName(pSkinEntry, SetDir, OverlayName);
                    }
                }
            }
        }

        if (!NewSetName.IsEmpty())
            ApplyGeneratedName(It.get(), SetDir, NewSetName);

        std::set<CAnimPrimitive> AnimPrimitives;
        pSet->GetUniquePrimitives(AnimPrimitives);

        for (const auto& rkPrim : AnimPrimitives)
        {
            CAnimation* pAnim = rkPrim.Animation();

            if (pAnim != nullptr)
            {
                ApplyGeneratedName(pAnim->Entry(), SetDir, rkPrim.Name());
                CAnimEventData* pEvents = pAnim->EventData();

                if (pEvents != nullptr)
                    ApplyGeneratedName(pEvents->Entry(), SetDir, rkPrim.Name());
            }
        }
    }
#endif
}

static void ProcessStrings(CResourceStore* store)
{
#if PROCESS_STRINGS
    // Generate string names
    NLog::Debug("Processing strings");
    const TString kStringsDir = "Strings/Uncategorized/";

    for (const auto& It : store->MakeTypedResourceView(EResourceType::StringTable))
    {
        if (It->IsNamed())
            continue;

        auto* pString = static_cast<CStringTable*>(It->Load());
        TString String;

        for (size_t iStr = 0; iStr < pString->NumStrings() && String.IsEmpty(); iStr++)
            String = CStringTable::StripFormatting(pString->GetString(ELanguage::English, iStr)).Trimmed();

        if (!String.IsEmpty())
        {
            TString Name = String.SubString(0, std::min(String.Size(), size_t{50})).Trimmed();
            Name.Replace("\n", " ");

            while (Name.EndsWith(".") || TString::IsWhitespace(Name.Back()))
                Name = Name.ChopBack(1);

            ApplyGeneratedName(pString->Entry(), kStringsDir, Name);
        }
    }
#endif
}

static void ProcessScans(const CGameProject* proj, CResourceStore* store)
{
#if PROCESS_SCANS
    // Generate scan names
    NLog::Debug("Processing scans");
    for (const auto& It : store->MakeTypedResourceView(EResourceType::Scan))
    {
        if (It->IsNamed())
            continue;

        auto* pScan = static_cast<CScan*>(It->Load());
        TString ScanName;

        if (ScanName.IsEmpty())
        {
            const CAssetID StringID = pScan->ScanStringPropertyRef().Get();
            if (const auto* pString = static_cast<const CStringTable*>(store->LoadResource(StringID, EResourceType::StringTable)))
                ScanName = pString->Entry()->Name();
        }

        ApplyGeneratedName(pScan->Entry(), It->DirectoryPath(), ScanName);

        if (!ScanName.IsEmpty() && proj->Game() <= EGame::Prime)
        {
            const auto& kParms = *static_cast<const SScanParametersMP1*>(pScan->ScanData().DataPointer());

            if (CResourceEntry* pEntry = store->FindEntry(kParms.GuiFrame))
                ApplyGeneratedName(pEntry, pEntry->DirectoryPath(), "ScanFrame");

            for (size_t iImg = 0; iImg < kParms.ScanImages.size(); iImg++)
            {
                const CAssetID ImageID = kParms.ScanImages[iImg].Texture;
                if (CResourceEntry* pImgEntry = store->FindEntry(ImageID))
                    ApplyGeneratedName(pImgEntry, pImgEntry->DirectoryPath(), fmt::format("{}_Image{}", ScanName, iImg));
            }
        }
    }
#endif
}

static void ProcessFonts(CResourceStore* store)
{
#if PROCESS_FONTS
    // Generate font names
    NLog::Debug("Processing fonts");
    for (const auto& It : store->MakeTypedResourceView(EResourceType::Font))
    {
        if (auto* pFont = static_cast<CFont*>(It->Load()))
        {
            ApplyGeneratedName(pFont->Entry(), pFont->Entry()->DirectoryPath(), pFont->FontName());

            if (CTexture* pFontTex = pFont->Texture())
                ApplyGeneratedName(pFontTex->Entry(), pFont->Entry()->DirectoryPath(), pFont->Entry()->Name() + "_tex");
        }
    }
#endif
}

static void RevertAutoNames(CResourceStore* store)
{
#if REVERT_AUTO_NAMES
    // Revert all auto-generated asset names back to default to prevent name conflicts resulting in inconsistent results.
    NLog::Debug("Reverting auto-generated names");

    for (auto& resource : store->MakeResourceView())
    {
        const bool HasCustomDir = !resource->HasFlag(EResEntryFlag::AutoResDir);
        const bool HasCustomName = !resource->HasFlag(EResEntryFlag::AutoResName);
        if (HasCustomDir && HasCustomName)
            continue;

        const TString NewDir = (HasCustomDir ? resource->DirectoryPath() : store->DefaultResourceDirPath());
        const TString NewName = (HasCustomName ? resource->Name() : resource->ID().ToString());
        resource->MoveAndRename(NewDir, NewName, true, true);
    }
#endif
}

void GenerateAssetNames(CGameProject* proj)
{
    NLog::Debug("*** Generating Asset Names ***");
    CResourceStore* store = proj->ResourceStore();

    RevertAutoNames(store);

    ProcessPackages(proj, store);
    ProcessWorlds(proj, store);
    ProcessModels(store);
    ProcessAudioGroups(store);
    ProcessAudioMacros(store);
    ProcessAnimCharSets(proj, store);
    ProcessStrings(store);
    ProcessScans(proj, store);
    ProcessFonts(store);

    store->RootDirectory()->DeleteEmptySubdirectories();
    store->ConditionalSaveStore();
    NLog::Debug("*** Asset Name Generation FINISHED ***");
}
