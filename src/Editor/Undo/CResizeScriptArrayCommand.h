#ifndef CRESIZESCRIPTARRAYCOMMAND_H
#define CRESIZESCRIPTARRAYCOMMAND_H

#include "CEditScriptPropertyCommand.h"

class CResizeScriptArrayCommand : public CEditScriptPropertyCommand
{
    /** Old/new model row counts; we store this here to support editing arrays on multiple instances at once */
    int mOldRowCount = -1;
    int mNewRowCount = -1;

public:
    CResizeScriptArrayCommand(IProperty* pProperty,
                              const QVector<CScriptObject*>& rkInstances,
                              CPropertyModel* pModel,
                              QModelIndex Index = QModelIndex(),
                              const QString& rkCommandName = "Resize Array"
                )
        :   CEditScriptPropertyCommand(pProperty, rkInstances, pModel, Index, rkCommandName)
    {
    }

    bool mergeWith(const QUndoCommand *pkOther) override
    {
        return false;
    }

    void SaveOldData() override
    {
        CEditScriptPropertyCommand::SaveOldData();

        if (mpModel)
        {
            mOldRowCount = mpModel->rowCount(mIndex);
        }
    }

    void SaveNewData() override
    {
        CEditScriptPropertyCommand::SaveNewData();

        if (mpModel)
        {
            mNewRowCount = mpModel->rowCount(mIndex);
        }
    }

    // Note in some cases undo/redo may be called when the change has already been applied outside of the undo command
    // This is why we need to check the array's actual current size instead of assuming it will match one of the arrays
    void undo() override
    {
        //@todo verify, do we need to fully override undo()?
        if (mpModel)
        {
            mpModel->ArrayAboutToBeResized(mIndex, mOldRowCount);
        }

        CEditScriptPropertyCommand::undo();

        if (mpModel)
        {
            mpModel->ArrayResized(mIndex, mNewRowCount);
        }
    }

    void redo() override
    {
        if (mpModel)
        {
            mpModel->ArrayAboutToBeResized(mIndex, mNewRowCount);
        }

        CEditScriptPropertyCommand::redo();

        if (mpModel)
        {
            mpModel->ArrayResized(mIndex, mOldRowCount);
        }
    }
};

#endif // CRESIZESCRIPTARRAYCOMMAND_H
