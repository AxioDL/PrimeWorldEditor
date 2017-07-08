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
    TString mName;
    std::vector<CVirtualDirectory*> mSubdirectories;
    std::vector<CResourceEntry*> mResources;

public:
    CVirtualDirectory(CResourceStore *pStore);
    CVirtualDirectory(const TString& rkName, CResourceStore *pStore);
    CVirtualDirectory(CVirtualDirectory *pParent, const TString& rkName, CResourceStore *pStore);
    ~CVirtualDirectory();

    bool IsEmpty() const;
    TString FullPath() const;
    CVirtualDirectory* GetRoot();
    CVirtualDirectory* FindChildDirectory(const TString& rkName, bool AllowCreate);
    CResourceEntry* FindChildResource(const TString& rkPath);
    CResourceEntry* FindChildResource(const TString& rkName, EResType Type);
    bool AddChild(const TString& rkPath, CResourceEntry *pEntry);
    bool RemoveChildDirectory(CVirtualDirectory *pSubdir);
    bool RemoveChildResource(CResourceEntry *pEntry);
    void RemoveEmptySubdirectories();

    static bool IsValidDirectoryName(const TString& rkName);
    static bool IsValidDirectoryPath(TString Path);

    // Accessors
    inline CVirtualDirectory* Parent() const    { return mpParent; }
    inline bool IsRoot() const                  { return !mpParent; }
    inline TString Name() const                 { return mName; }

    inline u32 NumSubdirectories() const                        { return mSubdirectories.size(); }
    inline CVirtualDirectory* SubdirectoryByIndex(u32 Index)    { return mSubdirectories[Index]; }
    inline u32 NumResources() const                             { return mResources.size(); }
    inline CResourceEntry* ResourceByIndex(u32 Index)           { return mResources[Index]; }
};

#endif // CVIRTUALDIRECTORY

