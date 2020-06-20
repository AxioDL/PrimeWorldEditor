#include "CTweakLoader.h"
#include "Core/Resource/Factory/CScriptLoader.h"
#include "Core/Resource/Script/NGameList.h"

std::unique_ptr<CTweakData> CTweakLoader::LoadCTWK(IInputStream& CTWK, CResourceEntry* pEntry)
{
    // Find the correct template based on the asset ID.
    static const std::unordered_map<uint32, const char*> skIdToTemplateName{
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

    auto Find = skIdToTemplateName.find(pEntry->ID().ToLong());
    ASSERT(Find != skIdToTemplateName.end());
    const char* pkTemplateName = Find->second;

    // Fetch template
    CGameTemplate* pGameTemplate = NGameList::GetGameTemplate(pEntry->Game());
    ASSERT(pGameTemplate != nullptr);

    CScriptTemplate* pTweakTemplate = pGameTemplate->FindMiscTemplate(pkTemplateName);
    ASSERT(pTweakTemplate != nullptr);

    // Load tweak data
    auto pTweakData = std::make_unique<CTweakData>(pTweakTemplate, pEntry->ID().ToLong(), pEntry);
    CScriptLoader::LoadStructData(CTWK, pTweakData->TweakData());

    // Verify
    if (!CTWK.EoF() && CTWK.PeekShort() != -1)
    {
        errorf("%s: unread property data, tweak template may be malformed (%d bytes left)", *CTWK.GetSourceString(), CTWK.Size() - CTWK.Tell());
        return nullptr;
    }

    return pTweakData;
}

void CTweakLoader::LoadNTWK(IInputStream& NTWK, EGame Game, std::vector<CTweakData*>& OutTweaks)
{
    // Validate file. NTWK basically embeds a bunch of tweak objects using the script layers
    // format, so it has the same version byte that script layers have.
    const uint32 Magic = NTWK.ReadULong();
    const uint8 LayerVersion = NTWK.ReadUByte();

    if (Magic != FOURCC('NTWK'))
    {
        errorf("Unrecognized NTWK magic: 0x%08X", Magic);
        return;
    }

    if (LayerVersion != 1)
    {
        errorf("Unrecognized layer version in NTWK: %d", LayerVersion);
        return;
    }

    CGameTemplate* pGameTemplate = NGameList::GetGameTemplate( Game );
    ASSERT(pGameTemplate != nullptr);

    // Start reading tweaks
    const uint32 NumTweaks = NTWK.ReadULong();

    for (uint32 TweakIdx = 0; TweakIdx < NumTweaks; TweakIdx++)
    {
        // Find the correct template based on the tweak ID.
        static const std::unordered_map<uint32, const char*> skIdToTemplateName{
            { FOURCC('TWAC'), "TweakAdvancedControls" },
            { FOURCC('TWAM'), "TweakAutoMapper" },
            { FOURCC('TWBL'), "TweakBall" },
            { FOURCC('TWC2'), "TweakPlayerControls" },
            { FOURCC('TWCB'), "TweakCameraBob" },
            { FOURCC('TWCC'), "TweakGamecubeControls" },
            { FOURCC('TWCT'), "TweakControls" },
            { FOURCC('TWEC'), "TweakExpertControls" },
            { FOURCC('TWGM'), "TweakGame" },
            { FOURCC('TWGT'), "TweakGraphicalTransitions" },
            { FOURCC('TWGU'), "TweakGui" },
            { FOURCC('TWGC'), "TweakGuiColors" },
            { FOURCC('TWP2'), "TweakPlayer" },
            { FOURCC('TWPC'), "TweakPlayerControls" },
            { FOURCC('TWPG'), "TweakPlayerGun" },
            { FOURCC('TWPL'), "TweakPlayer" },
            { FOURCC('TWPM'), "TweakPlayerGun" },
            { FOURCC('TWPA'), "TweakParticle" },
            { FOURCC('TWPR'), "TweakPlayerRes" },
            { FOURCC('TWRC'), "TweakRevolutionControls" },
            { FOURCC('TWSS'), "TweakSlideShow" },
            { FOURCC('TWTG'), "TweakTargeting" },
        };

        const uint32 TweakID = NTWK.ReadULong();
        const uint16 TweakSize = NTWK.ReadUShort();
        const uint32 NextTweak = NTWK.Tell() + TweakSize;

        auto Find = skIdToTemplateName.find(TweakID);

        if (Find == skIdToTemplateName.cend())
        {
            errorf("Unrecognized tweak ID: %s (0x%08X)", *CFourCC(TweakID).ToString(), TweakID);
            NTWK.GoTo(NextTweak);
            continue;
        }

        CScriptTemplate* pTweakTemplate = pGameTemplate->FindMiscTemplate(Find->second);
        ASSERT(pTweakTemplate != nullptr);

        // Load tweak data
        NTWK.Skip(0xC);
        auto* pTweakData = new CTweakData(pTweakTemplate, TweakID);
        CScriptLoader::LoadStructData(NTWK, pTweakData->TweakData());
        OutTweaks.push_back(pTweakData);

        NTWK.GoTo(NextTweak);
    }
}
