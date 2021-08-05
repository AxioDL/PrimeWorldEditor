#include "CTweakManager.h"
#include "Core/GameProject/CGameProject.h"
#include "Core/GameProject/CResourceIterator.h"
#include "Core/Tweaks/CTweakCooker.h"
#include "Core/Tweaks/CTweakLoader.h"

CTweakManager::CTweakManager(CGameProject* pInProject)
    : mpProject(pInProject)
{
}

CTweakManager::~CTweakManager()
{
    ClearTweaks();
}

void CTweakManager::LoadTweaks()
{
    ASSERT(mTweakObjects.empty());

    // MP1 - Load all tweak assets into memory
    if (mpProject->Game() <= EGame::Prime)
    {
        for (TResourceIterator<EResourceType::Tweaks> It(mpProject->ResourceStore()); It; ++It)
        {
            if (CTweakData* pTweaks = (CTweakData*)It->Load())
            {
                pTweaks->Lock();
                mTweakObjects.push_back(pTweaks);
            }
        }
    }

    // MP2+ - Load tweaks from Standard.ntwk
    else
    {
        if (!mpProject->IsTrilogy())
        {
            mStandardFilePath = mpProject->DiscFilesystemRoot(false) / "Standard.ntwk";
        }
        else
        {
            // For Wii builds, there is another game-dependent subfolder.
            EGame Game = mpProject->Game();
            TString GameName = (Game == EGame::Prime ? "MP1" : Game == EGame::Echoes ? "MP2" : "MP3");
            mStandardFilePath = mpProject->DiscFilesystemRoot(false) / GameName / "Standard.ntwk";

            // MP3 might actually be FrontEnd
            if (Game == EGame::Corruption && !FileUtil::Exists(mStandardFilePath))
            {
                mStandardFilePath = mpProject->DiscFilesystemRoot(false) / "fe/Standard.ntwk";
            }
        }

        if (FileUtil::Exists(mStandardFilePath))
        {
            CFileInStream StandardNTWK(mStandardFilePath, EEndian::BigEndian);
            CTweakLoader::LoadNTWK(StandardNTWK, mpProject->Game(), mTweakObjects);
        }
    }
}

bool CTweakManager::SaveTweaks()
{
    // If we don't have any tweaks loaded, nothing to do
    if (mTweakObjects.empty())
    {
        return false;
    }

    // MP1 - Save all tweak assets
    if (mpProject->Game() <= EGame::Prime)
    {
        bool SavedAll = true, SavedAny = false;

        for (CTweakData* pTweakData : mTweakObjects)
        {
            if (!pTweakData->Entry()->Save(true))
            {
                SavedAll = false;
            }
            else
            {
                SavedAny = true;
            }
        }

        if (SavedAny)
        {
            mpProject->ResourceStore()->ConditionalSaveStore();
        }

        return SavedAll;
    }
    // MP2+ - Save tweaks to Standard.ntwk
    else
    {
        CFileOutStream StandardNTWK(mStandardFilePath, EEndian::BigEndian);
        return CTweakCooker::CookNTWK(mTweakObjects, StandardNTWK);
    }
}

void CTweakManager::ClearTweaks()
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
    mTweakObjects.clear();
}
