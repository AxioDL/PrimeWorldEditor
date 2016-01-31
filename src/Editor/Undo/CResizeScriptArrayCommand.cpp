#include "CResizeScriptArrayCommand.h"

CResizeScriptArrayCommand::CResizeScriptArrayCommand(CPropertyModel *pModel, const QModelIndex& rkIndex, u32 NewSize)
    : CBasicPropertyCommand(pModel, rkIndex)
    , mpArray(static_cast<CArrayProperty*>(mpProperty))
    , mOldSize(mpArray->Count())
    , mNewSize(NewSize)
{
    mNewSizeLarger = mNewSize > mOldSize;

    if (!mNewSizeLarger)
    {
        for (u32 iSub = mNewSize; iSub < mOldSize; iSub++)
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

        mpModel->ArrayAboutToBeResized(mIndex, mOldSize);
        mpArray->Resize(mOldSize);

        if (!mNewSizeLarger)
        {
            u32 NumNewElements = mOldSize - mNewSize;

            for (u32 iSub = 0; iSub < NumNewElements; iSub++)
            {
                u32 Idx = iSub + mNewSize;
                mpArray->PropertyByIndex(Idx)->Copy(mDeletedProperties[iSub]);
            }
        }

        mpModel->ArrayResized(mIndex, mNewSize);
    }
}

void CResizeScriptArrayCommand::redo()
{
    // Whether we're increasing or decreasing in size, there's no need to restore deleted properties on redo.
    if (mNewSize != mOldSize)
    {
        if (mIsInArray) UpdateArraySubProperty();

        mpModel->ArrayAboutToBeResized(mIndex, mNewSize);
        mpArray->Resize(mNewSize);
        mpModel->ArrayResized(mIndex, mOldSize);
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
