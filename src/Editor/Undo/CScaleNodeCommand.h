#ifndef CSCALENODECOMMAND_H
#define CSCALENODECOMMAND_H

#include "IUndoCommand.h"
#include "ObjReferences.h"
#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>
#include <QList>

class CScaleNodeCommand : public IUndoCommand
{
    struct SNodeScale
    {
        CNodePtr pNode;
        CVector3f InitialPos;
        CVector3f InitialScale;
        CVector3f NewPos;
        CVector3f NewScale;
    };
    QList<SNodeScale> mNodeList;
    INodeEditor *mpEditor;
    bool mCommandEnded;

public:
    CScaleNodeCommand();
    CScaleNodeCommand(INodeEditor *pEditor, const QList<CSceneNode*>& rkNodes, const CVector3f& rkPivot, const CVector3f& rkDelta);
    ~CScaleNodeCommand();
    int id() const;
    bool mergeWith(const QUndoCommand *pkOther);
    void undo();
    void redo();
    bool AffectsCleanState() const { return true; }
    static CScaleNodeCommand* End();
};

#endif // CScaleNODECOMMAND_H
