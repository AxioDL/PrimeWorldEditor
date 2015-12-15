#ifndef CSCALENODECOMMAND_H
#define CSCALENODECOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <Scene/CSceneNode.h>
#include "../INodeEditor.h"

class CScaleNodeCommand : public QUndoCommand
{
    struct SNodeScale
    {
        CSceneNode *pNode;
        CVector3f initialPos;
        CVector3f initialScale;
        CVector3f newPos;
        CVector3f newScale;
    };
    QList<SNodeScale> mNodeList;
    INodeEditor *mpEditor;
    bool mCommandEnded;

public:
    CScaleNodeCommand();
    CScaleNodeCommand(INodeEditor *pEditor, const QList<CSceneNode*>& nodes, const CVector3f& pivot, const CVector3f& delta);
    ~CScaleNodeCommand();
    int id() const;
    bool mergeWith(const QUndoCommand *other);
    void undo();
    void redo();
    static CScaleNodeCommand* End();
};

#endif // CScaleNODECOMMAND_H
