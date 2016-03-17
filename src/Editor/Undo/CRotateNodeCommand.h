#ifndef CROTATENODECOMMAND_H
#define CROTATENODECOMMAND_H

#include "IUndoCommand.h"
#include "ObjReferences.h"
#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>
#include <QList>

class CRotateNodeCommand : public IUndoCommand
{
    struct SNodeRotate
    {
        CNodePtr pNode;
        CVector3f InitialPos;
        CQuaternion InitialRot;
        CVector3f NewPos;
        CQuaternion NewRot;
    };
    QList<SNodeRotate> mNodeList;
    INodeEditor *mpEditor;
    bool mCommandEnded;

public:
    CRotateNodeCommand();
    CRotateNodeCommand(INodeEditor *pEditor, const QList<CSceneNode*>& rkNodes, const CVector3f& rkPivot, const CQuaternion& rkDelta, ETransformSpace TransformSpace);
    ~CRotateNodeCommand();
    int id() const;
    bool mergeWith(const QUndoCommand *pkOther);
    void undo();
    void redo();
    bool AffectsCleanState() const { return true; }
    static CRotateNodeCommand* End();
};

#endif // CROTATENODECOMMAND_H
