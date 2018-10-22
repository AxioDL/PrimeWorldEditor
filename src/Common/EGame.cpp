#include "EGame.h"
#include "CFourCC.h"
#include "Common/Serialization/IArchive.h"

TString GetGameName(EGame Game)
{
    static const TString skGameNames[EGame::Max] =
    {
        "Metroid Prime Demo",
        "Metroid Prime",
        "Metroid Prime 2: Echoes Demo",
        "Metroid Prime 2: Echoes",
        "Metroid Prime 3: Corruption E3 2006 Prototype",
        "Metroid Prime 3: Corruption",
        "Donkey Kong Country Returns"
    };

    int GameIdx = (int) Game;
    return (GameIdx >= 0 && GameIdx < (int) EGame::Max) ? skGameNames[GameIdx] : "Unknown Game";
}

TString GetGameShortName(EGame Game)
{
    static const TString skGameNames[EGame::Max] = {
        "MP1Demo",
        "MP1",
        "MP2Demo",
        "MP2",
        "MP3Proto",
        "MP3",
        "DKCR"
    };

    int GameIdx = (int) Game;
    return (GameIdx >= 0 && GameIdx < (int) EGame::Max) ? skGameNames[GameIdx] : "Unknown";
}

CFourCC GameTo4CC(EGame Game)
{
    static const CFourCC skGame4CCs[EGame::Max] =
    {
        FOURCC('MP1D'), FOURCC('MPRM'),
        FOURCC('MP2D'), FOURCC('MP2E'),
        FOURCC('MP3P'), FOURCC('MP3C'),
        FOURCC('DKCR')
    };

    int GameIdx = (int) Game;
    return (GameIdx >= 0 && GameIdx < (int) EGame::Max) ? skGame4CCs[GameIdx] : FOURCC('UNKN');
}

EGame GameFrom4CC(CFourCC GameId)
{
    static const std::unordered_map<u32, EGame> skIdToGame =
    {
        { FOURCC('MP1D'), EGame::PrimeDemo },
        { FOURCC('MPRM'), EGame::Prime },
        { FOURCC('MP2D'), EGame::EchoesDemo },
        { FOURCC('MP2E'), EGame::Echoes },
        { FOURCC('MP3P'), EGame::CorruptionProto },
        { FOURCC('MP3C'), EGame::Corruption },
        { FOURCC('DKCR'), EGame::DKCReturns }
    };
    auto MapFindIter = skIdToGame.find(GameId.ToLong());
    return (MapFindIter != skIdToGame.end() ? MapFindIter->second : EGame::Invalid);
}

void Serialize(IArchive& rArc, EGame& rGame)
{
    // We serialize EGame as a fourCC in binary formats as a future-proofing measure.
    // Additionally, older versions of IArchive always serialized EGame as a fourCC.
    if (rArc.ArchiveVersion() < IArchive::eArVer_GameEnumClass || rArc.IsBinaryFormat())
    {
        CFourCC GameId;

        if (rArc.IsWriter())
        {
            GameId = GameTo4CC(rGame);
        }

        rArc.SerializePrimitive(GameId, 0);

        if (rArc.IsReader())
        {
            rGame = GameFrom4CC(GameId);
        }
    }
    else
    {
        DefaultEnumSerialize<EGame>(rArc, rGame);
    }
}
