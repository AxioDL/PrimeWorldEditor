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

    void undo() { mpSelection->SelectNode(*mpNode); }
    void redo() { mpSelection->DeselectNode(*mpNode); }
    bool AffectsCleanState() const { return false; }
};

#endif // CDESELECTNODECOMMAND_H
