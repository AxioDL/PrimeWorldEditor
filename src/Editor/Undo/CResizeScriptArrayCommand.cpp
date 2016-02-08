#include "CResizeScriptArrayCommand.h"

CResizeScriptArrayCommand::CResizeScriptArrayCommand(IProperty *pProp, CWorldEditor *pEditor, CPropertyModel *pModel, int NewSize)
    : CBasicPropertyCommand(pProp, pEditor)
    , mpArray(static_cast<CArrayProperty*>(mpProperty))
    , mpModel(pModel)
    , mOldSize(mpArray->Count())
    , mNewSize(NewSize)
{
    mNewSizeLarger = mNewSize > mOldSize;

    if (!mNewSizeLarger)
    {
        for (int iSub = mNewSize; iSub < mOldSize; iSub++)
        {
            mDeletedProperties << mpArray->PropertyByIndex(iSub)->Clone();
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
        if (mIsInArray) UpdateArraySubProperty();

        QModelIndex Index = mpModel->IndexForProperty(mpProperty);
        mpModel->ArrayAboutToBeResized(Index, (u32) mOldSize);
        mpArray->Resize(mOldSize);

        if (!mNewSizeLarger)
        {
            int NumNewElements = mOldSize - mNewSize;

            for (int iSub = 0; iSub < NumNewElements; iSub++)
            {
                u32 Idx = iSub + mNewSize;
                mpArray->PropertyByIndex(Idx)->Copy(mDeletedProperties[iSub]);
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
        if (mIsInArray) UpdateArraySubProperty();

        QModelIndex Index = mpModel->IndexForProperty(mpProperty);
        mpModel->ArrayAboutToBeResized(Index, (u32) mNewSize);
        mpArray->Resize(mNewSize);
        mpModel->ArrayResized(Index, (u32) mOldSize);
    }
}

void CResizeScriptArrayCommand::UpdateArraySubProperty()
{
    CArrayProperty *pOldArray = mpArray;
    CBasicPropertyCommand::UpdateArraySubProperty();
    mpArray = static_cast<CArrayProperty*>(mpProperty);

    if (pOldArray != mpArray)
    {
        for (int iDel = 0; iDel < mDeletedProperties.size(); iDel++)
            mDeletedProperties[iDel]->SetParent(mpArray);
    }
}
