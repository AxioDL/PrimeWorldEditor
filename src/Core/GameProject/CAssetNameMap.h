#ifndef CASSETNAMEMAP
#define CASSETNAMEMAP

#include "CResourceIterator.h"
#include "CResourceStore.h"
#include <Common/CAssetID.h>
#include <Common/Serialization/XML.h>
#include <map>
#include <memory>

const TString gkAssetListDir = "..\\resources\\list\\";

class CAssetNameMap
{
    struct SAssetNameInfo
    {
        TWideString Name;
        TWideString Directory;

        void Serialize(IArchive& rArc)
        {
            rArc << SERIAL_AUTO(Name) << SERIAL_AUTO(Directory);
        }
    };

    EGame mGame;
    std::map<CAssetID, SAssetNameInfo> mMap;
    static std::map<EGame, CAssetNameMap*> smGameMap;

    // Private Methods
    CAssetNameMap(EGame Game);

    void Serialize(IArchive& rArc)
    {
        rArc << SERIAL_CONTAINER("AssetNameMap", mMap, "Asset");
    }

public:
    void SaveAssetNames();
    void GetNameInfo(CAssetID ID, TString& rOutDirectory, TString& rOutName);
    void CopyFromStore(CResourceStore *pStore);

    // Static Methods
    static TString GetAssetListPath(EGame Game)
    {
        return gkAssetListDir + "AssetList" + GetGameShortName(Game) + ".xml";
    }

    static CAssetNameMap* GetGameNameMap(EGame Game)
    {
        auto Find = smGameMap.find(Game);
        if (Find != smGameMap.end()) return Find->second;

        CAssetNameMap *pMap = new CAssetNameMap(Game);
        smGameMap[Game] = pMap;
        return pMap;
    }
};

#endif // CASSETNAMEMAP

