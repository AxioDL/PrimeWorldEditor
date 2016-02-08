#ifndef CRESIZESCRIPTARRAYCOMMAND_H
#define CRESIZESCRIPTARRAYCOMMAND_H

#include "CBasicPropertyCommand.h"
#include "Editor/PropertyEdit/CPropertyModel.h"
#include <QUndoCommand>

class CResizeScriptArrayCommand : public CBasicPropertyCommand
{
    CArrayProperty *mpArray;
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
    virtual void UpdateArraySubProperty();
};

#endif // CRESIZESCRIPTARRAYCOMMAND_H
