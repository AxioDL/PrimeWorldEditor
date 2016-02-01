#ifndef CINVERTSELECTIONCOMMAND_H
#define CINVERTSELECTIONCOMMAND_H

#include "IUndoCommand.h"
#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>

class CInvertSelectionCommand : public IUndoCommand
{
    INodeEditor *mpEditor;
    QList<CSceneNode*> mOldSelection;
    QList<CSceneNode*> mNewSelection;
    QList<CSceneNode*> *mpSelection;

public:
    CInvertSelectionCommand(INodeEditor *pEditor, QList<CSceneNode*>& rSelection, CScene *pScene, FNodeFlags NodeFlags);
    ~CInvertSelectionCommand();
    void undo();
    void redo();
    virtual bool AffectsCleanState() const { return false; }
};

#endif // CINVERTSELECTIONCOMMAND_H
