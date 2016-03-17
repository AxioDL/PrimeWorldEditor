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
    INodeEditor *mpEditor;
    bool mCommandEnded;

public:
    CTranslateNodeCommand();
    CTranslateNodeCommand(INodeEditor *pEditor, const QList<CSceneNode*>& rkNodes, const CVector3f& rkDelta, ETransformSpace TransformSpace);
    int id() const;
    bool mergeWith(const QUndoCommand *pkOther);
    void undo();
    void redo();
    bool AffectsCleanState() const { return true; }
    static CTranslateNodeCommand* End();
};

#endif // CTRANSLATENODECOMMAND_H
