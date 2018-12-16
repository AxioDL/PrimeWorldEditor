#include "IEditPropertyCommand.h"

/** Save the current state of the object properties to the given data buffer */
void IEditPropertyCommand::SaveObjectStateToArray(std::vector<char>& rVector)
{
    CVectorOutStream MemStream(&rVector, EEndian::SystemEndian);
    CBasicBinaryWriter Writer(&MemStream, CSerialVersion(IArchive::skCurrentArchiveVersion, 0, mpProperty->Game()));

    QVector<void*> DataPointers;
    GetObjectDataPointers(DataPointers);

    foreach (void* pData, DataPointers)
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

    foreach (void* pData, DataPointers)
    {
       mpProperty->SerializeValue(pData, Reader);
    }
}

IEditPropertyCommand::IEditPropertyCommand(
        IProperty* pProperty,
        const QString& rkCommandName /*= "Edit Property"*/
        )
    : IUndoCommand(rkCommandName)
    , mpProperty(pProperty)
    , mSavedOldData(false)
    , mSavedNewData(false)
{
    ASSERT(mpProperty);
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
}

void IEditPropertyCommand::redo()
{
    ASSERT(mSavedOldData && mSavedNewData);
    RestoreObjectStateFromArray(mNewData);
}

bool IEditPropertyCommand::AffectsCleanState() const
{
    return true;
}
