#ifndef TENUMCOMBOBOX_H
#define TENUMCOMBOBOX_H

#include <QComboBox>
#include <codegen/EnumReflection.h>

/**
 * Combo box subclass that auto-fills with an enum
 * No custom signals because Q_OBJECT macro doesn't support templates
 */
template<typename EnumT>
class TEnumComboBox : public QComboBox
{
    /** Vector forming an index -> enum mapping */
    QVector<EnumT> mValueList;

public:
    /** Constructor */
    explicit TEnumComboBox(QWidget* pParent = nullptr)
        : QComboBox(pParent)
    {
        for (typename TEnumReflection<EnumT>::CIterator It; It; ++It)
        {
            if (It.Value() != TEnumReflection<EnumT>::ErrorValue())
            {
                addItem(It.Name());
                mValueList.push_back(It.Value());
            }
        }
    }

    EnumT currentEnum() const
    {
        return mValueList[ currentIndex() ];
    }
};

#endif // TENUMCOMBOBOX_H
