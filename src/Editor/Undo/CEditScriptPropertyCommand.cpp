#include "CEditScriptPropertyCommand.h"
#include "EUndoCommand.h"

CEditScriptPropertyCommand::CEditScriptPropertyCommand(CPropertyModel *pModel, const QModelIndex& rkIndex, IPropertyValue *pOldValue, bool IsDone)
    : QUndoCommand("Edit Property")
    , mpModel(pModel)
    , mIndex(rkIndex)
    , mCommandEnded(IsDone)
{
    mpProperty = pModel->PropertyForIndex(rkIndex, true);
    mpOldValue = pOldValue;
    mpNewValue = mpProperty->RawValue()->Clone();
}

CEditScriptPropertyCommand::~CEditScriptPropertyCommand()
{
    delete mpOldValue;
    delete mpNewValue;
}

int CEditScriptPropertyCommand::id() const
{
    return eEditScriptPropertyCmd;
}

bool CEditScriptPropertyCommand::mergeWith(const QUndoCommand *pkOther)
{
    if (!mCommandEnded && pkOther->id() == eEditScriptPropertyCmd)
    {
        const CEditScriptPropertyCommand *pkCmd = static_cast<const CEditScriptPropertyCommand*>(pkOther);

        if (pkCmd->mpProperty == mpProperty)
        {
            mpNewValue->Copy(pkCmd->mpNewValue);
            mCommandEnded = pkCmd->mCommandEnded;
            return true;
        }
    }

    return false;
}

void CEditScriptPropertyCommand::undo()
{
    mpProperty->RawValue()->Copy(mpOldValue);
    mpModel->NotifyPropertyModified(mIndex);
}

void CEditScriptPropertyCommand::redo()
{
    mpProperty->RawValue()->Copy(mpNewValue);
    mpModel->NotifyPropertyModified(mIndex);
}
