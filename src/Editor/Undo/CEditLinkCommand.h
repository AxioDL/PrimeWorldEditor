#ifndef CEDITLINKCOMMAND_H
#define CEDITLINKCOMMAND_H

#include "Editor/Undo/IUndoCommand.h"
#include "Editor/Undo/ObjReferences.h"

#include <Core/Resource/Script/CLink.h>

class CWorldEditor;

class CEditLinkCommand : public IUndoCommand
{
    CWorldEditor *mpEditor;
    CLinkPtr mpEditLink;

    CLink mOldLink;
    CLink mNewLink;
    uint32_t mOldSenderIndex;
    uint32_t mOldReceiverIndex;

    CInstancePtrList mAffectedInstances;

public:
    CEditLinkCommand(CWorldEditor *pEditor, CLink *pLink, const CLink& NewLink);

    void undo() override;
    void redo() override;
    bool AffectsCleanState() const override { return true; }
};

#endif // CEDITLINKCOMMAND_H
