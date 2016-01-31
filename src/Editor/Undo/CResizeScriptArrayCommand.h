#ifndef CRESIZESCRIPTARRAYCOMMAND_H
#define CRESIZESCRIPTARRAYCOMMAND_H

#include "CBasicPropertyCommand.h"
#include "Editor/PropertyEdit/CPropertyModel.h"
#include <QUndoCommand>

class CResizeScriptArrayCommand : public CBasicPropertyCommand
{
    CArrayProperty *mpArray;
    QVector<IProperty*> mDeletedProperties;

    u32 mOldSize;
    u32 mNewSize;
    bool mNewSizeLarger;

public:
    CResizeScriptArrayCommand(CPropertyModel *pModel, const QModelIndex& rkIndex, u32 NewSize);
    ~CResizeScriptArrayCommand();
    void undo();
    void redo();
    virtual void UpdateArraySubProperty();
};

#endif // CRESIZESCRIPTARRAYCOMMAND_H
