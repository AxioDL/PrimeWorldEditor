#include "Core/Tweaks/CTweakLoader.h"

#include "Core/Resource/Factory/CScriptLoader.h"
#include "Core/Resource/Script/CGameTemplate.h"
#include "Core/Resource/Script/NGameList.h"
#include "Core/Tweaks/CTweakData.h"

#include <Common/Log.h>

#include <unordered_map>

std::unique_ptr<CTweakData> CTweakLoader::LoadCTWK(IInputStream& CTWK, CResourceEntry* pEntry)
{
    // Find the correct template based on the asset ID.
    static const std::unordered_map<uint32_t, const char*> skIdToTemplateName{
        { 0x1D180D7C, "TweakParticle" },
        { 0x264A4972, "TweakPlayer" },
        { 0x33B3323A, "TweakGunRes" },
        { 0x39AD28D3, "TweakCameraBob" },
        { 0x3FAEC012, "TweakPlayerControls", },
        { 0x5ED56350, "TweakBall", },
        { 0x5F24EFF8, "TweakSlideShow", },
        { 0x6907A32D, "TweakPlayerGun", },
        { 0x85CA11E9, "TweakPlayerRes", },
        { 0x94C76ECD, "TweakTargeting", },
        { 0x953A7C63, "TweakGame", },
        { 0xC9954E56, "TweakGuiColors", },
        { 0xE66A4F86, "TweakAutoMapper", },
        { 0xED2E48A9, "TweakGui", },
        { 0xF1ED8FD7, "TweakPlayerControls", },
    };

    auto Find = skIdToTemplateName.find(pEntry->ID().ToU32());
    ASSERT(Find != skIdToTemplateName.end());
    const char* pkTemplateName = Find->second;

    // Fetch template
    CGameTemplate* pGameTemplate = NGameList::GetGameTemplate(pEntry->Game());
    ASSERT(pGameTemplate != nullptr);

    CScriptTemplate* pTweakTemplate = pGameTemplate->FindMiscTemplate(pkTemplateName);
    ASSERT(pTweakTemplate != nullptr);

    // Load tweak data
    auto pTweakData = std::make_unique<CTweakData>(pTweakTemplate, pEntry->ID().ToU32(), pEntry);
    CScriptLoader::LoadStructData(CTWK, pTweakData->TweakData(), pEntry->ResourceStore());

    // Verify
    if (!CTWK.EoF() && CTWK.PeekS16() != -1)
    {
        NLog::Error("{}: unread property data, tweak template may be malformed ({} bytes left)", CTWK.GetSourceString(), CTWK.Size() - CTWK.Tell());
        return nullptr;
    }

    return pTweakData;
}

void CTweakLoader::LoadNTWK(IInputStream& NTWK, EGame Game, std::vector<CTweakData*>& OutTweaks, CResourceStore* resourceStore)
{
    // Validate file. NTWK basically embeds a bunch of tweak objects using the script layers
    // format, so it has the same version byte that script layers have.
    const auto Magic = NTWK.ReadFourCC();
    const auto LayerVersion = NTWK.ReadU8();

    if (Magic != CFourCC("NTWK"))
    {
        NLog::Error("Unrecognized NTWK magic: 0x{:08X}", Magic.ToU32());
        return;
    }

    if (LayerVersion != 1)
    {
        NLog::Error("Unrecognized layer version in NTWK: {}", LayerVersion);
        return;
    }

    CGameTemplate* pGameTemplate = NGameList::GetGameTemplate( Game );
    ASSERT(pGameTemplate != nullptr);

    // Start reading tweaks
    const auto NumTweaks = NTWK.ReadU32();

    for (uint32_t TweakIdx = 0; TweakIdx < NumTweaks; TweakIdx++)
    {
        // Find the correct template based on the tweak ID.
        static const std::unordered_map<CFourCC, const char*> skIdToTemplateName{
            {CFourCC("TWAC"), "TweakAdvancedControls"},
            {CFourCC("TWAM"), "TweakAutoMapper"},
            {CFourCC("TWBL"), "TweakBall"},
            {CFourCC("TWC2"), "TweakPlayerControls"},
            {CFourCC("TWCB"), "TweakCameraBob"},
            {CFourCC("TWCC"), "TweakGamecubeControls"},
            {CFourCC("TWCT"), "TweakControls"},
            {CFourCC("TWEC"), "TweakExpertControls"},
            {CFourCC("TWGM"), "TweakGame"},
            {CFourCC("TWGT"), "TweakGraphicalTransitions"},
            {CFourCC("TWGU"), "TweakGui"},
            {CFourCC("TWGC"), "TweakGuiColors"},
            {CFourCC("TWP2"), "TweakPlayer"},
            {CFourCC("TWPC"), "TweakPlayerControls"},
            {CFourCC("TWPG"), "TweakPlayerGun"},
            {CFourCC("TWPL"), "TweakPlayer"},
            {CFourCC("TWPM"), "TweakPlayerGun"},
            {CFourCC("TWPA"), "TweakParticle"},
            {CFourCC("TWPR"), "TweakPlayerRes"},
            {CFourCC("TWRC"), "TweakRevolutionControls"},
            {CFourCC("TWSS"), "TweakSlideShow"},
            {CFourCC("TWTG"), "TweakTargeting"},
        };

        const auto TweakID = NTWK.ReadFourCC();
        const auto TweakSize = NTWK.ReadU16();
        const auto NextTweak = NTWK.Tell() + TweakSize;

        auto Find = skIdToTemplateName.find(TweakID);

        if (Find == skIdToTemplateName.cend())
        {
            NLog::Error("Unrecognized tweak ID: {} (0x{:08X})", TweakID.ToString(), TweakID.ToU32());
            NTWK.GoTo(NextTweak);
            continue;
        }

        CScriptTemplate* pTweakTemplate = pGameTemplate->FindMiscTemplate(Find->second);
        ASSERT(pTweakTemplate != nullptr);

        // Load tweak data
        NTWK.Skip(0xC);
        auto* pTweakData = new CTweakData(pTweakTemplate, TweakID.ToU32());
        CScriptLoader::LoadStructData(NTWK, pTweakData->TweakData(), resourceStore);
        OutTweaks.push_back(pTweakData);

        NTWK.GoTo(NextTweak);
    }
}
