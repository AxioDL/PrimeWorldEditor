#ifndef CEDITSCRIPTPROPERTYCOMMAND_H
#define CEDITSCRIPTPROPERTYCOMMAND_H

#include "Editor/Undo/IEditPropertyCommand.h"
#include "Editor/Undo/ObjReferences.h"

#include <QCoreApplication>
#include <span>

class CEditScriptPropertyCommand : public IEditPropertyCommand
{
protected:
    QList<CInstancePtr> mInstances;
    QModelIndex mIndex;

public:
    CEditScriptPropertyCommand(IProperty* pProperty,
                               std::span<CScriptObject*> kInstances,
                               CPropertyModel* pModel,
                               const QModelIndex& Index = {},
                               const QString& kCommandName = QCoreApplication::translate("CEditScriptPropertyCommand", "Edit Property"))
        : IEditPropertyCommand(pProperty, pModel, Index, kCommandName)
        , mIndex(Index)
    {
        // Convert CScriptObject pointers to CInstancePtrs
        mInstances.reserve(kInstances.size());

        for (auto* instance : kInstances)
            mInstances.push_back(CInstancePtr(instance));
    }

    void GetObjectDataPointers(QList<void*>& OutPointers) const override
    {
        // todo: support multiple objects being edited at once on the property view
        if (mIndex.isValid())
        {
            ASSERT(mInstances.size() == 1);
            OutPointers.push_back(mInstances[0]->PropertyData());
            return;
        }

        // grab instance pointers
        ASSERT(!mpProperty->IsArrayArchetype());

        OutPointers.resize(mInstances.size());

        for (qsizetype i = 0; i < mInstances.size(); i++)
            OutPointers[i] = mInstances[i]->PropertyData();
    }
};

#endif // CEDITSCRIPTPROPERTYCOMMAND_H
