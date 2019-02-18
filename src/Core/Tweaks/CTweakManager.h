#ifndef CTWEAKMANAGER_H
#define CTWEAKMANAGER_H

#include "CTweakData.h"

/** Class responsible for managing game tweak data, including saving/loading and providing access */
class CTweakManager
{
    /** Project */
    CGameProject* mpProject;

    /** All tweak resources in the current game */
    std::vector< CTweakData* > mTweakObjects;

    /** For MP2+, the path to Standard.ntwk */
    TString mStandardFilePath;

public:
    CTweakManager(CGameProject* pInProject);
    ~CTweakManager();
    void LoadTweaks();
    bool SaveTweaks();
    void ClearTweaks();

    // Accessors
    inline const std::vector<CTweakData*>& TweakObjects() const
    {
        return mTweakObjects;
    }
};

#endif // CTWEAKMANAGER_H
