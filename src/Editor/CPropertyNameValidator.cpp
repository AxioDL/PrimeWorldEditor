#include "CPropertyNameValidator.h"
#include "UICommon.h"
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
void CPropertyNameValidator::SetTypeNameOverride(const QString& kNewTypeName)
{
    mTypeNameOverride = kNewTypeName;
    emit changed();
}

/** Perform validation */
QValidator::State CPropertyNameValidator::validate(QString& rInput, int&) const
{
    if (mpProperty)
    {
        TString TypeName = (mTypeNameOverride.isEmpty() ? mpProperty->HashableTypeName() : TO_TSTRING(mTypeNameOverride));

        CCRC32 Hash;
        Hash.Hash( rInput.toStdString().c_str() );
        Hash.Hash( *TypeName );
        uint32 PropertyID = Hash.Digest();

        if (PropertyID != mpProperty->ID())
        {
            if (mpProperty->Type() == EPropertyType::Int)
            {
                CCRC32 Hash2;
                Hash2.Hash( rInput.toStdString().c_str() );
                Hash2.Hash( "choice" );
                PropertyID = Hash2.Digest();
            }
        }
        return ( PropertyID == mpProperty->ID() ? QValidator::Acceptable : QValidator::Invalid );
    }

    return QValidator::Invalid;
}
