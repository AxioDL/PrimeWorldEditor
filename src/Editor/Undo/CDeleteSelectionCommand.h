#ifndef CDELETESELECTIONCOMMAND_H
#define CDELETESELECTIONCOMMAND_H

#include "IUndoCommand.h"
#include "Editor/WorldEditor/CWorldEditor.h"
#include <Core/Scene/CSceneNode.h>

class CDeleteSelectionCommand : public IUndoCommand
{
    CWorldEditor *mpEditor;
    QList<CSceneNode*> mNodesToDelete;
    QList<CSceneNode*> mOldSelection;
    QList<CSceneNode*> mNewSelection;

public:
    CDeleteSelectionCommand(CWorldEditor *pEditor);
    void undo();
    void redo();
    bool AffectsCleanState() const { return true; }
};

#endif // CDELETESELECTIONCOMMAND_H
