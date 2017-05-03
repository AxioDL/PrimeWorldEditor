#include "CVirtualDirectory.h"
#include "CResourceEntry.h"
#include "CResourceStore.h"
#include "Core/Resource/CResource.h"
#include <algorithm>

CVirtualDirectory::CVirtualDirectory(CResourceStore *pStore)
    : mpParent(nullptr), mpStore(pStore)
{}

CVirtualDirectory::CVirtualDirectory(const TWideString& rkName, CResourceStore *pStore)
    : mpParent(nullptr), mName(rkName), mpStore(pStore)
{
    ASSERT(!mName.IsEmpty() && FileUtil::IsValidName(mName, true));
}

CVirtualDirectory::CVirtualDirectory(CVirtualDirectory *pParent, const TWideString& rkName, CResourceStore *pStore)
    : mpParent(pParent), mName(rkName), mpStore(pStore)
{
    ASSERT(!mName.IsEmpty() && FileUtil::IsValidName(mName, true));
}

CVirtualDirectory::~CVirtualDirectory()
{
    for (u32 iSub = 0; iSub < mSubdirectories.size(); iSub++)
        delete mSubdirectories[iSub];
}

bool CVirtualDirectory::IsEmpty() const
{
    if (!mResources.empty()) return false;

    for (u32 iSub = 0; iSub < mSubdirectories.size(); iSub++)
        if (!mSubdirectories[iSub]->IsEmpty()) return false;

    return true;
}

TWideString CVirtualDirectory::FullPath() const
{
    if (IsRoot())
        return L"";
    else
        return (mpParent && !mpParent->IsRoot() ? mpParent->FullPath() + mName + L'\\' : mName + L'\\');
}

CVirtualDirectory* CVirtualDirectory::GetRoot()
{
    return (mpParent ? mpParent->GetRoot() : this);
}

CVirtualDirectory* CVirtualDirectory::FindChildDirectory(const TWideString& rkName, bool AllowCreate)
{
    u32 SlashIdx = rkName.IndexOf(L"\\/");
    TWideString DirName = (SlashIdx == -1 ? rkName : rkName.SubString(0, SlashIdx));

    for (u32 iSub = 0; iSub < mSubdirectories.size(); iSub++)
    {
        CVirtualDirectory *pChild = mSubdirectories[iSub];

        if (pChild->Name().CaseInsensitiveCompare(DirName))
        {
            if (SlashIdx == -1)
                return pChild;

            else
            {
                TWideString Remaining = rkName.SubString(SlashIdx + 1, rkName.Size() - SlashIdx);

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

CResourceEntry* CVirtualDirectory::FindChildResource(const TWideString& rkPath)
{
    TWideString Dir = rkPath.GetFileDirectory();
    TWideString Name = rkPath.GetFileName();

    if (!Dir.IsEmpty())
    {
        CVirtualDirectory *pDir = FindChildDirectory(Dir, false);
        if (pDir) return pDir->FindChildResource(Name);
    }

    else if (!Name.IsEmpty())
    {
        TWideString Ext = Name.GetFileExtension();
        EResType Type = CResTypeInfo::TypeForCookedExtension(mpStore->Game(), Ext)->Type();
        return FindChildResource(Name.GetFileName(false), Type);
    }

    return nullptr;
}

CResourceEntry* CVirtualDirectory::FindChildResource(const TWideString& rkName, EResType Type)
{
    for (u32 iRes = 0; iRes < mResources.size(); iRes++)
    {
        if (rkName.CaseInsensitiveCompare(mResources[iRes]->Name()) && mResources[iRes]->ResourceType() == Type)
            return mResources[iRes];
    }

    return nullptr;
}

bool CVirtualDirectory::AddChild(const TWideString &rkPath, CResourceEntry *pEntry)
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
        u32 SlashIdx = rkPath.IndexOf(L"\\/");
        TWideString DirName = (SlashIdx == -1 ? rkPath : rkPath.SubString(0, SlashIdx));
        TWideString Remaining = (SlashIdx == -1 ? L"" : rkPath.SubString(SlashIdx + 1, rkPath.Size() - SlashIdx));

        // Check if this subdirectory already exists
        CVirtualDirectory *pSubdir = nullptr;

        for (u32 iSub = 0; iSub < mSubdirectories.size(); iSub++)
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
            mSubdirectories.push_back(pSubdir);

            std::sort(mSubdirectories.begin(), mSubdirectories.end(), [](CVirtualDirectory *pLeft, CVirtualDirectory *pRight) -> bool {
                return (pLeft->Name().ToUpper() < pRight->Name().ToUpper());
            });

            // As an optimization, don't recurse here. We've already verified the full path is valid, so we don't need to do it again.
            // We also know none of the remaining directories already exist because this is a new, empty directory.
            TWideStringList Components = Remaining.Split(L"/\\");

            for (auto Iter = Components.begin(); Iter != Components.end(); Iter++)
            {
                pSubdir = new CVirtualDirectory(pSubdir, *Iter, mpStore);
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

bool CVirtualDirectory::RemoveChildDirectory(CVirtualDirectory *pSubdir)
{
    ASSERT(pSubdir->IsEmpty());

    for (auto It = mSubdirectories.begin(); It != mSubdirectories.end(); It++)
    {
        if (*It == pSubdir)
        {
            mSubdirectories.erase(It);
            delete pSubdir;
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

// ************ STATIC ************
bool CVirtualDirectory::IsValidDirectoryName(const TWideString& rkName)
{
    return ( rkName != L"." &&
             rkName != L".." &&
             FileUtil::IsValidName(rkName, true) );
}

bool CVirtualDirectory::IsValidDirectoryPath(TWideString Path)
{
    // Entirely empty path is valid - this refers to root
    if (Path.IsEmpty())
        return true;

    // One trailing slash is allowed, but will cause IsValidName to fail, so we remove it here
    if (Path.EndsWith(L'/') || Path.EndsWith(L'\\'))
        Path = Path.ChopBack(1);

    TWideStringList Parts = Path.Split(L"/\\", true);

    for (auto Iter = Parts.begin(); Iter != Parts.end(); Iter++)
    {
        if (!IsValidDirectoryName(*Iter))
            return false;
    }

    return true;
}
