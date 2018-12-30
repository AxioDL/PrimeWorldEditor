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
    // Unimplemented
    return false;
}
