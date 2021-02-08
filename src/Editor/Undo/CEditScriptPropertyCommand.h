#ifndef CEDITSCRIPTPROPERTYCOMMAND_H
#define CEDITSCRIPTPROPERTYCOMMAND_H

#include "IEditPropertyCommand.h"
#include "ObjReferences.h"
#include "Editor/WorldEditor/CWorldEditor.h"

class CEditScriptPropertyCommand : public IEditPropertyCommand
{
protected:
    QVector<CInstancePtr> mInstances;
    QModelIndex mIndex;

public:
    CEditScriptPropertyCommand(IProperty* pProperty,
                               const QVector<CScriptObject*>& kInstances,
                               CPropertyModel* pModel,
                               QModelIndex Index = QModelIndex(),
                               const QString& kCommandName = "Edit Property")
        : IEditPropertyCommand(pProperty, pModel, Index, kCommandName)
        , mIndex(Index)
    {
        // Convert CScriptObject pointers to CInstancePtrs
        mInstances.reserve(kInstances.size());

        for (auto* instance : kInstances)
            mInstances.push_back(CInstancePtr(instance));
    }

    void GetObjectDataPointers(QVector<void*>& OutPointers) const override
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

        for (int i = 0; i < mInstances.size(); i++)
            OutPointers[i] = mInstances[i]->PropertyData();
    }
};

#endif // CEDITSCRIPTPROPERTYCOMMAND_H
