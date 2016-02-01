#ifndef CBASICPROPERTYCOMMAND_H
#define CBASICPROPERTYCOMMAND_H

#include "IUndoCommand.h"
#include "Editor/PropertyEdit/CPropertyModel.h"

class CBasicPropertyCommand : public IUndoCommand
{
protected:
    CPropertyModel *mpModel;
    IProperty *mpProperty;
    IPropertyTemplate *mpTemplate;
    QModelIndex mIndex;

    bool mIsInArray;
    QVector<u32> mArrayIndices;

public:
    CBasicPropertyCommand(CPropertyModel *pModel, const QModelIndex& rkIndex);
    virtual void UpdateArraySubProperty();
    virtual bool AffectsCleanState() const { return true; }
};

#endif // CBASICPROPERTYCOMMAND_H
