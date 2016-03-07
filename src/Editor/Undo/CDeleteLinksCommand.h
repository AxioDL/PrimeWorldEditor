#ifndef CDELETELINKSCOMMAND_H
#define CDELETELINKSCOMMAND_H

#include "IUndoCommand.h"
#include "Editor/WorldEditor/CWorldEditor.h"

class CDeleteLinksCommand : public IUndoCommand
{
    CWorldEditor *mpEditor;
    QList<CScriptObject*> mAffectedInstances;

    struct SDeletedLink
    {
        u32 State;
        u32 Message;
        CScriptObject *pSender;
        CScriptObject *pReceiver;
        u32 SenderIndex;
        u32 ReceiverIndex;
    };
    QVector<SDeletedLink> mLinks;

public:
    CDeleteLinksCommand(CWorldEditor *pEditor, CScriptObject *pObject, ELinkType Type, const QVector<u32>& rkIndices);
    void undo();
    void redo();
    bool AffectsCleanState() const { return true; }
};

#endif // CDELETELINKSCOMMAND_H
