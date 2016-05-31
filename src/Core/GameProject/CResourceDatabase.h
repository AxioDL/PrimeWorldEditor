#ifndef CRESOURCEDATABASE_H
#define CRESOURCEDATABASE_H

#include "Core/Resource/CResource.h"
#include <Common/CUniqueID.h>
#include <Common/TString.h>
#include <Common/types.h>
#include <map>

class CGameProject;
class CResourceDatabase;

class CResourceEntry
{
    CResourceDatabase *mpDatabase;
    CUniqueID mID;
    TWideString mFileDir;
    TWideString mFileName;
    EResType mResourceType;
    bool mNeedsRecook;

public:
    CResourceEntry(CResourceDatabase *pDatabase, const CUniqueID& rkID,
                   const TWideString& rkDir, const TWideString& rkFilename, EResType Type)
        : mpDatabase(pDatabase)
        , mID(rkID)
        , mFileDir(rkDir)
        , mFileName(rkFilename)
        , mResourceType(Type)
        , mNeedsRecook(false)
    {}

    bool HasRawVersion() const;
    bool HasCookedVersion() const;
    TString RawAssetPath() const;
    TString CookedAssetPath() const;
    bool NeedsRecook() const;

    // Accessors
    void SetDirty()                         { mNeedsRecook = true; }

    inline CUniqueID ID() const             { return mID; }
    inline TString FileDirectory() const    { return mFileDir; }
    inline TString FileName() const         { return mFileName; }
    inline EResType ResourceType() const    { return mResourceType; }
};

class CResourceDatabase
{
    CGameProject *mpProj;
    std::map<CUniqueID, CResourceEntry*> mResourceMap;

    enum EVersion
    {
        eVer_Initial,

        eVer_Max,
        eVer_Current = eVer_Max - 1
    };

public:
    CResourceDatabase(CGameProject *pProj);
    ~CResourceDatabase();
    void Load(const TString& rkPath);
    void Save(const TString& rkPath) const;

    CResourceEntry* FindResourceEntry(const CUniqueID& rkID) const;
    CResource* LoadResource(const CUniqueID& rkID) const;
    bool RegisterResource(const CUniqueID& rkID, const TWideString& rkDir, const TWideString& rkFileName, EResType Type);

    inline CGameProject* GameProject() const    { return mpProj; }
};

#endif // CRESOURCEDATABASE_H
