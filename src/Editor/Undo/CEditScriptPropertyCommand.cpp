#include "CEditScriptPropertyCommand.h"
#include "EUndoCommand.h"

CEditScriptPropertyCommand::CEditScriptPropertyCommand(IProperty *pProp, CWorldEditor *pEditor, IPropertyValue *pOldValue, bool IsDone, const QString& rkCommandName /*= "Edit Property"*/)
    : IUndoCommand(rkCommandName)
    , mpProp(pProp)
    , mpEditor(pEditor)
    , mCommandEnded(IsDone)
{
    mpOldValue = pOldValue;
    mpNewValue = pProp->RawValue()->Clone();
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

        if (pkCmd->mpProp == mpProp)
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
    IProperty *pProp = *mpProp;
    pProp->RawValue()->Copy(mpOldValue);
    mpEditor->OnPropertyModified(pProp);
    mCommandEnded = true;
}

void CEditScriptPropertyCommand::redo()
{
    IProperty *pProp = *mpProp;
    pProp->RawValue()->Copy(mpNewValue);
    mpEditor->OnPropertyModified(pProp);
}
