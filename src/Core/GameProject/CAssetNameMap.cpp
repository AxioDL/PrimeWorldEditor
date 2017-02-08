#include "CAssetNameMap.h"

void CAssetNameMap::LoadAssetNames(TString Path /*= gkAssetMapPath*/)
{
    CXMLReader Reader(Path);
    Serialize(Reader);
}

void CAssetNameMap::SaveAssetNames(TString Path /*= gkAssetMapPath*/)
{
    CXMLWriter Writer(Path, "AssetNameMap", 0, eUnknownGame);
    Serialize(Writer);
}

bool CAssetNameMap::GetNameInfo(CAssetID ID, TString& rOutDirectory, TString& rOutName)
{
    auto It = mMap.find(ID);

    if (It != mMap.end())
    {
        SAssetNameInfo& rInfo = It->second;
        rOutName = rInfo.Name;
        rOutDirectory = rInfo.Directory;
        return true;
    }

    else
    {
        rOutDirectory = "Uncategorized\\";
        rOutName = ID.ToString();
        return false;
    }
}

void CAssetNameMap::CopyFromStore(CResourceStore *pStore /*= gpResourceStore*/)
{
    // Do a first pass to remove old paths from used set to prevent false positives from eg. if two resources switch places
    for (CResourceIterator It(pStore); It; ++It)
    {
        if (It->IsCategorized() || It->IsNamed())
        {
            auto Find = mMap.find(It->ID());

            if (Find != mMap.end())
            {
                SAssetNameInfo& rInfo = Find->second;
                auto UsedFind = mUsedSet.find(rInfo);
                ASSERT(UsedFind != mUsedSet.end());
                mUsedSet.erase(UsedFind);
            }
        }
    }

    // Do a second pass to add the new paths to the map
    for (CResourceIterator It(pStore); It; ++It)
    {
        if (It->IsCategorized() || It->IsNamed())
        {
            CAssetID ID = It->ID();
            TWideString Name = It->Name();
            TWideString Directory = It->Directory()->FullPath();
            CFourCC Type = It->CookedExtension();
            SAssetNameInfo NameInfo { Name, Directory, Type };

            // Check for conflicts with new name
            if (mUsedSet.find(NameInfo) != mUsedSet.end())
            {
                SAssetNameInfo NewNameInfo = NameInfo;
                int NumConflicted = 0;

                while (mUsedSet.find(NewNameInfo) != mUsedSet.end())
                {
                    NewNameInfo.Name = NameInfo.Name + L'_' + TWideString::FromInt32(NumConflicted, 0, 10);
                    NumConflicted++;
                }

                TString OldPath = NameInfo.FullPath().ToUTF8();
                TString NewPath = NewNameInfo.FullPath().ToUTF8();
                Log::Warning("Detected name conflict when copying asset name from the resource store; renaming.");
                Log::Warning("\tOld Path: " + OldPath);
                Log::Warning("\tNew Path: " + NewPath);
                NameInfo.Name = NewNameInfo.Name;
            }

            // Assign to map
            mMap[ID] = NameInfo;
            mUsedSet.insert(NameInfo);
        }
    }
}

// ************ PRIVATE ************
void CAssetNameMap::Serialize(IArchive& rArc)
{
    rArc << SERIAL_CONTAINER("AssetNameMap", mMap, "Asset");

    if (rArc.IsReader())
        PostLoadValidate();
}

void CAssetNameMap::PostLoadValidate()
{
    // Make sure the newly loaded map doesn't contain any name conflicts.
    mIsValid = false;
    std::set<SAssetNameInfo> Dupes;

    for (auto Iter = mMap.begin(); Iter != mMap.end(); Iter++)
    {
        const SAssetNameInfo& rkInfo = Iter->second;

        if (mUsedSet.find(rkInfo) != mUsedSet.end())
            Dupes.insert(rkInfo);
        else
            mUsedSet.insert(rkInfo);
    }

    // If we detected any dupes, then this map can't be used
    if (!Dupes.empty())
    {
        Log::Error("Asset name map is invalid and cannot be used! Duplicate asset entries detected:");

        for (auto Iter = Dupes.begin(); Iter != Dupes.end(); Iter++)
        {
            const SAssetNameInfo& rkInfo = *Iter;
            TString FullPath = rkInfo.FullPath().ToUTF8();
            Log::Error("\t" + FullPath);
        }

        mMap.clear();
    }
    else
        mIsValid = true;
}
