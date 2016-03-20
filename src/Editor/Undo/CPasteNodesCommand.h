#ifndef CPASTENODESCOMMAND
#define CPASTENODESCOMMAND

#include "IUndoCommand.h"
#include "ObjReferences.h"
#include "Editor/CNodeCopyMimeData.h"
#include "Editor/WorldEditor/CWorldEditor.h"
#include <QClipboard>

class CPasteNodesCommand : public IUndoCommand
{
    CWorldEditor *mpEditor;
    CScriptLayer *mpLayer;
    CVector3f mPastePoint;
    CNodeCopyMimeData *mpMimeData;
    CNodePtrList mPastedNodes;
    CNodePtrList mOriginalSelection;

public:
    CPasteNodesCommand(CWorldEditor *pEditor, CScriptLayer *pLayer, CVector3f PastePoint);
    ~CPasteNodesCommand();
    void undo();
    void redo();

    bool AffectsCleanState() const { return true; }
};

#endif // CPASTENODESCOMMAND

