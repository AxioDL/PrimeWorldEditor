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
        u32 State;
        u32 Message;
        CInstancePtr pSender;
        CInstancePtr pReceiver;
        u32 SenderIndex;
        u32 ReceiverIndex;
    };
    QVector<SDeletedLink> mLinks;

public:
    CDeleteLinksCommand() {}
    CDeleteLinksCommand(CWorldEditor *pEditor, CScriptObject *pObject, ELinkType Type, const QVector<u32>& rkIndices);
    void undo();
    void redo();
    bool AffectsCleanState() const { return true; }
};

#endif // CDELETELINKSCOMMAND_H
