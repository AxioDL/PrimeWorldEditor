#ifndef CSAVESTORECOMMAND_H
#define CSAVESTORECOMMAND_H

#include "IUndoCommand.h"
#include <Core/GameProject/CResourceStore.h>

/** Command that calls ConditionalSaveStore on a resource store.
 *  This is meant to be added to undo macros that modify the resource store
 *  in order to trigger the store to resave when the macro is complete.
 */
class CSaveStoreCommand : public IUndoCommand
{
    CResourceStore* mpStore;

public:
    CSaveStoreCommand(CResourceStore* pInStore)
        : IUndoCommand("Save Store")
        , mpStore(pInStore)
    {}

    virtual void undo() override { mpStore->ConditionalSaveStore(); }
    virtual void redo() override { mpStore->ConditionalSaveStore(); }
    virtual bool AffectsCleanState() const override { return false; }
};

#endif // CSAVESTORECOMMAND_H
