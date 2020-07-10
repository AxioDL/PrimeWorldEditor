#ifndef CDELETESELECTIONCOMMAND_H
#define CDELETESELECTIONCOMMAND_H

#include "CDeleteLinksCommand.h"
#include "IUndoCommand.h"
#include "ObjReferences.h"
#include "Editor/WorldEditor/CWorldEditor.h"
#include <Core/Scene/CSceneNode.h>

// todo: currently only supports deleting script nodes; needs support for light nodes as well
// plus maybe it should be extensible enough to support other possible types
class CDeleteSelectionCommand : public IUndoCommand
{
    CWorldEditor *mpEditor;
    CNodePtrList mOldSelection;
    CNodePtrList mNewSelection;
    CInstancePtrList mLinkedInstances;

    struct SDeletedNode
    {
        // Node Info
        CNodePtr NodePtr;
        uint32 NodeID;
        CVector3f Position;
        CQuaternion Rotation;
        CVector3f Scale;

        // Instance Info
        CGameArea *pArea;
        CScriptLayer *pLayer;
        uint32 LayerIndex;
        std::vector<char> InstanceData;
    };
    QVector<SDeletedNode> mDeletedNodes;

    struct SDeletedLink
    {
        uint32 State;
        uint32 Message;
        uint32 SenderID;
        uint32 SenderIndex;
        uint32 ReceiverID;
        uint32 ReceiverIndex;
        CInstancePtr pSender;
        CInstancePtr pReceiver;
    };
    QVector<SDeletedLink> mDeletedLinks;

public:
    explicit CDeleteSelectionCommand(CWorldEditor *pEditor, const QString& rkCommandName = "Delete");
    void undo() override;
    void redo() override;
    bool AffectsCleanState() const override { return true; }
};

#endif // CDELETESELECTIONCOMMAND_H
