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
    ASSERT(mGame != eUnknownGame); // can't save game info that was never loaded

    if (Path.IsEmpty()) Path = GetDefaultGameInfoPath(mGame);
    CXMLWriter Writer(Path, "GameInfo", 0, mGame);
    Serialize(Writer);
    return Writer.Save();
}

void CGameInfo::Serialize(IArchive& rArc)
{
    // Validate game
    if (rArc.IsReader() && mGame != eUnknownGame)
    {
        ASSERT(mGame == rArc.Game());
    }

    // Serialize data
    if (mGame <= ePrime)
        rArc << SERIAL_CONTAINER("AreaNameMap", mAreaNameMap, "AreaName");
}

TString CGameInfo::GetAreaName(const CAssetID &rkID) const
{
    auto Iter = mAreaNameMap.find(rkID);
    return (Iter == mAreaNameMap.end() ? "" : Iter->second);
}

// ************ STATIC ************
EGame CGameInfo::RoundGame(EGame Game)
{
    if (Game == ePrimeDemo)         return ePrime;
    if (Game == eEchoesDemo)        return eEchoes;
    if (Game == eCorruptionProto)   return eCorruption;
    return Game;
}

TString CGameInfo::GetDefaultGameInfoPath(EGame Game)
{
    Game = RoundGame(Game);

    if (Game == eUnknownGame)
        return "";

    TString GameName = GetGameShortName(Game);
    return TString::Format("%s\\GameInfo%s.%s", *gkGameInfoDir, *GameName, *gkGameInfoExt);
}
