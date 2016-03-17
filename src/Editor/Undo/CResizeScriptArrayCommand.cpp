#include "CResizeScriptArrayCommand.h"

CResizeScriptArrayCommand::CResizeScriptArrayCommand(IProperty *pProp, CWorldEditor *pEditor, CPropertyModel *pModel, int NewSize)
    : IUndoCommand("Edit Property")
    , mpEditor(pEditor)
    , mpArray(pProp)
    , mpModel(pModel)
    , mOldSize(static_cast<CArrayProperty*>(pProp)->Count())
    , mNewSize(NewSize)
{
    mNewSizeLarger = mNewSize > mOldSize;

    if (!mNewSizeLarger)
    {
        CArrayProperty *pArray = static_cast<CArrayProperty*>(pProp);

        for (int iSub = mNewSize; iSub < mOldSize; iSub++)
        {
            mDeletedProperties << pArray->PropertyByIndex(iSub)->Clone(nullptr);
        }
    }
}

CResizeScriptArrayCommand::~CResizeScriptArrayCommand()
{
    foreach (IProperty *pProp, mDeletedProperties)
        delete pProp;
}

void CResizeScriptArrayCommand::undo()
{
    if (mNewSize != mOldSize)
    {
        // Update parents
        CArrayProperty *pArray = static_cast<CArrayProperty*>(*mpArray);

        foreach (IProperty *pProp, mDeletedProperties)
            pProp->SetParent(pArray);

        // Resize array
        QModelIndex Index = mpModel->IndexForProperty(pArray);
        mpModel->ArrayAboutToBeResized(Index, (u32) mOldSize);
        pArray->Resize(mOldSize);

        if (!mNewSizeLarger)
        {
            int NumNewElements = mOldSize - mNewSize;

            for (int iSub = 0; iSub < NumNewElements; iSub++)
            {
                u32 Idx = iSub + mNewSize;
                pArray->PropertyByIndex(Idx)->Copy(mDeletedProperties[iSub]);
            }
        }

        mpModel->ArrayResized(Index, (u32) mNewSize);
    }
}

void CResizeScriptArrayCommand::redo()
{
    // Whether we're increasing or decreasing in size, there's no need to restore deleted properties on redo.
    if (mNewSize != mOldSize)
    {
        CArrayProperty *pArray = static_cast<CArrayProperty*>(*mpArray);
        QModelIndex Index = mpModel->IndexForProperty(pArray);
        mpModel->ArrayAboutToBeResized(Index, (u32) mNewSize);
        pArray->Resize(mNewSize);
        mpModel->ArrayResized(Index, (u32) mOldSize);
    }
}
