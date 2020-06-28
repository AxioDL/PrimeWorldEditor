#ifndef CADDLINKCOMMAND_H
#define CADDLINKCOMMAND_H

#include "IUndoCommand.h"
#include "ObjReferences.h"
#include "Editor/WorldEditor/CWorldEditor.h"
#include <Core/Resource/Script/CLink.h>

class CAddLinkCommand : public IUndoCommand
{
    CWorldEditor *mpEditor;
    CLink mLink;
    CInstancePtrList mAffectedInstances;

public:
    CAddLinkCommand(CWorldEditor *pEditor, CLink Link);
    void undo() override;
    void redo() override;
    bool AffectsCleanState() const override { return true; }
};

#endif // CADDLINKCOMMAND_H
