#ifndef CTRANSLATENODECOMMAND_H
#define CTRANSLATENODECOMMAND_H

#include "IUndoCommand.h"
#include "ObjReferences.h"
#include <Core/Scene/CSceneNode.h>
#include "Editor/INodeEditor.h"
#include <QList>

class CTranslateNodeCommand : public IUndoCommand
{
    struct SNodeTranslate
    {
        CNodePtr pNode;
        CVector3f InitialPos;
        CVector3f NewPos;
    };
    QList<SNodeTranslate> mNodeList;
    INodeEditor *mpEditor = nullptr;
    bool mCommandEnded = false;

public:
    CTranslateNodeCommand();
    CTranslateNodeCommand(INodeEditor *pEditor, const QList<CSceneNode*>& rkNodes, const CVector3f& rkDelta, ETransformSpace TransformSpace);

    int id() const override;
    bool mergeWith(const QUndoCommand *pkOther) override;
    void undo() override;
    void redo() override;
    bool AffectsCleanState() const override { return true; }
    static CTranslateNodeCommand* End();
};

#endif // CTRANSLATENODECOMMAND_H
