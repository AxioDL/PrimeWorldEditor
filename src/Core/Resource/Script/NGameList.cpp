#include "NGameList.h"

namespace NGameList
{

/** Path for the templates directory */
const TString gkTemplatesDir = "../templates/";

/** Path to the game list file */
const TString gkGameListPath = gkTemplatesDir + "GameList.xml";

/** Info about a particular game serialized to the list */
struct SGameInfo
{
    TString Name;
    TString TemplatePath;
    std::unique_ptr<CGameTemplate> pTemplate;
    bool IsValid;

    SGameInfo()
        : IsValid(false)
    {}

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
SGameInfo gGameList[EGame::Max];

/** Whether the game list has been loaded */
bool gLoadedGameList = false;

/** Returns whether a game template has been loaded or not */
bool IsGameTemplateLoaded(EGame Game)
{
    int GameIdx = (int) Game;
    const SGameInfo& GameInfo = gGameList[GameIdx];
    return GameInfo.pTemplate != nullptr;
}

/** Serialize the game list to/from a file */
inline void SerializeGameList(IArchive& Arc)
{
    // Serialize the number of games with valid GameInfos.
    u32 NumGames = 0;

    if (Arc.IsWriter())
    {
        for (u32 GameIdx = 0; GameIdx < (u32) EGame::Max; GameIdx++)
        {
            if ( gGameList[GameIdx].IsValid )
                NumGames++;
        }
    }

    Arc.SerializeArraySize(NumGames);

    // Serialize the actual game info
    for (u32 GameIdx = 0; GameIdx < (u32) EGame::Max; GameIdx++)
    {
        // Skip games that don't have game templates when writing.
        if (Arc.IsWriter() && !gGameList[GameIdx].IsValid)
            continue;

        ENSURE( Arc.ParamBegin("Game", 0) );

        // Determine which game is being serialized
        EGame Game = (EGame) GameIdx;
        Arc << SerialParameter("ID", Game, SH_Attribute);
        ASSERT( Game != EGame::Invalid );

        gGameList[ (u32) Game ].Serialize(Arc);
        Arc.ParamEnd();
    }
}

/** Load the game list into memory */
void LoadGameList()
{
    ASSERT(!gLoadedGameList);

    CXMLReader Reader(gkGameListPath);
    ASSERT(Reader.IsValid());

    SerializeGameList(Reader);
    gLoadedGameList = true;
}

/** Save the game list back out to a file */
void SaveGameList()
{
    ASSERT(gLoadedGameList);

    CXMLWriter Writer(gkGameListPath, "GameList");
    ASSERT(Writer.IsValid());

    SerializeGameList(Writer);
}

/** Load all game templates into memory */
void LoadAllGameTemplates()
{
    for (int GameIdx = 0; GameIdx < (int) EGame::Max; GameIdx++)
        GetGameTemplate( (EGame) GameIdx );
}

/** Resave templates. If ForceAll is false, only saves templates that have been modified. */
void SaveTemplates(bool ForceAll /*= false*/)
{
    for (int GameIdx = 0; GameIdx < (int) EGame::Max; GameIdx++)
    {
        EGame Game = (EGame) GameIdx;
        if ( IsGameTemplateLoaded(Game) )
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

    ASSERT(Game >= (EGame) 0 && Game < EGame::Max);

    // Initialize the game list, if it hasn't been loaded yet.
    if (!gLoadedGameList)
    {
        LoadGameList();
    }

    int GameIdx = (int) Game;
    SGameInfo& GameInfo = gGameList[GameIdx];

    // Load the game template, if it hasn't been loaded yet.
    if (!GameInfo.pTemplate && !GameInfo.Name.IsEmpty())
    {
        TString GamePath = gkTemplatesDir + GameInfo.TemplatePath;
        GameInfo.pTemplate = std::make_unique<CGameTemplate>();
        GameInfo.pTemplate->Load(GamePath);
    }

    return GameInfo.pTemplate.get();
}

/** Clean up game list resources. This needs to be called on app shutdown to ensure things are cleaned up in the right order. */
void Shutdown()
{
    for (int GameIdx = 0; GameIdx < (int) EGame::Max; GameIdx++)
    {
        gGameList[GameIdx].Name = "";
        gGameList[GameIdx].TemplatePath = "";
        gGameList[GameIdx].pTemplate = nullptr;
    }
    gLoadedGameList = false;
}

}
