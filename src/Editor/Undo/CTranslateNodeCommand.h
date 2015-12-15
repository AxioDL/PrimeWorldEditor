#ifndef CTRANSLATENODECOMMAND_H
#define CTRANSLATENODECOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <Scene/CSceneNode.h>
#include "../INodeEditor.h"

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
