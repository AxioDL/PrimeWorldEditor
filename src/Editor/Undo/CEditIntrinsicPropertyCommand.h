#ifndef CEDITINTRINSICPROPERTYCOMMAND_H
#define CEDITINTRINSICPROPERTYCOMMAND_H

#include "Editor/Undo/IEditPropertyCommand.h"
#include <QCoreApplication>

class CEditIntrinsicPropertyCommand : public IEditPropertyCommand
{
protected:
    QList<void*> mDataPointers;

public:
    CEditIntrinsicPropertyCommand(IProperty* pProperty,
                                  const QList<void*>& kDataPointers,
                                  CPropertyModel* pModel,
                                  const QModelIndex& Index = {},
                                  const QString& kCommandName = QCoreApplication::translate("CEditIntrinsicPropertyCommand", "Edit Property"))
        : IEditPropertyCommand(pProperty, pModel, Index, kCommandName)
        , mDataPointers(kDataPointers)
    {
    }

    void GetObjectDataPointers(QList<void*>& rOutPointers) const override
    {
        rOutPointers = mDataPointers;
    }
};

#endif // CEDITINTRINSICPROPERTYCOMMAND_H
