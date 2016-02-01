#ifndef CSELECTALLCOMMAND_H
#define CSELECTALLCOMMAND_H

#include "IUndoCommand.h"
#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>

class CSelectAllCommand : public IUndoCommand
{
    INodeEditor *mpEditor;
    QList<CSceneNode*> mOldSelection;
    QList<CSceneNode*> mNewSelection;
    QList<CSceneNode*> *mpSelection;

public:
    CSelectAllCommand(INodeEditor *pEditor, QList<CSceneNode*>& rSelection, CScene *pScene, FNodeFlags NodeFlags);
    ~CSelectAllCommand();
    void undo();
    void redo();
    bool AffectsCleanState() const { return false; }
};

#endif // CSELECTALLCOMMAND_H
