#include "CBasicPropertyCommand.h"
#include <Core/Resource/Script/IPropertyTemplate.h>

CBasicPropertyCommand::CBasicPropertyCommand(IProperty *pProp, CWorldEditor *pEditor, const QString& rkCommandName /*="Edit Property"*/)
    : IUndoCommand(rkCommandName)
    , mpProperty(pProp)
    , mpTemplate(mpProperty->Template())
    , mpBaseStruct(pProp->RootStruct())
    , mpEditor(pEditor)
    , mIsInArray(false)
{
    // Check for array
    IProperty *pChild = mpProperty;
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
                if (pArray->PropertyByIndex(iSub) == pChild)
                {
                    mArrayIndices << iSub;
                    break;
                }
            }
        }
        pChild = pParent;
        pParent = pParent->Parent();
    }
}

void CBasicPropertyCommand::UpdateArraySubProperty()
{
    // If an array has been sized down and then back up, then we might have a pointer to an invalid property.
    // Since we can't assume our pointer is still valid, we'll use the template to find the corresponding property.
    IPropertyTemplate *pTemp = mpTemplate;
    CStructTemplate *pParent = mpTemplate->Parent();

    QVector<u32> SubIndices;
    int IndexIndex = 0;

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
        else
            SubIndices << 0;

        if (pParent->Type() == eArrayProperty)
        {
            SubIndices << mArrayIndices[IndexIndex];
            IndexIndex++;
        }

        pTemp = pParent;
        pParent = pParent->Parent();
    }

    // Find corresponding property
    mpProperty = mpBaseStruct;

    for (int iChild = SubIndices.size() - 1; iChild >= 0; iChild--)
        mpProperty = static_cast<CPropertyStruct*>(mpProperty)->PropertyByIndex(SubIndices[iChild]);
}
