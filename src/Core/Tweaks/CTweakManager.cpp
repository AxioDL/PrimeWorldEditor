#include "CTweakManager.h"
#include "Core/GameProject/CGameProject.h"
#include "Core/GameProject/CResourceIterator.h"

CTweakManager::CTweakManager(CGameProject* pInProject)
    : mpProject(pInProject)
{
}

void CTweakManager::LoadTweaks()
{
    // MP1 - Load all tweak assets into memory
    if (mpProject->Game() <= EGame::Prime)
    {
        for (TResourceIterator<EResourceType::Tweaks> It(mpProject->ResourceStore()); It; ++It)
        {
            CTweakData* pTweaks = (CTweakData*) It->Load();
            mTweakObjects.push_back(pTweaks);
        }
    }

    // MP2+ - Not supported, but tweaks are stored in Standard.ntwk
    else
    {
    }
}

void CTweakManager::SaveTweaks()
{
    // In MP1, to save an individual tweak asset, just call Tweak->Entry()->Save()
    // In MP2+, call this function.
    //@todo
}
