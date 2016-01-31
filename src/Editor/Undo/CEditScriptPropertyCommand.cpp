#include "CEditScriptPropertyCommand.h"
#include "EUndoCommand.h"

CEditScriptPropertyCommand::CEditScriptPropertyCommand(CPropertyModel *pModel, const QModelIndex& rkIndex, IPropertyValue *pOldValue, bool IsDone)
    : CBasicPropertyCommand(pModel, rkIndex)
    , mCommandEnded(IsDone)
{
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
    if (mIsInArray) UpdateArraySubProperty();
    mpProperty->RawValue()->Copy(mpOldValue);
    mpModel->NotifyPropertyModified(mIndex);
}

void CEditScriptPropertyCommand::redo()
{
    if (mIsInArray) UpdateArraySubProperty();
    mpProperty->RawValue()->Copy(mpNewValue);
    mpModel->NotifyPropertyModified(mIndex);
}
