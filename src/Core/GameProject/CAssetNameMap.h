#ifndef CASSETNAMEMAP
#define CASSETNAMEMAP

#include "CResourceIterator.h"
#include "CResourceStore.h"
#include <Common/CAssetID.h>
#include <Common/Serialization/XML.h>
#include <map>
#include <memory>

const TString gkAssetMapPath = "..\\resources\\gameinfo\\AssetNameMap";
const TString gkAssetMapExt = "xml";

class CAssetNameMap
{
    struct SAssetNameInfo
    {
        TWideString Name;
        TWideString Directory;
        CFourCC Type; // This is mostly just needed to verify no name conflicts

        TWideString FullPath() const
        {
            return Directory + Name + L'.' + Type.ToString().ToUTF16();
        }

        void Serialize(IArchive& rArc)
        {
            rArc << SERIAL_AUTO(Name) << SERIAL_AUTO(Directory) << SERIAL_AUTO(Type);
        }

        bool operator<(const SAssetNameInfo& rkOther) const
        {
            return FullPath().ToUpper() < rkOther.FullPath().ToUpper();
        }

        bool operator==(const SAssetNameInfo& rkOther) const
        {
            return FullPath().CaseInsensitiveCompare(rkOther.FullPath());
        }
    };

    std::set<SAssetNameInfo> mUsedSet; // Used to prevent name conflicts
    std::map<CAssetID, SAssetNameInfo> mMap;
    bool mIsValid;
    EIDLength mIDLength;

    // Private Methods
    void Serialize(IArchive& rArc);
    void PostLoadValidate();

public:
    CAssetNameMap(EIDLength IDLength) : mIsValid(true), mIDLength(IDLength)                       { ASSERT(mIDLength != eInvalidIDLength); }
    CAssetNameMap(EGame Game)         : mIsValid(true), mIDLength( CAssetID::GameIDLength(Game) ) { ASSERT(mIDLength != eInvalidIDLength); }
    bool LoadAssetNames(TString Path = "");
    bool SaveAssetNames(TString Path = "");
    bool GetNameInfo(CAssetID ID, TString& rOutDirectory, TString& rOutName);
    void CopyFromStore(CResourceStore *pStore);

    static TString DefaultNameMapPath(EIDLength IDLength);
    static TString DefaultNameMapPath(EGame Game);

    inline bool IsValid() const                 { return mIsValid; }
    inline static TString GetExtension()        { return gkAssetMapExt; }
};

#endif // CASSETNAMEMAP

