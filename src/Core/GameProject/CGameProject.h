#ifndef CGAMEPROJECT_H
#define CGAMEPROJECT_H

#include "CPackage.h"
#include "CResourceStore.h"
#include "Core/Resource/EGame.h"
#include <Common/CUniqueID.h>
#include <Common/TString.h>
#include <Common/types.h>

class CGameProject
{
    EGame mGame;
    TString mProjectName;
    TWideString mProjectRoot;
    TWideString mResourceDBPath;
    std::vector<CPackage*> mWorldPaks;
    std::vector<CPackage*> mResourcePaks;

public:
    CGameProject(const TWideString& rkProjRootDir)
        : mGame(eUnknownVersion)
        , mProjectName("UnnamedProject")
        , mProjectRoot(rkProjRootDir)
        , mResourceDBPath(L"ResourceDB.rdb")
    {}

    void AddPackage(CPackage *pPackage, bool WorldPak);

    // Directory Handling
    inline TWideString ProjectRoot() const                      { return mProjectRoot; }
    inline TWideString ResourceDBPath(bool Relative) const      { return Relative ? mResourceDBPath : mProjectRoot + mResourceDBPath; }
    inline TWideString DiscDir(bool Relative) const             { return Relative ? L"Disc\\" : mProjectRoot + L"Disc\\"; }
    inline TWideString ContentDir(bool Relative) const          { return Relative ? L"Content\\" : mProjectRoot + L"Content\\"; }
    inline TWideString CookedDir(bool Relative) const           { return Relative ? L"Cooked\\" : mProjectRoot + L"Cooked\\"; }

    // Accessors
    inline void SetGame(EGame Game)                     { mGame = Game; }
    inline void SetProjectName(const TString& rkName)   { mProjectName = rkName; }

    inline u32 NumWorldPaks() const                     { return mWorldPaks.size(); }
    inline CPackage* WorldPakByIndex(u32 Index) const   { return mWorldPaks[Index]; }

    inline EGame Game() const                           { return mGame; }
};

extern CGameProject *gpProject;

#endif // CGAMEPROJECT_H
