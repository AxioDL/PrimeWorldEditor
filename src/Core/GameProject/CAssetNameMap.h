#ifndef CASSETNAMEMAP
#define CASSETNAMEMAP

#include "CResourceIterator.h"
#include "CResourceStore.h"
#include <Common/CAssetID.h>
#include <Common/Serialization/XML.h>
#include <map>
#include <memory>

class CAssetNameMap
{
    struct SAssetNameInfo
    {
        TString Name;
        TString Directory;
        CFourCC Type; // This is mostly just needed to verify no name conflicts
        bool AutoGenName;
        bool AutoGenDir;

        TString FullPath() const
        {
            return Directory + Name + '.' + Type.ToString();
        }

        void Serialize(IArchive& rArc)
        {
            rArc << SerialParameter("Name", Name)
                 << SerialParameter("Directory", Directory)
                 << SerialParameter("Type", Type)
                 << SerialParameter("AutoGenName", AutoGenName)
                 << SerialParameter("AutoGenDir", AutoGenDir);
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
    CAssetNameMap(EIDLength IDLength) : mIsValid(true), mIDLength(IDLength)                       { ASSERT(mIDLength != kInvalidIDLength); }
    CAssetNameMap(EGame Game)         : mIsValid(true), mIDLength( CAssetID::GameIDLength(Game) ) { ASSERT(mIDLength != kInvalidIDLength); }
    bool LoadAssetNames(TString Path = "");
    bool SaveAssetNames(TString Path = "");
    bool GetNameInfo(CAssetID ID, TString& rOutDirectory, TString& rOutName, bool& rOutAutoGenDir, bool& rOutAutoGenName);
    void CopyFromStore(CResourceStore *pStore);

    static TString DefaultNameMapPath(EIDLength IDLength);
    static TString DefaultNameMapPath(EGame Game);

    bool IsValid() const { return mIsValid; }
    static TString GetExtension();
};

#endif // CASSETNAMEMAP

