#ifndef CBASICPROPERTYCOMMAND_H
#define CBASICPROPERTYCOMMAND_H

#include "Editor/PropertyEdit/CPropertyModel.h"
#include <QUndoCommand>

class CBasicPropertyCommand : public QUndoCommand
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
};

#endif // CBASICPROPERTYCOMMAND_H
