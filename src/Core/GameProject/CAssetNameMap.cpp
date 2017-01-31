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
    for (CResourceIterator It(pStore); It; ++It)
    {
        if (It->IsCategorized() || It->IsNamed())
        {
            CAssetID ID = It->ID();
            TWideString Name = It->Name();
            TWideString Directory = It->Directory()->FullPath();
            mMap[ID] = SAssetNameInfo { Name, Directory };
        }
    }
}
