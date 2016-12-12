#ifndef CASSETNAMEMAP
#define CASSETNAMEMAP

#include "CResourceIterator.h"
#include "CResourceStore.h"
#include <Common/CAssetID.h>
#include <Common/Serialization/XML.h>
#include <map>
#include <memory>

const TString gkAssetListDir = "..\\resources\\list\\";

struct SAssetNameInfo
{
    TWideString Name;
    TWideString Directory;

    void Serialize(IArchive& rArc)
    {
        rArc << SERIAL_AUTO(Name) << SERIAL_AUTO(Directory);
    }
};

class CAssetNameMap
{
    typedef std::map<CAssetID, SAssetNameInfo> TAssetMap;
    std::shared_ptr<TAssetMap> mpMap;

    void Serialize(IArchive& rArc)
    {
        rArc << SERIAL_CONTAINER("AssetNameMap", *mpMap.get(), "Asset");
    }

public:
    CAssetNameMap()
    {
        mpMap = std::make_shared<TAssetMap>(TAssetMap());
    }

    void GetNameInfo(CAssetID ID, TString& rOutDirectory, TString& rOutName)
    {
        auto It = mpMap->find(ID);

        if (It != mpMap->end())
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

    static TString GetAssetListPath(EGame Game)
    {
        return gkAssetListDir + "AssetList" + GetGameShortName(Game) + ".xml";
    }

    static CAssetNameMap LoadAssetNames(EGame Game)
    {
        TString ListPath = GetAssetListPath(Game);
        CXMLReader Reader(ListPath);

        CAssetNameMap Map;
        Map.Serialize(Reader);
        return Map;
    }

    static void SaveAssetNames(CResourceStore *pStore = gpResourceStore)
    {
        CAssetNameMap Map;

        for (CResourceIterator It(pStore); It; ++It)
        {
            if (It->IsCategorized() || It->IsNamed())
            {
                CAssetID ID = It->ID();
                TWideString Name = It->Name();
                TWideString Directory = It->Directory()->FullPath();
                (*Map.mpMap)[ID] = SAssetNameInfo { Name, Directory };
            }
        }

        TString ListPath = GetAssetListPath(pStore->Game());
        CXMLWriter Writer(ListPath, "AssetList", 0, pStore->Game());
        Map.Serialize(Writer);
    }
};

#endif // CASSETNAMEMAP

