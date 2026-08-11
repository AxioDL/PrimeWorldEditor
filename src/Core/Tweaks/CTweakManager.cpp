#include "Core/Tweaks/CTweakManager.h"

#include <Common/FileUtil.h>
#include <Common/FileIO/CFileInStream.h>
#include <Common/FileIO/CFileOutStream.h>
#include "Core/GameProject/CGameProject.h"
#include "Core/Tweaks/CTweakCooker.h"
#include "Core/Tweaks/CTweakData.h"
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

    const auto IsTrilogy = mpProject->IsTrilogy();
    const auto GameType = mpProject->Game();

    // MP1 - Load all tweak assets into memory
    if (GameType <= EGame::Prime)
    {
        for (const auto& entry : mpProject->ResourceStore()->MakeTypedResourceView(EResourceType::Tweaks))
        {
            if (auto* tweaks = static_cast<CTweakData*>(entry->Load()))
            {
                tweaks->Lock();
                mTweakObjects.push_back(tweaks);
            }
        }
    }
    else // MP2+ - Load tweaks from Standard.ntwk
    {
        const auto FSRoot = mpProject->DiscFilesystemRoot(false);

        if (IsTrilogy)
        {
            // For Wii builds, there is another game-dependent subfolder.
            const TString GameName = (GameType == EGame::Prime ? "MP1" : GameType == EGame::Echoes ? "MP2" : "MP3");
            mStandardFilePath = FSRoot / GameName / "Standard.ntwk";

            // MP3 might actually be FrontEnd
            if (GameType == EGame::Corruption && !FileUtil::Exists(mStandardFilePath))
            {
                mStandardFilePath = FSRoot / "fe/Standard.ntwk";
            }
        }
        else
        {
            mStandardFilePath = FSRoot / "Standard.ntwk";
        }

        if (FileUtil::Exists(mStandardFilePath))
        {
            CFileInStream StandardNTWK(mStandardFilePath, std::endian::big);
            CTweakLoader::LoadNTWK(StandardNTWK, GameType, mTweakObjects, mpProject->ResourceStore());
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
        CFileOutStream StandardNTWK(mStandardFilePath, std::endian::big);
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
