#include "Core/GameProject/CVirtualDirectory.h"

#include <Common/FileUtil.h>
#include <Common/Log.h>
#include <Common/Macros.h>
#include "Core/GameProject/CResourceEntry.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/CResource.h"
#include <algorithm>

CVirtualDirectory::CVirtualDirectory(CResourceStore *pStore)
    : mpStore(pStore)
{}

CVirtualDirectory::CVirtualDirectory(TString rkName, CResourceStore *pStore)
    : mpStore(pStore), mName(std::move(rkName))
{
    ASSERT(!mName.IsEmpty() && FileUtil::IsValidName(mName, true));
}

CVirtualDirectory::CVirtualDirectory(CVirtualDirectory *pParent, TString rkName, CResourceStore *pStore)
    : mpParent(pParent), mpStore(pStore), mName(std::move(rkName))
{
    ASSERT(!mName.IsEmpty() && FileUtil::IsValidName(mName, true));
}

CVirtualDirectory::~CVirtualDirectory()
{
    for (auto* subdirectory : mSubdirectories)
        delete subdirectory;
}

bool CVirtualDirectory::IsEmpty(bool CheckFilesystem) const
{
    if (!mResources.empty())
        return false;

    for (auto* subdirectory : mSubdirectories)
    {
        if (!subdirectory->IsEmpty(CheckFilesystem))
            return false;
    }

    if (CheckFilesystem && !FileUtil::IsEmpty(AbsolutePath()))
        return false;

    return true;
}

bool CVirtualDirectory::IsDescendantOf(const CVirtualDirectory *pDir) const
{
    return (this == pDir) || (mpParent != nullptr && pDir != nullptr && (mpParent == pDir || mpParent->IsDescendantOf(pDir)));
}

bool CVirtualDirectory::IsSafeToDelete() const
{
    // Return false if we contain any referenced assets.
    const bool HasReferences = std::ranges::any_of(mResources, [](const auto* entry) {
        return entry->IsLoaded() && entry->Resource()->IsReferenced();
    });
    if (HasReferences)
        return false;

    return std::ranges::all_of(mSubdirectories, &CVirtualDirectory::IsSafeToDelete);
}

TString CVirtualDirectory::FullPath() const
{
    if (IsRoot())
        return {};

    return (mpParent != nullptr ? TString(mpParent->FullPath() + mName) : mName) + '/';
}

TString CVirtualDirectory::AbsolutePath() const
{
    return mpStore->ResourcesDir() + FullPath();
}

CVirtualDirectory* CVirtualDirectory::GetRoot()
{
    return mpParent != nullptr ? mpParent->GetRoot() : this;
}

CVirtualDirectory* CVirtualDirectory::FindChildDirectory(const TString& rkName, bool AllowCreate)
{
    const auto SlashIdx = rkName.IndexOf("\\/");
    const TString DirName = (SlashIdx == -1 ? rkName : TString(rkName.SubString(0, SlashIdx)));

    const auto childIt = std::ranges::find_if(mSubdirectories, [&](const auto* child) {
        return child->Name().CaseInsensitiveCompare(DirName);
    });
    if (childIt != mSubdirectories.end())
    {
        if (SlashIdx == -1)
            return *childIt;

        const TString Remaining = rkName.SubString(SlashIdx + 1, rkName.Size() - SlashIdx);
        if (Remaining.IsEmpty())
            return *childIt;

        return (*childIt)->FindChildDirectory(Remaining, AllowCreate);
    }

    if (AllowCreate)
    {
        if (AddChild(rkName, nullptr))
        {
            CVirtualDirectory* pOut = FindChildDirectory(rkName, false);
            ASSERT(pOut != nullptr);
            return pOut;
        }
    }

    return nullptr;
}

CResourceEntry* CVirtualDirectory::FindChildResource(const TString& rkPath)
{
    const TString Dir = rkPath.GetFileDirectory();
    const TString Name = rkPath.GetFileName();

    if (!Dir.IsEmpty())
    {
        CVirtualDirectory* pDir = FindChildDirectory(Dir, false);
        if (pDir != nullptr)
            return pDir->FindChildResource(Name);
    }
    else if (!Name.IsEmpty())
    {
        const TString Ext = Name.GetFileExtension();
        const EResourceType Type = CResTypeInfo::TypeForCookedExtension(mpStore->Game(), CFourCC(Ext))->Type();
        return FindChildResource(Name.GetFileName(false), Type);
    }

    return nullptr;
}

CResourceEntry* CVirtualDirectory::FindChildResource(const TString& rkName, EResourceType Type)
{
    const auto it = std::ranges::find_if(mResources, [&](const auto* resource) {
        return resource->ResourceType() == Type && rkName.CaseInsensitiveCompare(resource->Name());
    });

    if (it == mResources.cend())
        return nullptr;

    return *it;
}

bool CVirtualDirectory::AddChild(const TString &rkPath, CResourceEntry *pEntry)
{
    if (rkPath.IsEmpty())
    {
        if (pEntry != nullptr)
        {
            mResources.push_back(pEntry);
            return true;
        }

        return false;
    }

    if (IsValidDirectoryPath(rkPath))
    {
        const auto SlashIdx = rkPath.IndexOf("\\/");
        TString DirName = (SlashIdx == -1 ? rkPath : TString(rkPath.SubString(0, SlashIdx)));
        const TString Remaining = (SlashIdx == -1 ? "" : rkPath.SubString(SlashIdx + 1, rkPath.Size() - SlashIdx));

        // Check if this subdirectory already exists
        const auto subIter = std::ranges::find_if(mSubdirectories, [&](const auto* dir) { return dir->Name() == DirName; });
        CVirtualDirectory* pSubdir = (subIter == mSubdirectories.end()) ? nullptr : *subIter;

        if (pSubdir == nullptr)
        {
            // Create new subdirectory
            pSubdir = new CVirtualDirectory(this, std::move(DirName), mpStore);

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

            for (auto& component : Components)
            {
                pSubdir = new CVirtualDirectory(pSubdir, std::move(component), mpStore);

                if (!pSubdir->CreateFilesystemDirectory())
                {
                    delete pSubdir;
                    return false;
                }

                pSubdir->Parent()->mSubdirectories.push_back(pSubdir);
            }

            if (pEntry != nullptr)
                pSubdir->mResources.push_back(pEntry);

            return true;
        }

        // If we have another valid child to add, return whether that operation completed successfully
        if (!Remaining.IsEmpty() || pEntry != nullptr)
            return pSubdir->AddChild(Remaining, pEntry);

        // Otherwise, we're done, so just return true
        return true;
    }

    return false;
}

bool CVirtualDirectory::AddChild(CVirtualDirectory *pDir)
{
    if (pDir->Parent() != this)
        return false;

    if (FindChildDirectory(pDir->Name(), false) != nullptr)
        return false;

    mSubdirectories.push_back(pDir);
    SortSubdirectories();

    return true;
}

bool CVirtualDirectory::RemoveChildDirectory(CVirtualDirectory* pSubdir)
{
    const auto it = std::ranges::find(mSubdirectories, pSubdir);
    if (it == mSubdirectories.cend())
        return false;

    mSubdirectories.erase(it);
    return true;
}

bool CVirtualDirectory::RemoveChildResource(CResourceEntry* pEntry)
{
    const auto it = std::ranges::find(mResources, pEntry);
    if (it == mResources.cend())
        return false;

    mResources.erase(it);
    return true;
}

void CVirtualDirectory::SortSubdirectories()
{
    std::ranges::sort(mSubdirectories, [](const auto* pLeft, const auto* pRight) {
        return pLeft->Name().ToUpper() < pRight->Name().ToUpper();
    });
}

bool CVirtualDirectory::Rename(const TString& rkNewName)
{
    NLog::Debug("MOVING DIRECTORY: {} --> {}", FullPath(), mpParent->FullPath() + rkNewName + '/');

    if (!IsRoot())
    {
        if (mpParent->FindChildDirectory(rkNewName, false) == nullptr)
        {
            const TString AbsPath = AbsolutePath();
            const TString NewPath = mpParent->AbsolutePath() + rkNewName + "/";

            if (FileUtil::MoveDirectory(AbsPath, NewPath))
            {
                mName = rkNewName;
                mpStore->SetCacheDirty();
                mpParent->SortSubdirectories();
                return true;
            }
        }
    }

    NLog::Error("DIRECTORY MOVE FAILED");
    return false;
}

bool CVirtualDirectory::Delete()
{
    ASSERT(IsEmpty(true) && !IsRoot());

    if (IsEmpty(true) && !IsRoot())
    {
        if (FileUtil::DeleteDirectory(AbsolutePath(), true))
        {
            if (mpParent == nullptr || mpParent->RemoveChildDirectory(this))
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
    for (size_t SubdirIdx = 0; SubdirIdx < mSubdirectories.size(); SubdirIdx++)
    {
        CVirtualDirectory *pDir = mSubdirectories[SubdirIdx];

        if (pDir->IsEmpty(true))
        {
            pDir->Delete();
            SubdirIdx--;
        }
        else
        {
            pDir->DeleteEmptySubdirectories();
        }
    }
}

bool CVirtualDirectory::CreateFilesystemDirectory()
{
    const TString AbsPath = AbsolutePath();

    if (!FileUtil::Exists(AbsPath))
    {
        const bool CreateSuccess = FileUtil::MakeDirectory(AbsPath);

        if (!CreateSuccess)
            NLog::Error("FAILED to create filesystem directory: {}", AbsPath);

        return CreateSuccess;
    }

    return true;
}

bool CVirtualDirectory::SetParent(CVirtualDirectory *pParent)
{
    ASSERT(!pParent->IsDescendantOf(this));
    if (mpParent == pParent)
        return true;

    NLog::Debug("MOVING DIRECTORY: {} -> {}", FullPath(), pParent->FullPath() + mName + '/');

    // Check for a conflict
    CVirtualDirectory *pConflictDir = pParent->FindChildDirectory(mName, false);

    if (pConflictDir != nullptr)
    {
        NLog::Error("DIRECTORY MOVE FAILED: Conflicting directory exists at the destination path!");
        return false;
    }

    // Move filesystem contents to new path
    const TString AbsOldPath = mpStore->ResourcesDir() + FullPath();
    const TString AbsNewPath = mpStore->ResourcesDir() + pParent->FullPath() + mName + '/';

    if (mpParent->RemoveChildDirectory(this) && FileUtil::MoveDirectory(AbsOldPath, AbsNewPath))
    {
        mpParent = pParent;
        mpParent->AddChild(this);
        mpStore->SetCacheDirty();
        return true;
    }
    else
    {
        NLog::Error("DIRECTORY MOVE FAILED: Filesystem move operation failed!");
        mpParent->AddChild(this);
        return false;
    }
}

// ************ STATIC ************
bool CVirtualDirectory::IsValidDirectoryName(const TString& rkName)
{
    return rkName != "." &&
           rkName != ".." &&
           FileUtil::IsValidName(rkName, true);
}

bool CVirtualDirectory::IsValidDirectoryPath(TString Path)
{
    // Entirely empty path is valid - this refers to root
    if (Path.IsEmpty())
        return true;

    // One trailing slash is allowed, but will cause IsValidName to fail, so we remove it here
    if (Path.EndsWith('/') || Path.EndsWith('\\'))
        Path = Path.ChopBack(1);

    const TStringList Parts = Path.Split("/\\", true);
    return std::ranges::all_of(Parts, &IsValidDirectoryName);
}
