#include "CTweakManager.h"
#include "Core/GameProject/CGameProject.h"
#include "Core/GameProject/CResourceIterator.h"
#include "Core/Tweaks/CTweakLoader.h"

CTweakManager::CTweakManager(CGameProject* pInProject)
    : mpProject(pInProject)
{
}

void CTweakManager::LoadTweaks()
{
    ASSERT( mTweakObjects.empty() );

    // MP1 - Load all tweak assets into memory
    if (mpProject->Game() <= EGame::Prime)
    {
        for (TResourceIterator<EResourceType::Tweaks> It(mpProject->ResourceStore()); It; ++It)
        {
            CTweakData* pTweaks = (CTweakData*) It->Load();
            pTweaks->Lock();
            mTweakObjects.push_back(pTweaks);
        }
    }

    // MP2+ - Load tweaks from Standard.ntwk
    else
    {
        TString FilePath = mpProject->DiscFilesystemRoot(false) + "Standard.ntwk";
        CFileInStream StandardNTWK(FilePath, EEndian::BigEndian);
        CTweakLoader::LoadNTWK(StandardNTWK, mpProject->Game(), mTweakObjects);
    }
}

CTweakManager::~CTweakManager()
{
    for (CTweakData* pTweakData : mTweakObjects)
    {
        if (pTweakData->Entry() != nullptr)
        {
            pTweakData->Release();
        }
        else
        {
            delete pTweakData;
        }
    }
}

void CTweakManager::SaveTweaks()
{
    // In MP1, to save an individual tweak asset, just call Tweak->Entry()->Save()
    // In MP2+, call this function.
    //@todo
}
