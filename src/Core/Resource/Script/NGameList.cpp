#include "Core/Resource/Script/NGameList.h"

#include <Common/Log.h>
#include <Common/Serialization/CXMLReader.h>
#include <Common/Serialization/CXMLWriter.h>
#include "Core/Resource/Script/CGameTemplate.h"

#include <array>

namespace NGameList
{
namespace
{
/** Path for the templates directory */
const TString gkTemplatesDir = "templates/";

/** Path to the game list file */
const TString gkGameListPath = gkTemplatesDir + "GameList.xml";

/** Info about a particular game serialized to the list */
struct SGameInfo
{
    TString Name;
    TString TemplatePath;
    std::unique_ptr<CGameTemplate> pTemplate;
    bool IsValid = false;

    SGameInfo() = default;

    void Serialize(IArchive& Arc)
    {
        Arc << SerialParameter("Name", Name)
            << SerialParameter("GameTemplate", TemplatePath);

        if (Arc.IsReader())
        {
            IsValid = true;
        }
    }
};

// See comment in CTemplateEditDialog.cpp by UpdateTypeName for why
// this exists (silly reasons!).
constexpr auto GamesMinusMP1R = static_cast<size_t>(EGame::Max) - 1;
std::array<SGameInfo, static_cast<size_t>(EGame::Max)> gGameList;

/** Whether the game list has been loaded */
bool gLoadedGameList = false;

/** Returns whether a game template has been loaded or not */
bool IsGameTemplateLoaded(EGame Game)
{
    const auto GameIdx = static_cast<size_t>(Game);
    const SGameInfo& GameInfo = gGameList[GameIdx];
    return GameInfo.pTemplate != nullptr;
}

/** Serialize the game list to/from a file */
void SerializeGameList(IArchive& Arc)
{
    // Serialize the number of games with valid GameInfos.
    uint32_t NumGames = 0;

    if (Arc.IsWriter())
    {
        for (uint32_t GameIdx = 0; GameIdx < GamesMinusMP1R; GameIdx++)
        {
            if (gGameList[GameIdx].IsValid)
                NumGames++;
        }
    }

    Arc.SerializeArraySize(NumGames);

    // Serialize the actual game info
    for (uint32_t GameIdx = 0; GameIdx < GamesMinusMP1R; GameIdx++)
    {
        // Skip games that don't have game templates when writing.
        if (Arc.IsWriter() && !gGameList[GameIdx].IsValid)
            continue;

        ENSURE(Arc.ParamBegin("Game", 0));

        // Determine which game is being serialized
        auto Game = static_cast<EGame>(GameIdx);
        Arc << SerialParameter("ID", Game, SH_Attribute);
        ASSERT(Game != EGame::Invalid);

        gGameList[static_cast<uint32_t>(Game)].Serialize(Arc);
        Arc.ParamEnd();
    }
}
} // Anonymous namespace

/** Load the game list into memory */
void LoadGameList()
{
    ASSERT(!gLoadedGameList);
    NLog::Debug("Loading game list");

    CXMLReader Reader(gDataDir + gkGameListPath);
    ASSERT(Reader.IsValid());

    SerializeGameList(Reader);
    gLoadedGameList = true;
}

/** Save the game list back out to a file */
void SaveGameList()
{
    ASSERT(gLoadedGameList);
    NLog::Debug("Saving game list");

    CXMLWriter Writer(gDataDir + gkGameListPath, "GameList");
    ASSERT(Writer.IsValid());

    SerializeGameList(Writer);
}

/** Load all game templates into memory */
void LoadAllGameTemplates()
{
    for (size_t GameIdx = 0; GameIdx < GamesMinusMP1R; GameIdx++)
        GetGameTemplate(static_cast<EGame>(GameIdx));
}

/** Resave templates. If ForceAll is false, only saves templates that have been modified. */
void SaveTemplates(bool ForceAll)
{
    for (size_t GameIdx = 0; GameIdx < GamesMinusMP1R; GameIdx++)
    {
        const auto Game = static_cast<EGame>(GameIdx);
        if (IsGameTemplateLoaded(Game))
        {
            CGameTemplate* pGameTemplate = GetGameTemplate(Game);
            pGameTemplate->SaveGameTemplates(ForceAll);
        }
    }
}

/** Get the game template for a given game */
CGameTemplate* GetGameTemplate(EGame Game)
{
    // Game must be valid!
    if (Game == EGame::Invalid)
    {
        return nullptr;
    }

    ASSERT(Game >= EGame::PrimeDemo && static_cast<size_t>(Game) < GamesMinusMP1R);

    // Initialize the game list, if it hasn't been loaded yet.
    if (!gLoadedGameList)
    {
        LoadGameList();
    }

    const int GameIdx = static_cast<int>(Game);
    SGameInfo& GameInfo = gGameList[GameIdx];

    // Load the game template, if it hasn't been loaded yet.
    if (!GameInfo.pTemplate && !GameInfo.Name.IsEmpty())
    {
        const TString GamePath = gDataDir + gkTemplatesDir + GameInfo.TemplatePath;
        GameInfo.pTemplate = std::make_unique<CGameTemplate>();
        GameInfo.pTemplate->Load(GamePath);
    }

    return GameInfo.pTemplate.get();
}

/** Clean up game list resources. This needs to be called on app shutdown to ensure things are cleaned up in the right order. */
void Shutdown()
{
    for (size_t GameIdx = 0; GameIdx < GamesMinusMP1R; GameIdx++)
    {
        gGameList[GameIdx].Name.clear();
        gGameList[GameIdx].TemplatePath.clear();
        gGameList[GameIdx].pTemplate = nullptr;
    }
    gLoadedGameList = false;
}

}
