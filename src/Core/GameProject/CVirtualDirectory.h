#ifndef CVIRTUALDIRECTORY
#define CVIRTUALDIRECTORY

/* Virtual directory system used to look up resources by their location in the filesystem. */
#include <Common/AssertMacro.h>
#include <Common/TString.h>
#include <vector>

class CResourceEntry;

class CVirtualDirectory
{
    CVirtualDirectory *mpParent;
    TWideString mName;
    std::vector<CVirtualDirectory*> mSubdirectories;
    std::vector<CResourceEntry*> mResources;

public:
    CVirtualDirectory();
    CVirtualDirectory(const TWideString& rkName);
    CVirtualDirectory(CVirtualDirectory *pParent, const TWideString& rkName);
    ~CVirtualDirectory();

    bool IsEmpty() const;
    TWideString FullPath() const;
    CVirtualDirectory* GetRoot();
    CVirtualDirectory* FindChildDirectory(const TWideString& rkName, bool AllowCreate);
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

