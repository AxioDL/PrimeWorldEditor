#include "CBasicPropertyCommand.h"
#include <Core/Resource/Script/IPropertyTemplate.h>

CBasicPropertyCommand::CBasicPropertyCommand(CPropertyModel *pModel, const QModelIndex& rkIndex)
    : IUndoCommand("Edit Property")
    , mpModel(pModel)
    , mpProperty(pModel->PropertyForIndex(rkIndex, true))
    , mpTemplate(mpProperty->Template())
    , mIndex(rkIndex)
    , mIsInArray(false)
{
    // Check for array
    IProperty *pProp = mpProperty;
    IProperty *pParent = mpProperty->Parent();

    while (pParent)
    {
        if (pParent->Type() == eArrayProperty)
        {
            mIsInArray = true;

            // Find array index
            CArrayProperty *pArray = static_cast<CArrayProperty*>(pParent);

            for (u32 iSub = 0; iSub < pArray->Count(); iSub++)
            {
                if (pArray->PropertyByIndex(iSub) == pProp)
                {
                    mArrayIndices << iSub;
                    break;
                }
            }
        }
        pProp = pParent;
        pParent = pParent->Parent();
    }
}

void CBasicPropertyCommand::UpdateArraySubProperty()
{
    // If an array has been sized down and then back up, then we might have an index to an invalid property.
    // Since we can't assume our index is still valid, we'll use the template and the model to find the corresponding property.
    IPropertyTemplate *pTemp = mpTemplate;
    CStructTemplate *pParent = mpTemplate->Parent();

    QVector<u32> SubIndices;
    int IndexIndex = 0;

    if (mIndex.internalId() & 0x1)
        SubIndices << mIndex.row();

    while (pParent)
    {
        if (pParent->Type() != eArrayProperty || static_cast<CArrayTemplate*>(pParent)->Count() > 1)
        {
            for (u32 iSub = 0; iSub < pParent->Count(); iSub++)
            {
                if (pParent->PropertyByIndex(iSub) == pTemp)
                {
                    SubIndices << iSub;
                    break;
                }
            }
        }

        if (pParent->Type() == eArrayProperty)
        {
            SubIndices << mArrayIndices[IndexIndex];
            IndexIndex++;
        }

        pTemp = pParent;
        pParent = pParent->Parent();
    }

    // Find corresponding index
    QModelIndex Index = QModelIndex();

    for (int iSub = SubIndices.size() - 1; iSub >= 0; iSub--)
        Index = mpModel->index(SubIndices[iSub], 0, Index);

    Index = Index.sibling(Index.row(), 1);

    // Get property
    mpProperty = mpModel->PropertyForIndex(Index, true);
    mIndex = Index;
}
