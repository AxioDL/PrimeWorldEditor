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
    NTWK.WriteFourCC(FOURCC('NTWK'));                  // NTWK magic
    NTWK.WriteUByte(1);                                // Version number; must be 1
    NTWK.WriteULong(static_cast<uint32>(kTweaks.size())); // Number of tweak objects

    for (uint32 TweakIdx = 0; TweakIdx < kTweaks.size(); TweakIdx++)
    {
        const CTweakData* pTweakData = kTweaks[TweakIdx];

        // Tweaks in MP2+ are saved with the script object data format
        // Write a dummy script object header here
        const uint32 TweakObjectStart = NTWK.Tell();
        NTWK.WriteULong(pTweakData->TweakID());      // Object ID
        NTWK.WriteUShort(0);                         // Object size
        NTWK.WriteULong(TweakIdx);                   // Instance ID
        NTWK.WriteUShort(0);                         // Link count

        const CStructRef TweakProperties = pTweakData->TweakData();
        CScriptCooker ScriptCooker(TweakProperties.Property()->Game());
        ScriptCooker.WriteProperty(NTWK, TweakProperties.Property(), TweakProperties.DataPointer(), false);

        const uint32 TweakObjectEnd = NTWK.Tell();
        const uint16 TweakObjectSize = static_cast<uint16>(TweakObjectEnd - TweakObjectStart - 6);
        NTWK.GoTo(TweakObjectStart + 4);
        NTWK.WriteUShort(TweakObjectSize);
        NTWK.GoTo(TweakObjectEnd);
    }

    NTWK.WriteToBoundary(32, 0);
    return true;
}
