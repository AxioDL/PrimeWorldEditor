#ifndef CDELETELINKSCOMMAND_H
#define CDELETELINKSCOMMAND_H

#include "IUndoCommand.h"
#include "ObjReferences.h"
#include "Editor/WorldEditor/CWorldEditor.h"

class CDeleteLinksCommand : public IUndoCommand
{
    CWorldEditor *mpEditor;
    CInstancePtrList mAffectedInstances;

    struct SDeletedLink
    {
        uint32 State;
        uint32 Message;
        CInstancePtr pSender;
        CInstancePtr pReceiver;
        uint32 SenderIndex;
        uint32 ReceiverIndex;
    };
    QVector<SDeletedLink> mLinks;

public:
    CDeleteLinksCommand() {}
    CDeleteLinksCommand(CWorldEditor *pEditor, CScriptObject *pObject, ELinkType Type, const QVector<uint32>& rkIndices);
    void undo();
    void redo();
    bool AffectsCleanState() const { return true; }
};

#endif // CDELETELINKSCOMMAND_H
