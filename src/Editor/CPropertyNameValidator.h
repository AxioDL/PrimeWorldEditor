#ifndef CPROPERTYNAMEVALIDATOR_H
#define CPROPERTYNAMEVALIDATOR_H

#include <QValidator>
#include <Core/Resource/Script/Property/Properties.h>

/** QValidator subclass that checks if a property name is valid */
class CPropertyNameValidator : public QValidator
{
    Q_OBJECT

    /** The property being validated against */
    IPropertyNew* mpProperty;

public:
    CPropertyNameValidator(QObject* pParent = 0);

    /** Set the property to validate against */
    void SetProperty(IPropertyNew* pProp);

    /** Perform validation */
    QValidator::State validate(QString& rInput, int& rPos) const;
};

#endif // CPROPERTYNAMEVALIDATOR_H
