#include "IEditPropertyCommand.h"
#include "Editor/CEditorApplication.h"
#include "Editor/WorldEditor/CWorldEditor.h"

/** Save the current state of the object properties to the given data buffer */
void IEditPropertyCommand::SaveObjectStateToArray(std::vector<char>& rVector)
{
    CVectorOutStream MemStream(&rVector, EEndian::SystemEndian);
    CBasicBinaryWriter Writer(&MemStream, CSerialVersion(IArchive::skCurrentArchiveVersion, 0, mpProperty->Game()));

    QVector<void*> DataPointers;
    GetObjectDataPointers(DataPointers);

    for (void* pData : DataPointers)
    {
        mpProperty->SerializeValue(pData, Writer);
    }
}

/** Restore the state of the object properties from the given data buffer */
void IEditPropertyCommand::RestoreObjectStateFromArray(std::vector<char>& rArray)
{
    CBasicBinaryReader Reader(rArray.data(), rArray.size(), CSerialVersion(IArchive::skCurrentArchiveVersion, 0, mpProperty->Game()));

    QVector<void*> DataPointers;
    GetObjectDataPointers(DataPointers);

    for (void* pData : DataPointers)
    {
       mpProperty->SerializeValue(pData, Reader);
    }
}

IEditPropertyCommand::IEditPropertyCommand(
        IProperty* pProperty,
        CPropertyModel* pModel,
        const QModelIndex& kIndex,
        const QString& kCommandName /*= "Edit Property"*/
        )
    : IUndoCommand(kCommandName)
    , mpProperty(pProperty)
    , mpModel(pModel)
    , mIndex(kIndex)
{
    ASSERT(mpProperty);

    if (!mIndex.isValid())
    {
        // If the property being passed in is part of an array archetype, then we MUST have a QModelIndex.
        // Without the index, there's no way to identify the correct child being edited.
        // So if we don't have an index, we need to serialize the entire array property.
        if (mpProperty->IsArrayArchetype())
        {
            while (mpProperty && mpProperty->IsArrayArchetype())
            {
                mpProperty = mpProperty->Parent();
            }
            ASSERT(mpProperty && !mpProperty->IsArrayArchetype());
        }

        // Now we can fetch the index from the model
        mIndex = mpModel->IndexForProperty(mpProperty);
    }
}

void IEditPropertyCommand::SaveOldData()
{
    SaveObjectStateToArray(mOldData);
    mSavedOldData = true;
}

void IEditPropertyCommand::SaveNewData()
{
    SaveObjectStateToArray(mNewData);
    mSavedNewData = true;
}

bool IEditPropertyCommand::IsNewDataDifferent()
{
    if (mOldData.size() != mNewData.size()) return true;
    return memcmp(mOldData.data(), mNewData.data(), mNewData.size()) != 0;
}

void IEditPropertyCommand::SetEditComplete(bool IsComplete)
{
    mCommandEnded = IsComplete;
}

/** IUndoCommand/QUndoCommand interface */
int IEditPropertyCommand::id() const
{
    return (int) EUndoCommand::EditPropertyCmd;
}

bool IEditPropertyCommand::mergeWith(const QUndoCommand *pkOther)
{
    if (!mCommandEnded)
    {
        const IEditPropertyCommand* pkCmd = dynamic_cast<const IEditPropertyCommand*>(pkOther);

        if (pkCmd && pkCmd->mpProperty == mpProperty)
        {
            QVector<void*> MyPointers;
            GetObjectDataPointers(MyPointers);

            QVector<void*> TheirPointers;
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

void IEditPropertyCommand::undo()
{
    ASSERT(mSavedOldData && mSavedNewData);
    RestoreObjectStateFromArray(mOldData);
    mCommandEnded = true;

    if (mpModel && mIndex.isValid())
    {
        mpModel->NotifyPropertyModified(mIndex);
    }
    else
    {
        gpEdApp->WorldEditor()->OnPropertyModified(mpProperty);
    }
}

void IEditPropertyCommand::redo()
{
    ASSERT(mSavedOldData && mSavedNewData);
    RestoreObjectStateFromArray(mNewData);

    if (mpModel && mIndex.isValid())
    {
        mpModel->NotifyPropertyModified(mIndex);
    }
    else
    {
        gpEdApp->WorldEditor()->OnPropertyModified(mpProperty);
    }
}

bool IEditPropertyCommand::AffectsCleanState() const
{
    return true;
}
