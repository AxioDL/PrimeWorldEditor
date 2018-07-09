#ifndef CEDITSCRIPTPROPERTYCOMMAND_H
#define CEDITSCRIPTPROPERTYCOMMAND_H

#include "IUndoCommand.h"
#include "ObjReferences.h"
#include "EUndoCommand.h"
#include "Editor/PropertyEdit/CPropertyModel.h"
#include "Editor/WorldEditor/CWorldEditor.h"

class IEditPropertyCommand : public IUndoCommand
{
    std::vector<char> mOldData;
    std::vector<char> mNewData;

protected:
    IPropertyNew* mpProperty;
    bool mCommandEnded;
    bool mSavedOldData;
    bool mSavedNewData;

    /** Save the current state of the object properties to the given data buffer */
    void SaveObjectStateToArray(std::vector<char>& rVector)
    {
        CVectorOutStream MemStream(&rVector, IOUtil::kSystemEndianness);
        CBasicBinaryWriter Writer(&MemStream, CSerialVersion(IArchive::skCurrentArchiveVersion, 0, mpProperty->Game()));

        std::vector<void*> DataPointers;
        GetObjectDataPointers(DataPointers);

        for (int PtrIdx = 0; PtrIdx < DataPointers.size(); PtrIdx++)
        {
            void* pData = DataPointers[PtrIdx];
            mpProperty->SerializeValue(pData, Writer);
        }
    }

    /** Restore the state of the object properties from the given data buffer */
    void RestoreObjectStateFromArray(std::vector<char>& rArray)
    {
        CBasicBinaryReader Reader(rArray.data(), rArray.size(), CSerialVersion(IArchive::skCurrentArchiveVersion, 0, mpProperty->Game()));

        std::vector<void*> DataPointers;
        GetObjectDataPointers(DataPointers);

        for (int PtrIdx = 0; PtrIdx < DataPointers.size(); PtrIdx++)
        {
           void* pData = DataPointers[PtrIdx];
           mpProperty->SerializeValue(pData, Reader);
        }
    }

public:
    IEditPropertyCommand(IPropertyNew* pProperty, const QString& rkCommandName = "Edit Property")
        : IUndoCommand(rkCommandName)
        , mpProperty(pProperty)
        , mSavedOldData(false)
        , mSavedNewData(false)
    {}

    void SaveOldData()
    {
        SaveObjectStateToArray(mOldData);
        mSavedOldData = true;
    }

    void SaveNewData()
    {
        SaveObjectStateToArray(mNewData);
        mSavedNewData = true;
    }

    bool IsNewDataDifferent()
    {
        if (mOldData.size() != mNewData.size()) return false;
        return memcmp(mOldData.data(), mNewData.data(), mNewData.size()) != 0;
    }

    void SetEditComplete(bool IsComplete)
    {
        mCommandEnded = IsComplete;
    }

    /** Interface */
    virtual void GetObjectDataPointers(std::vector<void*>& rOutPointers) const = 0;

    /** IUndoCommand/QUndoCommand interface */
    int id() const
    {
        return eEditScriptPropertyCmd;
    }

    bool mergeWith(const QUndoCommand *pkOther)
    {
        if (!mCommandEnded)
        {
            const IEditPropertyCommand* pkCmd = dynamic_cast<const IEditPropertyCommand*>(pkOther);

            if (pkCmd && pkCmd->mpProperty == mpProperty)
            {
                std::vector<void*> MyPointers;
                GetObjectDataPointers(MyPointers);

                std::vector<void*> TheirPointers;
                pkCmd->GetObjectDataPointers(TheirPointers);

                if (TheirPointers.size() == MyPointers.size())
                {
                    for (int PtrIdx = 0; PtrIdx < MyPointers.size(); PtrIdx++)
                    {
                        if (MyPointers[PtrIdx] != TheirPointers[PtrIdx])
                            return false;
                    }

                    // Match
                    mNewData = pkCmd->mNewData;
                    mCommandEnded = pkCmd->mCommandEnded;
                    return true;
                }
            }
        }

        return false;
    }

    void undo()
    {
        ASSERT(mSavedOldData && mSavedNewData);
        RestoreObjectStateFromArray(mOldData);
        mCommandEnded = true;
    }

    void redo()
    {
        ASSERT(mSavedOldData && mSavedNewData);
        RestoreObjectStateFromArray(mNewData);
    }

    bool AffectsCleanState() const
    {
        return true;
    }
};

class CEditScriptPropertyCommand : public IEditPropertyCommand
{
    std::vector<CInstancePtr> mInstances;
    CWorldEditor* mpEditor;

public:
    CEditScriptPropertyCommand(CWorldEditor* pEditor, CScriptObject* pInstance, IPropertyNew* pProperty, const QString& rkCommandName = "Edit Property")
        : IEditPropertyCommand(pProperty, rkCommandName)
        , mpEditor(pEditor)
    {
        mInstances.push_back( CInstancePtr(pInstance) );
    }

    virtual void GetObjectDataPointers(std::vector<void*>& rOutPointers) const override
    {
        rOutPointers.resize(mInstances.size());

        for (int InstanceIdx = 0; InstanceIdx < mInstances.size(); InstanceIdx++)
        {
            rOutPointers[InstanceIdx] = mInstances[InstanceIdx]->PropertyData();
        }
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
