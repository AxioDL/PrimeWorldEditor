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
    std::map<CAssetID, TString> mAreaNameMap;

public:
    CGameInfo()
        : mGame(eUnknownGame)
    {}

    void LoadGameInfo(EGame Game);
    void LoadGameInfo(TString Path);
    void SaveGameInfo(TString Path = "");
    void Serialize(IArchive& rArc);

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

