#ifndef TSERIALIZEUNDOCOMMAND_H
#define TSERIALIZEUNDOCOMMAND_H

#include "IUndoCommand.h"
#include <Common/Common.h>

/**
 * Undo command that works by restoring the full state of an object
 * on undo/redo. To use, create the command object, apply the change
 * you want to make to the object, and then push the command.
 *
 * Commands with IsActionComplete=false will be merged.
 * To prevent merging, push a final command with IsActionComplete=true.
 *
 * The current implementatation is "dumb" and just saves and restores
 * the entire object, including parameters that have not changed. Due
 * to this, it could be memory-intensive and not super performant.
 * As a result, it's probably not a good idea to use this on very
 * large objects right now.
 */
template<typename ObjectT>
class TSerializeUndoCommand : public IUndoCommand
{
    ObjectT* mpObject;
    std::vector<char> mOldData;
    std::vector<char> mNewData;
    bool mIsActionComplete;

public:
    TSerializeUndoCommand(const QString& kText, ObjectT* pObject, bool IsActionComplete)
        : IUndoCommand(kText)
        , mpObject(pObject)
        , mIsActionComplete(IsActionComplete)
    {
        // Save old state of object
        CVectorOutStream Out(&mOldData, EEndian::SystemEndian);
        CBasicBinaryWriter Writer(&Out, 0, EGame::Invalid);
        mpObject->Serialize(Writer);
    }

    /** IUndoCommand interface */
    int id() const override
    {
        return FOURCC('TSUC');
    }

    void undo() override
    {
        // Restore old state of object
        CMemoryInStream In(&mOldData[0], mOldData.size(), EEndian::SystemEndian);
        CBasicBinaryReader Reader(&In, CSerialVersion(0,0,EGame::Invalid));
        mpObject->Serialize(Reader);
    }

    void redo() override
    {
        // First call when command is pushed - save new state of object
        if (mNewData.empty())
        {
            CVectorOutStream Out(&mNewData, EEndian::SystemEndian);
            CBasicBinaryWriter Writer(&Out, 0, EGame::Invalid);
            mpObject->Serialize(Writer);

            // Obsolete command if memory buffers match
            if (mIsActionComplete)
            {
                if (mOldData.size() == mNewData.size())
                {
                    if (memcmp(mOldData.data(), mNewData.data(), mNewData.size()) == 0)
                    {
                        setObsolete(true);
                    }
                }
            }
        }
        // Subsequent calls - restore new state of object
        else
        {
            CMemoryInStream In(&mNewData[0], mNewData.size(), EEndian::SystemEndian);
            CBasicBinaryReader Reader(&In, CSerialVersion(0,0,EGame::Invalid));
            mpObject->Serialize(Reader);
        }
    }

    bool mergeWith(const QUndoCommand* pkOther) override
    {
        if (!mIsActionComplete && pkOther->id() == id())
        {
            const TSerializeUndoCommand* pkSerializeCommand =
                    static_cast<const TSerializeUndoCommand*>(pkOther);

            mNewData = pkSerializeCommand->mNewData;
            mIsActionComplete = pkSerializeCommand->mIsActionComplete;

            // Obsolete command if memory buffers match
            if (mIsActionComplete)
            {
                if (mOldData.size() == mNewData.size())
                {
                    if (memcmp(mOldData.data(), mNewData.data(), mNewData.size()) == 0)
                    {
                        setObsolete(true);
                    }
                }
            }

            return true;
        }
        return false;
    }

    bool AffectsCleanState() const override
    {
        return true;
    }
};

#endif // TSERIALIZEUNDOCOMMAND_H
