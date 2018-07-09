#ifndef CEDITSCRIPTPROPERTYCOMMAND_H
#define CEDITSCRIPTPROPERTYCOMMAND_H

#include "IEditPropertyCommand.h"
#include "ObjReferences.h"
#include "Editor/WorldEditor/CWorldEditor.h"

class CEditScriptPropertyCommand : public IEditPropertyCommand
{
    std::vector<CInstancePtr> mInstances;
    CWorldEditor* mpEditor;

public:
    CEditScriptPropertyCommand(CWorldEditor* pEditor, const QModelIndex& rkIndex, CPropertyModel* pInModel, const QString& rkCommandName = "Edit Property")
        : IEditPropertyCommand(rkIndex, pInModel, rkCommandName)
        , mpEditor(pEditor)
    {
        mInstances.push_back( CInstancePtr(pInModel->GetScriptObject()) );
    }

    virtual void GetObjectDataPointers(std::vector<void*>& rOutPointers) const override
    {
        rOutPointers.resize(mInstances.size());

        //@todo support multiple objects
        rOutPointers[0] = mpModel->DataPointerForIndex(mIndex);
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
