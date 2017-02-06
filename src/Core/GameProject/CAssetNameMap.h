#ifndef CASSETNAMEMAP
#define CASSETNAMEMAP

#include "CResourceIterator.h"
#include "CResourceStore.h"
#include <Common/CAssetID.h>
#include <Common/Serialization/XML.h>
#include <map>
#include <memory>

const TString gkAssetMapPath = "..\\resources\\gameinfo\\AssetNameMap.xml";
const TString gkAssetMapExt = "xml";

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

    std::map<CAssetID, SAssetNameInfo> mMap;

    // Private Methods
    void Serialize(IArchive& rArc)
    {
        rArc << SERIAL_CONTAINER("AssetNameMap", mMap, "Asset");
    }

public:
    void LoadAssetNames(TString Path = gkAssetMapPath);
    void SaveAssetNames(TString Path = gkAssetMapPath);
    bool GetNameInfo(CAssetID ID, TString& rOutDirectory, TString& rOutName);
    void CopyFromStore(CResourceStore *pStore);

    inline static TString DefaultNameMapPath()  { return gkAssetMapPath; }
    inline static TString GetExtension()        { return gkAssetMapExt; }
};

#endif // CASSETNAMEMAP

