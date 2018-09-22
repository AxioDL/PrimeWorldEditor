#ifndef CEDITSCRIPTPROPERTYCOMMAND_H
#define CEDITSCRIPTPROPERTYCOMMAND_H

#include "IEditPropertyCommand.h"
#include "ObjReferences.h"
#include "Editor/WorldEditor/CWorldEditor.h"

class CEditScriptPropertyCommand : public IEditPropertyCommand
{
protected:
    QVector<CInstancePtr> mInstances;
    CWorldEditor* mpEditor;
    QModelIndex mIndex;

public:
    CEditScriptPropertyCommand(IProperty* pProperty,
                               CWorldEditor* pEditor,
                               const QVector<CScriptObject*>& rkInstances,
                               QModelIndex Index = QModelIndex(),
                               const QString& rkCommandName = "Edit Property")
        : IEditPropertyCommand(pProperty, rkCommandName)
        , mpEditor(pEditor)
        , mIndex(Index)
    {
        // If the property being passed in is part of an array archetype, then we MUST have a QModelIndex.
        // Without the index, there's no way to identify the correct child being edited.
        if (!Index.isValid() && pProperty && pProperty->IsArrayArchetype())
        {
            while (pProperty && pProperty->IsArrayArchetype())
            {
                pProperty = pProperty->Parent();
            }
            ASSERT(pProperty && !pProperty->IsArrayArchetype());
        }

        // Convert CScriptObject pointers to CInstancePtrs
        mInstances.reserve( rkInstances.size() );

        for (int i = 0; i < rkInstances.size(); i++)
            mInstances.push_back( CInstancePtr(rkInstances[i]) );
    }

    virtual void GetObjectDataPointers(QVector<void*>& rOutPointers) const override
    {
        // todo: support multiple objects being edited at once on the property view
        if (mIndex.isValid())
        {
            ASSERT(mInstances.size() == 1);
            rOutPointers << mInstances[0]->PropertyData();
            return;
        }

        // grab instance pointers
        ASSERT(!mpProperty->IsArrayArchetype());

        rOutPointers.resize(mInstances.size());

        for (int i = 0; i < mInstances.size(); i++)
            rOutPointers[i] = mInstances[i]->PropertyData();
    }

    virtual void undo() override
    {
        IEditPropertyCommand::undo();
        NotifyWorldEditor();
    }

    virtual void redo() override
    {
        IEditPropertyCommand::redo();
        NotifyWorldEditor();
    }

    void NotifyWorldEditor()
    {
        for (int InstanceIdx = 0; InstanceIdx < mInstances.size(); InstanceIdx++)
            mpEditor->OnPropertyModified(*mInstances[InstanceIdx], mpProperty);
    }
};

#endif // CEDITSCRIPTPROPERTYCOMMAND_H
