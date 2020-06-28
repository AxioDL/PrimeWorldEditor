#ifndef CEDITLINKCOMMAND_H
#define CEDITLINKCOMMAND_H

#include "IUndoCommand.h"
#include "ObjReferences.h"
#include "Editor/WorldEditor/CWorldEditor.h"
#include <Core/Resource/Script/CLink.h>

class CEditLinkCommand : public IUndoCommand
{
    CWorldEditor *mpEditor;
    CLinkPtr mpEditLink;

    CLink mOldLink;
    CLink mNewLink;
    uint32 mOldSenderIndex;
    uint32 mOldReceiverIndex;

    CInstancePtrList mAffectedInstances;

public:
    CEditLinkCommand(CWorldEditor *pEditor, CLink *pLink, CLink NewLink);
    QList<CScriptObject*> AffectedInstances() const;
    void undo() override;
    void redo() override;
    bool AffectsCleanState() const override { return true; }
};

#endif // CEDITLINKCOMMAND_H
