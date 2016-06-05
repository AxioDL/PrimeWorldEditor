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
    inline TWideString ProjectRoot() const                      { return mProjectRoot; }
    inline TWideString DiscDir(bool Relative) const             { return Relative ? L"Disc\\" : mProjectRoot + L"Disc\\"; }
    inline TWideString ResourcesDir(bool Relative) const        { return Relative ? L"Resources\\" : mProjectRoot + L"Resources\\"; }
    inline TWideString WorldsDir(bool Relative) const           { return Relative ? L"Worlds\\" : mProjectRoot + L"Worlds\\"; }
    inline TWideString CookedDir(bool Relative) const           { return Relative ? L"Cooked\\" : mProjectRoot + L"Cooked\\"; }
    inline TWideString CookedResourcesDir(bool Relative) const  { return CookedDir(Relative) + L"Resources\\"; }
    inline TWideString CookedWorldsDir(bool Relative) const     { return CookedDir(Relative) + L"Worlds\\"; }

    // Accessors
    inline void SetGame(EGame Game)                     { mGame = Game; }
    inline void SetProjectName(const TString& rkName)   { mProjectName = rkName; }

    inline u32 NumWorldPaks() const                     { return mWorldPaks.size(); }
    inline CPackage* WorldPakByIndex(u32 Index) const   { return mWorldPaks[Index]; }

    inline EGame Game() const                           { return mGame; }
    inline CResourceDatabase* ResourceDatabase() const  { return mpResourceDatabase; }
};

extern CGameProject *gpProject;

#endif // CGAMEPROJECT_H
