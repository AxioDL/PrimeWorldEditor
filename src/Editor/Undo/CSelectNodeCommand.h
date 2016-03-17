#ifndef CSELECTNODECOMMAND_H
#define CSELECTNODECOMMAND_H

#include "IUndoCommand.h"
#include "ObjReferences.h"
#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>

class CSelectNodeCommand : public IUndoCommand
{
    CNodePtr mpNode;
    CNodeSelection *mpSelection;

public:
    CSelectNodeCommand(CNodeSelection *pSelection, CSceneNode *pNode)
        : IUndoCommand("Select")
        , mpNode(pNode)
        , mpSelection(pSelection)
    {}

    void undo() { mpSelection->DeselectNode(*mpNode); }
    void redo() { mpSelection->SelectNode(*mpNode); }
    bool AffectsCleanState() const { return false; }
};

#endif // CSELECTNODECOMMAND_H
