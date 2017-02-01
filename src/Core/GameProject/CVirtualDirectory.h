#ifndef CVIRTUALDIRECTORY
#define CVIRTUALDIRECTORY

/* Virtual directory system used to look up resources by their location in the filesystem. */
#include "Core/Resource/EResType.h"
#include <Common/AssertMacro.h>
#include <Common/TString.h>
#include <vector>

class CResourceEntry;
class CResourceStore;

class CVirtualDirectory
{
    CVirtualDirectory *mpParent;
    CResourceStore *mpStore;
    TWideString mName;
    std::vector<CVirtualDirectory*> mSubdirectories;
    std::vector<CResourceEntry*> mResources;

public:
    CVirtualDirectory(CResourceStore *pStore);
    CVirtualDirectory(const TWideString& rkName, CResourceStore *pStore);
    CVirtualDirectory(CVirtualDirectory *pParent, const TWideString& rkName, CResourceStore *pStore);
    ~CVirtualDirectory();

    bool IsEmpty() const;
    TWideString FullPath() const;
    CVirtualDirectory* GetRoot();
    CVirtualDirectory* FindChildDirectory(const TWideString& rkName, bool AllowCreate);
    CResourceEntry* FindChildResource(const TWideString& rkPath);
    CResourceEntry* FindChildResource(const TWideString& rkName, EResType Type);
    void AddChild(const TWideString& rkPath, CResourceEntry *pEntry);
    bool RemoveChildDirectory(CVirtualDirectory *pSubdir);
    bool RemoveChildResource(CResourceEntry *pEntry);

    // Accessors
    inline CVirtualDirectory* Parent() const    { return mpParent; }
    inline bool IsRoot() const                  { return !mpParent; }
    inline TWideString Name() const             { return mName; }

    inline u32 NumSubdirectories() const                        { return mSubdirectories.size(); }
    inline CVirtualDirectory* SubdirectoryByIndex(u32 Index)    { return mSubdirectories[Index]; }
    inline u32 NumResources() const                             { return mResources.size(); }
    inline CResourceEntry* ResourceByIndex(u32 Index)           { return mResources[Index]; }
};

#endif // CVIRTUALDIRECTORY

