#include "CPropertyNameValidator.h"
#include <Common/Hash/CCRC32.h>

CPropertyNameValidator::CPropertyNameValidator(QObject* pParent)
    : QValidator(pParent)
{}

/** Set the property to validate against */
void CPropertyNameValidator::SetProperty(IPropertyTemplate* pProp)
{
    mpProperty = pProp;
    emit changed();
}

/** Perform validation */
QValidator::State CPropertyNameValidator::validate(QString& rInput, int&) const
{
    if (mpProperty)
    {
        CCRC32 Hash;
        Hash.Hash( rInput.toStdString().c_str() );
        Hash.Hash( mpProperty->GetTypeNameString() );
        u32 PropertyID = Hash.Digest();

        return ( PropertyID == mpProperty->PropertyID() ? QValidator::Acceptable : QValidator::Invalid );
    }

    return QValidator::Invalid;
}
