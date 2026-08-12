#ifndef CADDLINKCOMMAND_H
#define CADDLINKCOMMAND_H

#include "Editor/Undo/IUndoCommand.h"
#include "Editor/Undo/ObjReferences.h"
#include <Core/Resource/Script/CLink.h>

class CWorldEditor;

class CAddLinkCommand : public IUndoCommand
{
    CWorldEditor *mpEditor;
    CLink mLink;
    CInstancePtrList mAffectedInstances;

public:
    CAddLinkCommand(CWorldEditor *pEditor, const CLink& Link);
    void undo() override;
    void redo() override;
    bool AffectsCleanState() const override { return true; }
};

#endif // CADDLINKCOMMAND_H
