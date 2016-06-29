#ifndef CRESOURCEENTRY_H
#define CRESOURCEENTRY_H

#include "CVirtualDirectory.h"
#include "Core/Resource/EResType.h"
#include <Common/CUniqueID.h>
#include <Common/types.h>

class CResource;
class CResourceStore;

class CResourceEntry
{
    CResourceStore *mpStore;
    CResource *mpResource;
    CUniqueID mID;
    EResType mType;
    EGame mGame;
    CVirtualDirectory *mpDirectory;
    TWideString mFileName;
    bool mNeedsRecook;
    bool mTransient;

public:
    CResourceEntry(CResourceStore *pStore, const CUniqueID& rkID,
                   const TWideString& rkDir, const TWideString& rkFilename,
                   EResType Type, bool Transient = false);
    ~CResourceEntry();
    bool HasRawVersion() const;
    bool HasCookedVersion() const;
    TString RawAssetPath(bool Relative = false) const;
    TString CookedAssetPath(bool Relative = false) const;
    bool NeedsRecook() const;
    void SetGame(EGame NewGame);
    CResource* Load();
    CResource* Load(IInputStream& rInput);
    bool Unload();

    // Accessors
    void SetDirty()                             { mNeedsRecook = true; }

    inline bool IsLoaded() const                { return mpResource != nullptr; }
    inline CResource* Resource() const          { return mpResource; }
    inline CUniqueID ID() const                 { return mID; }
    inline EGame Game() const                   { return mGame; }
    inline CVirtualDirectory* Directory() const { return mpDirectory; }
    inline TString FileName() const             { return mFileName; }
    inline EResType ResourceType() const        { return mType; }
    inline bool IsTransient() const             { return mTransient; }

protected:
    CResource* InternalLoad(IInputStream& rInput);
};

#endif // CRESOURCEENTRY_H
