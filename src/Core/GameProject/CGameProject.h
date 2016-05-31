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
    TWideString mProjectRoot;
    CResourceDatabase *mpResourceDatabase;
    std::vector<CPackage*> mWorldPaks;
    std::vector<CPackage*> mResourcePaks;

public:
    CGameProject(const TWideString& rkProjRootDir)
        : mGame(eUnknownVersion)
        , mProjectName("UnnamedProject")
        , mProjectRoot(rkProjRootDir)
        , mpResourceDatabase(new CResourceDatabase(this))
    {}

    void AddPackage(CPackage *pPackage, bool WorldPak);

    // Directory Handling
    inline TWideString ProjectRoot() const          { return mProjectRoot; }
    inline TWideString DiscDir() const              { return mProjectRoot + L"Disc\\"; }
    inline TWideString ResourcesDir() const         { return mProjectRoot + L"Resources\\"; }
    inline TWideString WorldsDir() const            { return mProjectRoot + L"Worlds\\"; }
    inline TWideString CookedDir() const            { return mProjectRoot + L"Cooked\\"; }
    inline TWideString CookedResourcesDir() const   { return CookedDir() + L"Resources\\"; }
    inline TWideString CookedWorldsDir() const      { return CookedDir() + L"Worlds\\"; }

    // Accessors
    inline void SetGame(EGame Game)                     { mGame = Game; }
    inline void SetProjectName(const TString& rkName)   { mProjectName = rkName; }

    inline EGame Game() const                           { return mGame; }
    inline CResourceDatabase* ResourceDatabase() const  { return mpResourceDatabase; }
};

extern CGameProject *gpProject;

#endif // CGAMEPROJECT_H
