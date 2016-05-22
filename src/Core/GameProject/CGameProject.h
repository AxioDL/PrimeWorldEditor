#ifndef CGAMEPROJECT_H
#define CGAMEPROJECT_H

#include "CPackage.h"
#include "CResourceDatabase.h"
#include "Core/Resource/EGame.h"
#include <Common/CUniqueID.h>
#include <Common/TString.h>
#include <Common/types.h>

class CGameProject
{
    EGame mGame;
    TString mProjectName;
    TString mProjectRoot;
    CResourceDatabase *mpResourceDatabase;
    std::vector<CPackage*> mWorldPaks;
    std::vector<CPackage*> mResourcePaks;

public:
    CGameProject()
        : mGame(eUnknownVersion)
        , mProjectName("UnnamedProject")
        , mpResourceDatabase(new CResourceDatabase)
    {}

    void AddPackage(CPackage *pPackage, bool WorldPak);

    inline void SetGame(EGame Game)                     { mGame = Game; }
    inline void SetProjectName(const TString& rkName)   { mProjectName = rkName; }

    inline EGame Game() const                           { return mGame; }
    inline CResourceDatabase* ResourceDatabase() const  { return mpResourceDatabase; }
};

extern CGameProject *gpProject;

#endif // CGAMEPROJECT_H
