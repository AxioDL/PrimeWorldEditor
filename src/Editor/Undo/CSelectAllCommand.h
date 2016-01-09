#ifndef CSELECTALLCOMMAND_H
#define CSELECTALLCOMMAND_H

#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>

#include <QUndoCommand>

class CSelectAllCommand : public QUndoCommand
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
};

#endif // CSELECTALLCOMMAND_H
