#ifndef NGAMELIST_H
#define NGAMELIST_H

#include "CGameTemplate.h"

namespace NGameList
{

/** Load all game templates into memory
 *  This normally isn't necessary to call, as game templates will be lazy-loaded the
 *  first time they are requested.
 */
void LoadAllGameTemplates();

/** Load the game list into memory. This is normally not necessary to call. */
void LoadGameList();

/** Save the game list back out to a file */
void SaveGameList();

/** Resave templates. If ForceAll is false, only saves templates that have been modified. */
void SaveTemplates(bool ForceAll = false);

/** Get the game template for a given game */
CGameTemplate* GetGameTemplate(EGame Game);

/** Clean up game list resources. This needs to be called on app shutdown to ensure things are cleaned up in the right order. */
void Shutdown();

}

#endif // NGAMELIST_H
