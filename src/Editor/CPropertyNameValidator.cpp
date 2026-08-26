#include "Editor/CPropertyNameValidator.h"
#include "Editor/UICommon.h"

#include <Common/Hash/CCRC32.h>

CPropertyNameValidator::CPropertyNameValidator(QObject* pParent)
    : QValidator(pParent)
{}

/** Set the property to validate against */
void CPropertyNameValidator::SetProperty(IProperty* pProp)
{
    mpProperty = pProp;
    emit changed();
}

/** Set the type name override */
void CPropertyNameValidator::SetTypeNameOverride(QString kNewTypeName)
{
    mTypeNameOverride = std::move(kNewTypeName);
    emit changed();
}

/** Perform validation */
QValidator::State CPropertyNameValidator::validate(QString& rInput, int&) const
{
    if (!mpProperty)
        return QValidator::Invalid;

    const TString TypeName = mTypeNameOverride.isEmpty() ? mpProperty->HashableTypeName() : TO_TSTRING(mTypeNameOverride);
    const auto InputString = rInput.toStdString();

    CCRC32 Hash;
    Hash.Hash(InputString);
    Hash.Hash(TypeName);

    uint32_t PropertyID = Hash.Digest();

    if (PropertyID != mpProperty->ID())
    {
        if (mpProperty->Type() == EPropertyType::Int)
        {
            CCRC32 Hash2;
            Hash2.Hash(InputString);
            Hash2.Hash("choice");
            PropertyID = Hash2.Digest();
        }
    }

    return PropertyID == mpProperty->ID() ? QValidator::Acceptable : QValidator::Invalid;
}
