#ifndef CDESELECTNODECOMMAND_H
#define CDESELECTNODECOMMAND_H

#include "IUndoCommand.h"
#include "ObjReferences.h"
#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>

class CDeselectNodeCommand : public IUndoCommand
{
    CNodePtr mpNode;
    CNodeSelection *mpSelection;
public:
    CDeselectNodeCommand(CNodeSelection *pSelection, CSceneNode *pNode)
        : IUndoCommand("Deselect")
        , mpNode(pNode)
        , mpSelection(pSelection)
    {}

    void undo() override { mpSelection->SelectNode(*mpNode); }
    void redo() override { mpSelection->DeselectNode(*mpNode); }
    bool AffectsCleanState() const override { return false; }
};

#endif // CDESELECTNODECOMMAND_H
