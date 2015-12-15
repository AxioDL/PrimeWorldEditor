#ifndef CTRANSLATENODECOMMAND_H
#define CTRANSLATENODECOMMAND_H

#include <Core/Scene/CSceneNode.h>
#include "Editor/INodeEditor.h"

#include <QUndoCommand>
#include <QList>

class CTranslateNodeCommand : public QUndoCommand
{
    struct SNodeTranslate
    {
        CSceneNode *pNode;
        CVector3f initialPos;
        CVector3f newPos;
    };
    QList<SNodeTranslate> mNodeList;
    INodeEditor *mpEditor;
    bool mCommandEnded;

public:
    CTranslateNodeCommand();
    CTranslateNodeCommand(INodeEditor *pEditor, const QList<CSceneNode*>& nodes, const CVector3f& delta, ETransformSpace transformSpace);
    ~CTranslateNodeCommand();
    int id() const;
    bool mergeWith(const QUndoCommand *other);
    void undo();
    void redo();
    static CTranslateNodeCommand* End();
};

#endif // CTRANSLATENODECOMMAND_H
