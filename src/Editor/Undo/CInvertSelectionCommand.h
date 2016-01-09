#ifndef CINVERTSELECTIONCOMMAND_H
#define CINVERTSELECTIONCOMMAND_H

#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>

#include <QUndoCommand>

class CInvertSelectionCommand : public QUndoCommand
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
};

#endif // CINVERTSELECTIONCOMMAND_H
