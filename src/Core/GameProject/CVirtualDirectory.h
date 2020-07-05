#ifndef CVIRTUALDIRECTORY
#define CVIRTUALDIRECTORY

/* Virtual directory system used to look up resources by their location in the filesystem. */
#include "Core/Resource/EResType.h"
#include <Common/Macros.h>
#include <Common/TString.h>
#include <vector>

class CResourceEntry;
class CResourceStore;

class CVirtualDirectory
{
    CVirtualDirectory *mpParent = nullptr;
    CResourceStore *mpStore;
    TString mName;
    std::vector<CVirtualDirectory*> mSubdirectories;
    std::vector<CResourceEntry*> mResources;

public:
    explicit CVirtualDirectory(CResourceStore *pStore);
    CVirtualDirectory(const TString& rkName, CResourceStore *pStore);
    CVirtualDirectory(CVirtualDirectory *pParent, const TString& rkName, CResourceStore *pStore);
    ~CVirtualDirectory();

    bool IsEmpty(bool CheckFilesystem) const;
    bool IsDescendantOf(CVirtualDirectory *pDir) const;
    bool IsSafeToDelete() const;
    TString FullPath() const;
    TString AbsolutePath() const;
    CVirtualDirectory* GetRoot();
    CVirtualDirectory* FindChildDirectory(const TString& rkName, bool AllowCreate);
    CResourceEntry* FindChildResource(const TString& rkPath);
    CResourceEntry* FindChildResource(const TString& rkName, EResourceType Type);
    bool AddChild(const TString& rkPath, CResourceEntry *pEntry);
    bool AddChild(CVirtualDirectory *pDir);
    bool RemoveChildDirectory(CVirtualDirectory *pSubdir);
    bool RemoveChildResource(CResourceEntry *pEntry);
    void SortSubdirectories();
    bool Rename(const TString& rkNewName);
    bool Delete();
    void DeleteEmptySubdirectories();
    bool CreateFilesystemDirectory();
    bool SetParent(CVirtualDirectory *pParent);

    static bool IsValidDirectoryName(const TString& rkName);
    static bool IsValidDirectoryPath(TString Path);

    // Accessors
    CVirtualDirectory* Parent() const    { return mpParent; }
    bool IsRoot() const                  { return !mpParent; }
    TString Name() const                 { return mName; }

    size_t NumSubdirectories() const                                 { return mSubdirectories.size(); }
    CVirtualDirectory* SubdirectoryByIndex(size_t Index)             { return mSubdirectories[Index]; }
    const CVirtualDirectory* SubdirectoryByIndex(size_t Index) const { return mSubdirectories[Index]; }
    size_t NumResources() const                                      { return mResources.size(); }
    CResourceEntry* ResourceByIndex(size_t Index)                    { return mResources[Index]; }
    const CResourceEntry* ResourceByIndex(size_t Index) const        { return mResources[Index]; }
};

#endif // CVIRTUALDIRECTORY

