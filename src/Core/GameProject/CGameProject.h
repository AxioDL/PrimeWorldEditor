#ifndef CGAMEPROJECT_H
#define CGAMEPROJECT_H

#include "CPackage.h"
#include "CResourceStore.h"
#include "Core/Resource/EGame.h"
#include <Common/FileUtil.h>
#include <Common/CUniqueID.h>
#include <Common/TString.h>
#include <Common/types.h>

class CGameProject
{
    EGame mGame;
    TString mProjectName;
    TWideString mProjectRoot;
    TWideString mResourceDBPath;
    std::vector<CPackage*> mPackages;

    enum EProjectVersion
    {
        eVer_Initial,

        eVer_Max,
        eVer_Current = eVer_Max - 1
    };
public:
    CGameProject(const TWideString& rkProjRootDir)
        : mGame(eUnknownVersion)
        , mProjectName("Unnamed Project")
        , mProjectRoot(rkProjRootDir)
        , mResourceDBPath(L"ResourceDB.rdb")
    {}

    void Load();
    void Save();

    // Directory Handling
    inline TWideString ProjectRoot() const                      { return mProjectRoot; }
    inline TWideString ResourceDBPath(bool Relative) const      { return Relative ? mResourceDBPath : mProjectRoot + mResourceDBPath; }
    inline TWideString DiscDir(bool Relative) const             { return Relative ? L"Disc\\" : mProjectRoot + L"Disc\\"; }
    inline TWideString ContentDir(bool Relative) const          { return Relative ? L"Content\\" : mProjectRoot + L"Content\\"; }
    inline TWideString CookedDir(bool Relative) const           { return Relative ? L"Cooked\\" : mProjectRoot + L"Cooked\\"; }
    inline TWideString PackagesDir(bool Relative) const         { return Relative ? L"Packages\\" : mProjectRoot + L"Packages\\"; }
    inline TWideString ProjectPath() const                      { return mProjectRoot + FileUtil::SanitizeName(mProjectName.ToUTF16(), false) + L".prj"; }

    // Accessors
    inline void SetGame(EGame Game)                     { mGame = Game; }
    inline void SetProjectName(const TString& rkName)   { mProjectName = rkName; }

    inline u32 NumPackages() const                      { return mPackages.size(); }
    inline CPackage* PackageByIndex(u32 Index) const    { return mPackages[Index]; }
    inline void AddPackage(CPackage *pPackage)          { mPackages.push_back(pPackage); }

    inline EGame Game() const                           { return mGame; }
};

extern CGameProject *gpProject;

#endif // CGAMEPROJECT_H
