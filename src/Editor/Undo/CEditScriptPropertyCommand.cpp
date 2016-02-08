#include "CEditScriptPropertyCommand.h"
#include "EUndoCommand.h"

CEditScriptPropertyCommand::CEditScriptPropertyCommand(IProperty *pProp, CWorldEditor *pEditor, IPropertyValue *pOldValue, bool IsDone, const QString& rkCommandName /*= "Edit Property"*/)
    : CBasicPropertyCommand(pProp, pEditor, rkCommandName)
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
    mpEditor->OnPropertyModified(mpProperty);
    mCommandEnded = true;
}

void CEditScriptPropertyCommand::redo()
{
    if (mIsInArray) UpdateArraySubProperty();
    mpProperty->RawValue()->Copy(mpNewValue);
    mpEditor->OnPropertyModified(mpProperty);
}
