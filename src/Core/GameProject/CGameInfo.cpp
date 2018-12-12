#include "CGameInfo.h"
#include <Common/FileUtil.h>

bool CGameInfo::LoadGameInfo(EGame Game)
{
    Game = RoundGame(Game);
    mGame = Game;

    TString Path = GetDefaultGameInfoPath(Game);
    return LoadGameInfo(Path);
}

bool CGameInfo::LoadGameInfo(TString Path)
{
    CXMLReader Reader(Path);

    if (Reader.IsValid())
    {
        Serialize(Reader);
        return true;
    }
    else return false;
}

bool CGameInfo::SaveGameInfo(TString Path /*= ""*/)
{
    ASSERT(mGame != EGame::Invalid); // can't save game info that was never loaded

    if (Path.IsEmpty()) Path = GetDefaultGameInfoPath(mGame);
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
    for (uint32 iBuild = 0; iBuild < mBuilds.size(); iBuild++)
    {
        const SBuildInfo& rkBuildInfo = mBuilds[iBuild];

        if (rkBuildInfo.Version == BuildVer && rkBuildInfo.Region == Region)
            return rkBuildInfo.Name;
    }

    return "Unknown Build";
}

TString CGameInfo::GetAreaName(const CAssetID &rkID) const
{
    auto Iter = mAreaNameMap.find(rkID);
    return (Iter == mAreaNameMap.end() ? "" : Iter->second);
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

    TString GameName = GetGameShortName(Game);
    return TString::Format("%s/GameInfo%s.%s", *gkGameInfoDir, *GameName, *gkGameInfoExt);
}
