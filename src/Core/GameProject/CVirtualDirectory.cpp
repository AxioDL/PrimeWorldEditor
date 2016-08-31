#include "CVirtualDirectory.h"
#include "CResourceEntry.h"
#include "CResourceStore.h"
#include <algorithm>

CVirtualDirectory::CVirtualDirectory()
    : mpParent(nullptr)
{}

CVirtualDirectory::CVirtualDirectory(const TWideString& rkName)
    : mpParent(nullptr), mName(rkName)
{}

CVirtualDirectory::CVirtualDirectory(CVirtualDirectory *pParent, const TWideString& rkName)
    : mpParent(pParent), mName(rkName)
{}

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
    return (mpParent && !mpParent->IsRoot() ? mpParent->FullPath() + L'\\' + mName + L"\\" : mName + L"\\");
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

        if (pChild->Name() == DirName)
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
        AddChild(rkName, nullptr);
        CVirtualDirectory *pOut = FindChildDirectory(rkName, false);
        ASSERT(pOut != nullptr);
        return pOut;
    }

    return nullptr;
}

CResourceEntry* CVirtualDirectory::FindChildResource(const TWideString& rkPath)
{
    TWideString Dir = rkPath.GetFileDirectory();
    TWideString Name = rkPath.GetFileName(false);

    if (!Dir.IsEmpty())
    {
        CVirtualDirectory *pDir = FindChildDirectory(Dir, false);
        if (pDir) return pDir->FindChildResource(Name);
    }

    else
    {
        for (u32 iRes = 0; iRes < mResources.size(); iRes++)
        {
            if (mResources[iRes]->Name() == Name)
                return mResources[iRes];
        }
    }

    return nullptr;
}

void CVirtualDirectory::AddChild(const TWideString &rkPath, CResourceEntry *pEntry)
{
    if (rkPath.IsEmpty())
    {
        if (pEntry)
            mResources.push_back(pEntry);
    }

    else
    {
        u32 SlashIdx = rkPath.IndexOf(L"\\/");
        TWideString DirName = (SlashIdx == -1 ? rkPath : rkPath.SubString(0, SlashIdx));
        CVirtualDirectory *pSubdir = nullptr;

        // Check if this subdirectory already exists
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
            pSubdir = new CVirtualDirectory(this, DirName);
            mSubdirectories.push_back(pSubdir);

            std::sort(mSubdirectories.begin(), mSubdirectories.end(), [](CVirtualDirectory *pLeft, CVirtualDirectory *pRight) -> bool {
                return (pLeft->Name() < pRight->Name());
            });
        }

        TWideString Remaining = (SlashIdx == -1 ? L"" : rkPath.SubString(SlashIdx + 1, rkPath.Size() - SlashIdx));
        pSubdir->AddChild(Remaining, pEntry);
    }
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

            if (mpParent && IsEmpty())
                mpParent->RemoveChildDirectory(this);

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

            if (mpParent && IsEmpty())
                mpParent->RemoveChildDirectory(this);

            return true;
        }
    }

    return false;
}
