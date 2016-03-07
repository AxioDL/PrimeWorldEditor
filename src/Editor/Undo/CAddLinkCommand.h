#ifndef CADDLINKCOMMAND_H
#define CADDLINKCOMMAND_H

#include "IUndoCommand.h"
#include "Editor/WorldEditor/CWorldEditor.h"
#include <Core/Resource/Script/CLink.h>

class CAddLinkCommand : public IUndoCommand
{
    CWorldEditor *mpEditor;
    CLink mLink;
    QList<CScriptObject*> mAffectedInstances;

public:
    CAddLinkCommand(CWorldEditor *pEditor, CLink Link);
    void undo();
    void redo();
    bool AffectsCleanState() const { return true; }
};

#endif // CADDLINKCOMMAND_H
