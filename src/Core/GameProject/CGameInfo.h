#ifndef CGAMEINFO
#define CGAMEINFO

#include <Common/AssertMacro.h>
#include <Common/CAssetID.h>
#include <Common/TString.h>
#include <Common/Serialization/IArchive.h>
#include <Common/Serialization/XML.h>
#include <map>

const TString gkGameInfoDir = "..\\resources\\gameinfo";
const TString gkGameInfoExt = "xml";

class CGameInfo
{
    EGame mGame;

    // List of known builds of each game
    struct SBuildInfo
    {
        float Version;
        ERegion Region;
        TString Name;

        void Serialize(IArchive& rArc)
        {
            rArc << SERIAL_AUTO(Version) << SERIAL_AUTO(Region) << SERIAL_AUTO(Name);
        }
    };
    std::vector<SBuildInfo> mBuilds;

    // List of internal area names; used for MP1 which doesn't store area names in the MLVL
    std::map<CAssetID, TString> mAreaNameMap;

public:
    CGameInfo()
        : mGame(eUnknownGame)
    {}

    bool LoadGameInfo(EGame Game);
    bool LoadGameInfo(TString Path);
    bool SaveGameInfo(TString Path = "");
    void Serialize(IArchive& rArc);

    TString GetBuildName(float BuildVer, ERegion Region) const;
    TString GetAreaName(const CAssetID& rkID) const;

    // Accessors
    inline EGame Game() const   { return mGame; }

    // Static
    static CGameInfo* GetGameInfo(EGame Game);
    static EGame RoundGame(EGame Game);
    static TString GetDefaultGameInfoPath(EGame Game);

    inline static TString GetExtension()    { return gkGameInfoExt; }
};

#endif // CGAMEINFO

