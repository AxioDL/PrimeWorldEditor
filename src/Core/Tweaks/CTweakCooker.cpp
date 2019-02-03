#include "CTweakCooker.h"
#include "Core/Resource/Cooker/CScriptCooker.h"

/** Cooker entry point */
bool CTweakCooker::CookCTWK(CTweakData* pTweakData, IOutputStream& CTWK)
{
    CStructRef TweakProperties = pTweakData->TweakData();
    CScriptCooker ScriptCooker(pTweakData->Game());
    ScriptCooker.WriteProperty(CTWK, TweakProperties.Property(), TweakProperties.DataPointer(), true);
    return true;
}

bool CTweakCooker::CookNTWK(const std::vector<CTweakData*>& kTweaks, IOutputStream& NTWK)
{
    NTWK.WriteFourCC( FOURCC('NTWK') ); // NTWK magic
    NTWK.WriteByte( 1 );                // Version number; must be 1
    NTWK.WriteLong( kTweaks.size() );   // Number of tweak objects

    for (uint TweakIdx = 0; TweakIdx < kTweaks.size(); TweakIdx++)
    {
        CTweakData* pTweakData = kTweaks[TweakIdx];

        // Tweaks in MP2+ are saved with the script object data format
        // Write a dummy script object header here
        uint TweakObjectStart = NTWK.Tell();
        NTWK.WriteLong( pTweakData->TweakID() );    // Object ID
        NTWK.WriteShort( 0 );                       // Object size
        NTWK.WriteLong( TweakIdx );                 // Instance ID
        NTWK.WriteShort( 0 );                       // Link count

        CStructRef TweakProperties = pTweakData->TweakData();
        CScriptCooker ScriptCooker(TweakProperties.Property()->Game());
        ScriptCooker.WriteProperty(NTWK, TweakProperties.Property(), TweakProperties.DataPointer(), false);

        uint TweakObjectEnd = NTWK.Tell();
        uint TweakObjectSize = (uint16) (TweakObjectEnd - TweakObjectStart - 6);
        NTWK.GoTo(TweakObjectStart + 4);
        NTWK.WriteShort(TweakObjectSize);
        NTWK.GoTo(TweakObjectEnd);
    }

    NTWK.WriteToBoundary(32, 0);
    return true;
}
