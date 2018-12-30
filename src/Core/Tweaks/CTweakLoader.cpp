#include "CTweakLoader.h"
#include "Core/Resource/Factory/CScriptLoader.h"
#include "Core/Resource/Script/NGameList.h"

CTweakData* CTweakLoader::LoadCTWK(IInputStream& CTWK, CResourceEntry* pEntry)
{
    // Find the correct template based on the asset ID.
    static const std::unordered_map<uint, const char*> skIdToTemplateName =
    {
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
        { 0xF1ED8FD7, "TweakPlayerControls", }
    };

    auto Find = skIdToTemplateName.find( pEntry->ID().ToLong() );
    ASSERT( Find != skIdToTemplateName.end() );
    const char* pkTemplateName = Find->second;

    // Fetch template
    CGameTemplate* pGameTemplate = NGameList::GetGameTemplate( pEntry->Game() );
    ASSERT( pGameTemplate != nullptr );

    CScriptTemplate* pTweakTemplate = pGameTemplate->FindMiscTemplate(pkTemplateName);
    ASSERT( pTweakTemplate != nullptr );

    // Load tweak data
    CTweakData* pTweakData = new CTweakData(pTweakTemplate, pEntry);
    CScriptLoader::LoadStructData( CTWK, pTweakData->TweakData() );

    // Verify
    if (!CTWK.EoF() && CTWK.PeekShort() != -1)
    {
        errorf("%s: unread property data, tweak template may be malformed (%d bytes left)", *CTWK.GetSourceString(), CTWK.Size() - CTWK.Tell());
        delete pTweakData;
        return nullptr;
    }

    return pTweakData;
}

void CTweakLoader::LoadNTWK(IInputStream& NTWK, std::vector<CTweakData*>& OutTweaks)
{
    // Unimplemented
}
