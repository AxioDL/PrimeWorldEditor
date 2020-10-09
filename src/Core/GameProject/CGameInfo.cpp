#include "CGameInfo.h"
#include "CResourceStore.h"
#include <Common/FileUtil.h>
#include <algorithm>

constexpr char gkGameInfoDir[] = "resources/gameinfo";
constexpr char gkGameInfoExt[] = "xml";

bool CGameInfo::LoadGameInfo(EGame Game)
{
    Game = RoundGame(Game);
    mGame = Game;

    TString Path = GetDefaultGameInfoPath(Game);
    return LoadGameInfo(Path);
}

bool CGameInfo::LoadGameInfo(const TString& Path)
{
    CXMLReader Reader(Path);

    if (Reader.IsValid())
    {
        Serialize(Reader);
        return true;
    }

    return false;
}

bool CGameInfo::SaveGameInfo(TString Path)
{
    ASSERT(mGame != EGame::Invalid); // can't save game info that was never loaded

    if (Path.IsEmpty())
        Path = GetDefaultGameInfoPath(mGame);

    CXMLWriter Writer(Path, "GameInfo", 0, mGame);
    Serialize(Writer);
    return Writer.Save();
}

void CGameInfo::Serialize(IArchive& rArc)
{
    // Validate game
    if (rArc.IsReader() && mGame != EGame::Invalid)
    {
        ASSERT(mGame == rArc.Game());
    }

    // Serialize data
    rArc << SerialParameter("GameBuilds", mBuilds);

    if (mGame <= EGame::Prime)
        rArc << SerialParameter("AreaNameMap", mAreaNameMap);
}

TString CGameInfo::GetBuildName(float BuildVer, ERegion Region) const
{
    const auto it = std::find_if(mBuilds.cbegin(), mBuilds.cend(),
                                 [=](const auto& entry) { return entry.Version == BuildVer && entry.Region == Region; });

    if (it == mBuilds.cend())
        return "Unknown Build";

    return it->Name;
}

TString CGameInfo::GetAreaName(const CAssetID &rkID) const
{
    const auto Iter = mAreaNameMap.find(rkID);
    return Iter == mAreaNameMap.cend() ? "" : Iter->second;
}

// ************ STATIC ************
EGame CGameInfo::RoundGame(EGame Game)
{
    if (Game == EGame::PrimeDemo)         return EGame::Prime;
    if (Game == EGame::EchoesDemo)        return EGame::Echoes;
    if (Game == EGame::CorruptionProto)   return EGame::Corruption;
    return Game;
}

TString CGameInfo::GetDefaultGameInfoPath(EGame Game)
{
    Game = RoundGame(Game);

    if (Game == EGame::Invalid)
        return "";

    const TString GameName = GetGameShortName(Game);
    return TString::Format("%s/%s/GameInfo%s.%s", gDataDir.ToStdString().c_str(), gkGameInfoDir, GameName.ToStdString().c_str(), gkGameInfoExt);
}

TString CGameInfo::GetExtension()
{
    return gkGameInfoExt;
}
