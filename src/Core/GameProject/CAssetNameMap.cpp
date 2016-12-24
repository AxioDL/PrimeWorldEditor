#include "CAssetNameMap.h"

std::map<EGame, CAssetNameMap*> CAssetNameMap::smGameMap;

CAssetNameMap::CAssetNameMap(EGame Game)
    : mGame(Game)
{
    TString ListPath = GetAssetListPath(mGame);
    CXMLReader Reader(ListPath);
    Serialize(Reader);
}

void CAssetNameMap::SaveAssetNames()
{
    TString ListPath = GetAssetListPath(mGame);
    CXMLWriter Writer(ListPath, "AssetList", 0, mGame);
    Serialize(Writer);
}

void CAssetNameMap::GetNameInfo(CAssetID ID, TString& rOutDirectory, TString& rOutName)
{
    auto It = mMap.find(ID);

    if (It != mMap.end())
    {
        SAssetNameInfo& rInfo = It->second;
        rOutName = rInfo.Name;
        rOutDirectory = rInfo.Directory;
    }

    else
    {
        rOutDirectory = "Uncategorized\\";
        rOutName = ID.ToString();
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
