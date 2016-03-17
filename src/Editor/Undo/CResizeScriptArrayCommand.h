#ifndef CRESIZESCRIPTARRAYCOMMAND_H
#define CRESIZESCRIPTARRAYCOMMAND_H

#include "IUndoCommand.h"
#include "ObjReferences.h"
#include "Editor/PropertyEdit/CPropertyModel.h"
#include "Editor/WorldEditor/CWorldEditor.h"
#include <QUndoCommand>

// todo: make this more general... it shouldn't be relying on a CPropertyModel pointer
class CResizeScriptArrayCommand : public IUndoCommand
{
    CWorldEditor *mpEditor;
    CPropertyPtr mpArray;
    QVector<IProperty*> mDeletedProperties;
    CPropertyModel *mpModel;

    int mOldSize;
    int mNewSize;
    bool mNewSizeLarger;

public:
    CResizeScriptArrayCommand(IProperty *pProp, CWorldEditor *pEditor, CPropertyModel *pModel, int NewSize);
    ~CResizeScriptArrayCommand();
    void undo();
    void redo();
    bool AffectsCleanState() const { return true; }
};

#endif // CRESIZESCRIPTARRAYCOMMAND_H
