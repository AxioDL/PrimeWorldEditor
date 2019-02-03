#include "CVirtualDirectory.h"
#include "CResourceEntry.h"
#include "CResourceStore.h"
#include "Core/Resource/CResource.h"
#include <algorithm>

CVirtualDirectory::CVirtualDirectory(CResourceStore *pStore)
    : mpParent(nullptr), mpStore(pStore)
{}

CVirtualDirectory::CVirtualDirectory(const TString& rkName, CResourceStore *pStore)
    : mpParent(nullptr), mName(rkName), mpStore(pStore)
{
    ASSERT(!mName.IsEmpty() && FileUtil::IsValidName(mName, true));
}

CVirtualDirectory::CVirtualDirectory(CVirtualDirectory *pParent, const TString& rkName, CResourceStore *pStore)
    : mpParent(pParent), mName(rkName), mpStore(pStore)
{
    ASSERT(!mName.IsEmpty() && FileUtil::IsValidName(mName, true));
}

CVirtualDirectory::~CVirtualDirectory()
{
    for (uint32 iSub = 0; iSub < mSubdirectories.size(); iSub++)
        delete mSubdirectories[iSub];
}

bool CVirtualDirectory::IsEmpty(bool CheckFilesystem) const
{
    if (!mResources.empty())
        return false;

    for (uint32 iSub = 0; iSub < mSubdirectories.size(); iSub++)
        if (!mSubdirectories[iSub]->IsEmpty(CheckFilesystem))
            return false;

    if (CheckFilesystem && !FileUtil::IsEmpty( AbsolutePath() ))
        return false;

    return true;
}

bool CVirtualDirectory::IsDescendantOf(CVirtualDirectory *pDir) const
{
    return (this == pDir) || (mpParent && pDir && (mpParent == pDir || mpParent->IsDescendantOf(pDir)));
}

bool CVirtualDirectory::IsSafeToDelete() const
{
    // Return false if we contain any referenced assets.
    for (CResourceEntry* pEntry : mResources)
    {
        if (pEntry->IsLoaded() && pEntry->Resource()->IsReferenced())
        {
            return false;
        }
    }

    for (CVirtualDirectory* pSubdir : mSubdirectories)
    {
        if (!pSubdir->IsSafeToDelete())
        {
            return false;
        }
    }

    return true;
}

TString CVirtualDirectory::FullPath() const
{
    if (IsRoot())
        return "";
    else
        return (mpParent ? mpParent->FullPath() + mName : mName) + '/';
}

TString CVirtualDirectory::AbsolutePath() const
{
    return mpStore->ResourcesDir() + FullPath();
}

CVirtualDirectory* CVirtualDirectory::GetRoot()
{
    return (mpParent ? mpParent->GetRoot() : this);
}

CVirtualDirectory* CVirtualDirectory::FindChildDirectory(const TString& rkName, bool AllowCreate)
{
    uint32 SlashIdx = rkName.IndexOf("\\/");
    TString DirName = (SlashIdx == -1 ? rkName : rkName.SubString(0, SlashIdx));

    for (uint32 iSub = 0; iSub < mSubdirectories.size(); iSub++)
    {
        CVirtualDirectory *pChild = mSubdirectories[iSub];

        if (pChild->Name().CaseInsensitiveCompare(DirName))
        {
            if (SlashIdx == -1)
                return pChild;

            else
            {
                TString Remaining = rkName.SubString(SlashIdx + 1, rkName.Size() - SlashIdx);

                if (Remaining.IsEmpty())
                    return pChild;
                else
                    return pChild->FindChildDirectory(Remaining, AllowCreate);
            }
        }
    }

    if (AllowCreate)
    {
        if ( AddChild(rkName, nullptr) )
        {
            CVirtualDirectory *pOut = FindChildDirectory(rkName, false);
            ASSERT(pOut != nullptr);
            return pOut;
        }
    }

    return nullptr;
}

CResourceEntry* CVirtualDirectory::FindChildResource(const TString& rkPath)
{
    TString Dir = rkPath.GetFileDirectory();
    TString Name = rkPath.GetFileName();

    if (!Dir.IsEmpty())
    {
        CVirtualDirectory *pDir = FindChildDirectory(Dir, false);
        if (pDir) return pDir->FindChildResource(Name);
    }

    else if (!Name.IsEmpty())
    {
        TString Ext = Name.GetFileExtension();
        EResourceType Type = CResTypeInfo::TypeForCookedExtension(mpStore->Game(), Ext)->Type();
        return FindChildResource(Name.GetFileName(false), Type);
    }

    return nullptr;
}

CResourceEntry* CVirtualDirectory::FindChildResource(const TString& rkName, EResourceType Type)
{
    for (uint32 iRes = 0; iRes < mResources.size(); iRes++)
    {
        if (rkName.CaseInsensitiveCompare(mResources[iRes]->Name()) && mResources[iRes]->ResourceType() == Type)
            return mResources[iRes];
    }

    return nullptr;
}

bool CVirtualDirectory::AddChild(const TString &rkPath, CResourceEntry *pEntry)
{
    if (rkPath.IsEmpty())
    {
        if (pEntry)
        {
            mResources.push_back(pEntry);
            return true;
        }
        else
            return false;
    }

    else if (IsValidDirectoryPath(rkPath))
    {
        uint32 SlashIdx = rkPath.IndexOf("\\/");
        TString DirName = (SlashIdx == -1 ? rkPath : rkPath.SubString(0, SlashIdx));
        TString Remaining = (SlashIdx == -1 ? "" : rkPath.SubString(SlashIdx + 1, rkPath.Size() - SlashIdx));

        // Check if this subdirectory already exists
        CVirtualDirectory *pSubdir = nullptr;

        for (uint32 iSub = 0; iSub < mSubdirectories.size(); iSub++)
        {
            if (mSubdirectories[iSub]->Name() == DirName)
            {
                pSubdir = mSubdirectories[iSub];
                break;
            }
        }

        if (!pSubdir)
        {
            // Create new subdirectory
            pSubdir = new CVirtualDirectory(this, DirName, mpStore);

            if (!pSubdir->CreateFilesystemDirectory())
            {
                delete pSubdir;
                return false;
            }

            mSubdirectories.push_back(pSubdir);
            SortSubdirectories();

            // As an optimization, don't recurse here. We've already verified the full path is valid, so we don't need to do it again.
            // We also know none of the remaining directories already exist because this is a new, empty directory.
            TStringList Components = Remaining.Split("/\\");

            for (auto Iter = Components.begin(); Iter != Components.end(); Iter++)
            {
                pSubdir = new CVirtualDirectory(pSubdir, *Iter, mpStore);

                if (!pSubdir->CreateFilesystemDirectory())
                {
                    delete pSubdir;
                    return false;
                }

                pSubdir->Parent()->mSubdirectories.push_back(pSubdir);
            }

            if (pEntry)
                pSubdir->mResources.push_back(pEntry);

            return true;
        }

        // If we have another valid child to add, return whether that operation completed successfully
        else if (!Remaining.IsEmpty() || pEntry)
            return pSubdir->AddChild(Remaining, pEntry);

        // Otherwise, we're done, so just return true
        else
            return true;
    }

    else
        return false;
}

bool CVirtualDirectory::AddChild(CVirtualDirectory *pDir)
{
    if (pDir->Parent() != this) return false;
    if (FindChildDirectory(pDir->Name(), false) != nullptr) return false;

    mSubdirectories.push_back(pDir);
    SortSubdirectories();

    return true;
}

bool CVirtualDirectory::RemoveChildDirectory(CVirtualDirectory *pSubdir)
{
    for (auto It = mSubdirectories.begin(); It != mSubdirectories.end(); It++)
    {
        if (*It == pSubdir)
        {
            mSubdirectories.erase(It);
            return true;
        }
    }

    return false;
}

bool CVirtualDirectory::RemoveChildResource(CResourceEntry *pEntry)
{
    for (auto It = mResources.begin(); It != mResources.end(); It++)
    {
        if (*It == pEntry)
        {
            mResources.erase(It);
            return true;
        }
    }

    return false;
}

void CVirtualDirectory::SortSubdirectories()
{
    std::sort(mSubdirectories.begin(), mSubdirectories.end(), [](CVirtualDirectory *pLeft, CVirtualDirectory *pRight) -> bool {
        return (pLeft->Name().ToUpper() < pRight->Name().ToUpper());
    });
}

bool CVirtualDirectory::Rename(const TString& rkNewName)
{
    debugf("MOVING DIRECTORY: %s --> %s", *FullPath(), *mpParent->FullPath() + rkNewName + '/');

    if (!IsRoot())
    {
        if (!mpParent->FindChildDirectory(rkNewName, false))
        {
            TString AbsPath = AbsolutePath();
            TString NewPath = mpParent->AbsolutePath() + rkNewName + "/";

            if (FileUtil::MoveDirectory(AbsPath, NewPath))
            {
                mName = rkNewName;
                mpStore->SetCacheDirty();
                mpParent->SortSubdirectories();
                return true;
            }
        }
    }

    errorf("DIRECTORY MOVE FAILED");
    return false;
}

bool CVirtualDirectory::Delete()
{
    ASSERT(IsEmpty(true) && !IsRoot());

    if (IsEmpty(true) && !IsRoot())
    {
        if (FileUtil::DeleteDirectory(AbsolutePath(), true))
        {
            if (!mpParent || mpParent->RemoveChildDirectory(this))
            {
                mpStore->SetCacheDirty();
                delete this;
                return true;
            }
        }
    }

    return false;
}

void CVirtualDirectory::DeleteEmptySubdirectories()
{
    for (uint32 SubdirIdx = 0; SubdirIdx < mSubdirectories.size(); SubdirIdx++)
    {
        CVirtualDirectory *pDir = mSubdirectories[SubdirIdx];

        if (pDir->IsEmpty(true))
        {
            pDir->Delete();
            SubdirIdx--;
        }
        else
            pDir->DeleteEmptySubdirectories();
    }
}

bool CVirtualDirectory::CreateFilesystemDirectory()
{
    TString AbsPath = AbsolutePath();

    if (!FileUtil::Exists(AbsPath))
    {
        bool CreateSuccess = FileUtil::MakeDirectory(AbsPath);

        if (!CreateSuccess)
            errorf("FAILED to create filesystem directory: %s", *AbsPath);

        return CreateSuccess;
    }

    return true;
}

bool CVirtualDirectory::SetParent(CVirtualDirectory *pParent)
{
    ASSERT(!pParent->IsDescendantOf(this));
    if (mpParent == pParent) return true;

    debugf("MOVING DIRECTORY: %s -> %s", *FullPath(), *(pParent->FullPath() + mName + '/'));

    // Check for a conflict
    CVirtualDirectory *pConflictDir = pParent->FindChildDirectory(mName, false);

    if (pConflictDir)
    {
        errorf("DIRECTORY MOVE FAILED: Conflicting directory exists at the destination path!");
        return false;
    }

    // Move filesystem contents to new path
    TString AbsOldPath = mpStore->ResourcesDir() + FullPath();
    TString AbsNewPath = mpStore->ResourcesDir() + pParent->FullPath() + mName + '/';

    if (mpParent->RemoveChildDirectory(this) && FileUtil::MoveDirectory(AbsOldPath, AbsNewPath))
    {
        mpParent = pParent;
        mpParent->AddChild(this);
        mpStore->SetCacheDirty();
        return true;
    }
    else
    {
        errorf("DIRECTORY MOVE FAILED: Filesystem move operation failed!");
        mpParent->AddChild(this);
        return false;
    }
}

// ************ STATIC ************
bool CVirtualDirectory::IsValidDirectoryName(const TString& rkName)
{
    return ( rkName != "." &&
             rkName != ".." &&
             FileUtil::IsValidName(rkName, true) );
}

bool CVirtualDirectory::IsValidDirectoryPath(TString Path)
{
    // Entirely empty path is valid - this refers to root
    if (Path.IsEmpty())
        return true;

    // One trailing slash is allowed, but will cause IsValidName to fail, so we remove it here
    if (Path.EndsWith('/') || Path.EndsWith('\\'))
        Path = Path.ChopBack(1);

    TStringList Parts = Path.Split("/\\", true);

    for (auto Iter = Parts.begin(); Iter != Parts.end(); Iter++)
    {
        if (!IsValidDirectoryName(*Iter))
            return false;
    }

    return true;
}
