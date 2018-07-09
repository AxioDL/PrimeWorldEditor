#ifndef CRESIZESCRIPTARRAYCOMMAND_H
#define CRESIZESCRIPTARRAYCOMMAND_H

#include "CEditScriptPropertyCommand.h"

class CResizeScriptArrayCommand : public CEditScriptPropertyCommand
{
public:
    CResizeScriptArrayCommand(CWorldEditor* pEditor, const QModelIndex& rkIndex, CPropertyModel* pInModel, const QString& rkCommandName = "Resize Array")
        : CEditScriptPropertyCommand(pEditor, rkIndex, pInModel, rkCommandName)
    {}

    bool mergeWith(const QUndoCommand *pkOther)
    {
        return false;
    }

    // Note in some cases undo/redo may be called when the change has already been applied outside of the undo command
    // This is why we need to check the array's actual current size instead of assuming it will match one of the arrays
    void undo()
    {
        // unpleasant cast, but easiest/fastest way to access the sizes
        int NewSize = *((int*)mOldData.data());
        int OldSize = CurrentArrayCount();

        mpModel->ArrayAboutToBeResized(mIndex, NewSize);
        CEditScriptPropertyCommand::undo();
        mpModel->ArrayResized(mIndex, OldSize);
    }

    void redo()
    {
        // unpleasant cast, but easiest/fastest way to access the sizes
        int NewSize = *((int*)mNewData.data());
        int OldSize = CurrentArrayCount();

        mpModel->ArrayAboutToBeResized(mIndex, NewSize);
        CEditScriptPropertyCommand::redo();
        mpModel->ArrayResized(mIndex, OldSize);
    }

    int CurrentArrayCount()
    {
        void* pData = mpModel->DataPointerForIndex(mIndex);
        CArrayProperty* pArray = TPropCast<CArrayProperty>(mpProperty);
        return pArray->ArrayCount(pData);
    }
};

#endif // CRESIZESCRIPTARRAYCOMMAND_H
