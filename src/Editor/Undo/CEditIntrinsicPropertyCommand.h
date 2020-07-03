#ifndef CEDITINTRINSICPROPERTYCOMMAND_H
#define CEDITINTRINSICPROPERTYCOMMAND_H

#include "IEditPropertyCommand.h"

class CEditIntrinsicPropertyCommand : public IEditPropertyCommand
{
protected:
    QVector<void*> mDataPointers;

public:
    CEditIntrinsicPropertyCommand(IProperty* pProperty,
                                  const QVector<void*>& kDataPointers,
                                  CPropertyModel* pModel,
                                  QModelIndex Index = QModelIndex(),
                                  const QString& kCommandName = "Edit Property")
        : IEditPropertyCommand(pProperty, pModel, Index, kCommandName)
        , mDataPointers(kDataPointers)
    {
    }

    void GetObjectDataPointers(QVector<void*>& rOutPointers) const override
    {
        rOutPointers = mDataPointers;
    }
};

#endif // CEDITINTRINSICPROPERTYCOMMAND_H
