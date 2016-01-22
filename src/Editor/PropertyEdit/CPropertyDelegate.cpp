#include "CPropertyDelegate.h"
#include "CPropertyRelay.h"

#include "Editor/UICommon.h"
#include "Editor/Widgets/WColorPicker.h"
#include "Editor/Widgets/WDraggableSpinBox.h"
#include "Editor/Widgets/WIntegralSpinBox.h"
#include "Editor/Widgets/WResourceSelector.h"

#include <Core/Resource/Script/IProperty.h>
#include <Core/Resource/Script/IPropertyTemplate.h>

#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QLineEdit>

// This macro should be used on every widget where changes should be reflected in realtime and not just when the edit is finished.
#define CONNECT_RELAY(Widget, Index, Signal) \
    CPropertyRelay *pRelay = new CPropertyRelay(Widget, Index); \
    connect(Widget, SIGNAL(Signal), pRelay, SLOT(OnWidgetEdited())); \
    connect(pRelay, SIGNAL(WidgetEdited(QWidget*, const QModelIndex&)), this, SLOT(WidgetEdited(QWidget*, const QModelIndex&)));

CPropertyDelegate::CPropertyDelegate(QObject *pParent /*= 0*/)
    : QStyledItemDelegate(pParent)
    , mpModel(nullptr)
{
}

void CPropertyDelegate::SetModel(CPropertyModel *pModel)
{
    mpModel = pModel;
}

QWidget* CPropertyDelegate::createEditor(QWidget *pParent, const QStyleOptionViewItem& /*rkOption*/, const QModelIndex& rkIndex) const
{
    if (!mpModel) return nullptr;
    IProperty *pProp = mpModel->PropertyForIndex(rkIndex, false);
    QWidget *pOut = nullptr;

    if (pProp)
    {
        switch (pProp->Type())
        {
        case eBoolProperty:
            pOut = new QCheckBox(pParent);
            break;

        case eShortProperty:
        case eLongProperty:
            pOut = new WIntegralSpinBox(pParent);
            break;

        case eFloatProperty:
        {
            WDraggableSpinBox *pSpinBox = new WDraggableSpinBox(pParent);
            CONNECT_RELAY(pSpinBox, rkIndex, valueChanged(double))
            pSpinBox->setSingleStep(0.1);
            pOut = pSpinBox;
            break;
        }

        case eColorProperty:
        {
            WColorPicker *pColorPicker = new WColorPicker(pParent);
            CONNECT_RELAY(pColorPicker, rkIndex, colorChanged(QColor))
            pOut = pColorPicker;
            break;
        }

        case eStringProperty:
            pOut = new QLineEdit(pParent);
            break;

        case eEnumProperty:
        {
            QComboBox *pComboBox = new QComboBox(pParent);
            CEnumTemplate *pTemp = static_cast<CEnumTemplate*>(pProp->Template());

            for (u32 iEnum = 0; iEnum < pTemp->NumEnumerators(); iEnum++)
                pComboBox->addItem(TO_QSTRING(pTemp->EnumeratorName(iEnum)));

            pOut = pComboBox;
            break;
        }

        case eFileProperty:
        {
            WResourceSelector *pSelector = new WResourceSelector(pParent);
            CFileTemplate *pTemp = static_cast<CFileTemplate*>(pProp->Template());
            pSelector->SetAllowedExtensions(pTemp->Extensions());
            pOut = pSelector;
            break;
        }

        }
    }

    // Check for sub-property of vector/color/character
    else if (rkIndex.internalId() & 0x1)
    {
        pProp = mpModel->PropertyForIndex(rkIndex, true);

        // Handle character
        if (pProp->Type() == eCharacterProperty)
            pOut = CreateCharacterEditor(pParent, rkIndex);

        // Handle bitfield
        else if (pProp->Type() == eBitfieldProperty)
            pOut = new QCheckBox(pParent);

        // Handle vector/color
        else
        {
            WDraggableSpinBox *pSpinBox = new WDraggableSpinBox(pParent);
            CONNECT_RELAY(pSpinBox, rkIndex, valueChanged(double))
            pSpinBox->setSingleStep(0.1);

            // Limit to range of 0-1 on colors
            pProp = mpModel->PropertyForIndex(rkIndex, true);

            if (pProp->Type() == eColorProperty)
            {
                pSpinBox->setMinimum(0.0);
                pSpinBox->setMaximum(1.0);
            }

            pOut = pSpinBox;
        }
    }

    if (pOut)
    {
        pOut->setFocusPolicy(Qt::StrongFocus);
    }

    return pOut;
}

void CPropertyDelegate::setEditorData(QWidget *pEditor, const QModelIndex &rkIndex) const
{
    if (pEditor)
    {
        // Set editor data for regular property
        IProperty *pProp = mpModel->PropertyForIndex(rkIndex, false);

        if (pProp)
        {
            switch (pProp->Type())
            {

            case eBoolProperty:
            {
                QCheckBox *pCheckBox = static_cast<QCheckBox*>(pEditor);
                TBoolProperty *pBool = static_cast<TBoolProperty*>(pProp);
                pCheckBox->setChecked(pBool->Get());
                break;
            }

            case eShortProperty:
            {
                WIntegralSpinBox *pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);
                TShortProperty *pShort = static_cast<TShortProperty*>(pProp);
                pSpinBox->setValue(pShort->Get());
                break;
            }


            case eLongProperty:
            {
                WIntegralSpinBox *pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);
                TLongProperty *pLong = static_cast<TLongProperty*>(pProp);
                pSpinBox->setValue(pLong->Get());
                break;
            }

            case eFloatProperty:
            {
                WDraggableSpinBox *pSpinBox = static_cast<WDraggableSpinBox*>(pEditor);
                TFloatProperty *pFloat = static_cast<TFloatProperty*>(pProp);
                pSpinBox->setValue(pFloat->Get());
                break;
            }

            case eColorProperty:
            {
                WColorPicker *pColorPicker = static_cast<WColorPicker*>(pEditor);
                TColorProperty *pColor = static_cast<TColorProperty*>(pProp);

                CColor SrcColor = pColor->Get();
                QColor Color;
                Color.setRed(SrcColor.r * 255);
                Color.setGreen(SrcColor.g * 255);
                Color.setBlue(SrcColor.b * 255);
                Color.setAlpha(SrcColor.a * 255);

                pColorPicker->setColor(Color);
                break;
            }

            case eStringProperty:
            {
                QLineEdit *pLineEdit = static_cast<QLineEdit*>(pEditor);
                TStringProperty *pString = static_cast<TStringProperty*>(pProp);
                pLineEdit->setText(TO_QSTRING(pString->Get()));
                break;
            }

            case eEnumProperty:
            {
                QComboBox *pComboBox = static_cast<QComboBox*>(pEditor);
                TEnumProperty *pEnum = static_cast<TEnumProperty*>(pProp);
                pComboBox->setCurrentIndex(pEnum->Get());
                break;
            }

            case eFileProperty:
            {
                WResourceSelector *pSelector = static_cast<WResourceSelector*>(pEditor);
                TFileProperty *pFile = static_cast<TFileProperty*>(pProp);
                pSelector->SetResource(pFile->Get());
                break;
            }

            }
        }

        // Set editor data for character/bitfield/vector/color sub-property
        else if (rkIndex.internalId() & 0x1)
        {
            pProp = mpModel->PropertyForIndex(rkIndex, true);

            if (pProp->Type() == eCharacterProperty)
                SetCharacterEditorData(pEditor, rkIndex);

            else if (pProp->Type() == eBitfieldProperty)
            {
                QCheckBox *pCheckBox = static_cast<QCheckBox*>(pEditor);
                TBitfieldProperty *pBitfield = static_cast<TBitfieldProperty*>(pProp);
                u32 Mask = static_cast<CBitfieldTemplate*>(pBitfield->Template())->FlagMask(rkIndex.row());
                bool Set = (pBitfield->Get() & Mask) != 0;
                pCheckBox->setChecked(Set);
            }

            else
            {
                WDraggableSpinBox *pSpinBox = static_cast<WDraggableSpinBox*>(pEditor);
                float Value;

                if (pProp->Type() == eVector3Property)
                {
                    TVector3Property *pVector = static_cast<TVector3Property*>(pProp);
                    CVector3f Vector = pVector->Get();

                    if (rkIndex.row() == 0) Value = Vector.x;
                    if (rkIndex.row() == 1) Value = Vector.y;
                    if (rkIndex.row() == 2) Value = Vector.z;
                }

                else if (pProp->Type() == eColorProperty)
                {
                    TColorProperty *pColor = static_cast<TColorProperty*>(pProp);
                    CColor Color = pColor->Get();

                    if (rkIndex.row() == 0) Value = Color.r;
                    if (rkIndex.row() == 1) Value = Color.g;
                    if (rkIndex.row() == 2) Value = Color.b;
                    if (rkIndex.row() == 3) Value = Color.a;
                }

                pSpinBox->setValue((double) Value);
            }
        }
    }
}

void CPropertyDelegate::setModelData(QWidget *pEditor, QAbstractItemModel* /*pModel*/, const QModelIndex &rkIndex) const
{
    if (!mpModel) return;
    if (!pEditor) return;

    IProperty *pProp = mpModel->PropertyForIndex(rkIndex, false);

    if (pProp)
    {
        switch (pProp->Type())
        {

        case eBoolProperty:
        {
            QCheckBox *pCheckBox = static_cast<QCheckBox*>(pEditor);
            TBoolProperty *pBool = static_cast<TBoolProperty*>(pProp);
            pBool->Set(pCheckBox->isChecked());
            break;
        }

        case eShortProperty:
        {
            WIntegralSpinBox *pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);
            TShortProperty *pShort = static_cast<TShortProperty*>(pProp);
            pShort->Set(pSpinBox->value());
            break;
        }

        case eLongProperty:
        {
            WIntegralSpinBox *pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);
            TLongProperty *pLong = static_cast<TLongProperty*>(pProp);
            pLong->Set(pSpinBox->value());
            break;
        }

        case eFloatProperty:
        {
            WDraggableSpinBox *pSpinBox = static_cast<WDraggableSpinBox*>(pEditor);
            TFloatProperty *pFloat = static_cast<TFloatProperty*>(pProp);
            pFloat->Set((float) pSpinBox->value());
            break;
        }

        case eColorProperty:
        {
            WColorPicker *pColorPicker = static_cast<WColorPicker*>(pEditor);
            TColorProperty *pColor = static_cast<TColorProperty*>(pProp);

            QColor SrcColor = pColorPicker->getColor();
            CColor Color;
            Color.r = SrcColor.red() / 255.f;
            Color.g = SrcColor.green() / 255.f;
            Color.b = SrcColor.blue() / 255.f;
            Color.a = SrcColor.alpha() / 255.f;
            pColor->Set(Color);

            // Make sure sub-properties update with the new color
            mpModel->UpdateSubProperties(rkIndex);
            break;
        }

        case eStringProperty:
        {
            QLineEdit *pLineEdit = static_cast<QLineEdit*>(pEditor);
            TStringProperty *pString = static_cast<TStringProperty*>(pProp);
            pString->Set(TO_TSTRING(pLineEdit->text()));
            break;
        }

        case eEnumProperty:
        {
            QComboBox *pComboBox = static_cast<QComboBox*>(pEditor);
            TEnumProperty *pEnum = static_cast<TEnumProperty*>(pProp);
            pEnum->Set(pComboBox->currentIndex());
            break;
        }

        }
    }

    // Check for character/bitfield/vector/color sub-properties
    else if (rkIndex.internalId() & 0x1)
    {
        pProp = mpModel->PropertyForIndex(rkIndex, true);

        if (pProp->Type() == eCharacterProperty)
            SetCharacterModelData(pEditor, rkIndex);

        else if (pProp->Type() == eBitfieldProperty)
        {
            QCheckBox *pCheckBox = static_cast<QCheckBox*>(pEditor);
            TBitfieldProperty *pBitfield = static_cast<TBitfieldProperty*>(pProp);
            u32 Mask = static_cast<CBitfieldTemplate*>(pProp->Template())->FlagMask(rkIndex.row());

            int Flags = pBitfield->Get();
            if (pCheckBox->isChecked()) Flags |= Mask;
            else Flags &= ~Mask;
            pBitfield->Set(Flags);
        }

        else
        {
            WDraggableSpinBox *pSpinBox = static_cast<WDraggableSpinBox*>(pEditor);

            if (pProp->Type() == eVector3Property)
            {
                TVector3Property *pVector = static_cast<TVector3Property*>(pProp);
                CVector3f Value = pVector->Get();

                if (rkIndex.row() == 0) Value.x = (float) pSpinBox->value();
                if (rkIndex.row() == 1) Value.y = (float) pSpinBox->value();
                if (rkIndex.row() == 2) Value.z = (float) pSpinBox->value();

                pVector->Set(Value);
            }

            else if (pProp->Type() == eColorProperty)
            {
                TColorProperty *pColor = static_cast<TColorProperty*>(pProp);
                CColor Value = pColor->Get();

                if (rkIndex.row() == 0) Value.r = (float) pSpinBox->value();
                if (rkIndex.row() == 1) Value.g = (float) pSpinBox->value();
                if (rkIndex.row() == 2) Value.b = (float) pSpinBox->value();
                if (rkIndex.row() == 3) Value.a = (float) pSpinBox->value();

                pColor->Set(Value);
            }

            mpModel->dataChanged(rkIndex.parent(), rkIndex.parent());
        }
    }
}

bool CPropertyDelegate::eventFilter(QObject *pObject, QEvent *pEvent)
{
    if (pEvent->type() == QEvent::Wheel)
    {
        QWidget *pWidget = static_cast<QWidget*>(pObject);

        if (!pWidget->hasFocus())
            return true;

        pEvent->ignore();
        return false;
    }

    return QStyledItemDelegate::eventFilter(pObject, pEvent);
}

// Character properties have separate functions because they're somewhat complicated - they have different layouts in different games
QWidget* CPropertyDelegate::CreateCharacterEditor(QWidget *pParent, const QModelIndex& rkIndex) const
{
    TCharacterProperty *pProp = static_cast<TCharacterProperty*>(mpModel->PropertyForIndex(rkIndex, true));
    CAnimationParameters Params = pProp->Get();

    // Determine property type
    EPropertyType Type = DetermineCharacterPropType(Params.Version(), rkIndex);
    if (Type == eUnknownProperty) return nullptr;

    // Create widget
    if (Type == eFileProperty)
    {
        WResourceSelector *pSelector = new WResourceSelector(pParent);

        if (Params.Version() <= eEchoes)
            pSelector->SetAllowedExtensions("ANCS");
        else
            pSelector->SetAllowedExtensions("CHAR");

        return pSelector;
    }

    if (Type == eEnumProperty)
    {
        QComboBox *pComboBox = new QComboBox(pParent);
        CAnimSet *pAnimSet = Params.AnimSet();

        if (pAnimSet)
        {
            for (u32 iChr = 0; iChr < pAnimSet->getNodeCount(); iChr++)
                pComboBox->addItem(TO_QSTRING(pAnimSet->getNodeName(iChr)));
        }

        return pComboBox;
    }

    if (Type == eLongProperty)
        return new WIntegralSpinBox(pParent);

    return nullptr;
}

void CPropertyDelegate::SetCharacterEditorData(QWidget *pEditor, const QModelIndex& rkIndex) const
{
    TCharacterProperty *pProp = static_cast<TCharacterProperty*>(mpModel->PropertyForIndex(rkIndex, true));
    CAnimationParameters Params = pProp->Get();
    EPropertyType Type = DetermineCharacterPropType(Params.Version(), rkIndex);

    if (Type == eFileProperty)
    {
        static_cast<WResourceSelector*>(pEditor)->SetResource(Params.AnimSet());
    }

    else if (Type == eEnumProperty)
    {
        static_cast<QComboBox*>(pEditor)->setCurrentIndex(Params.CharacterIndex());
    }

    else if (Type == eLongProperty)
    {
        int UnkIndex = (Params.Version() <= eEchoes ? rkIndex.row() - 2 : rkIndex.row() - 1);
        u32 Value = Params.Unknown(UnkIndex);
        static_cast<WIntegralSpinBox*>(pEditor)->setValue(Value);
    }
}

void CPropertyDelegate::SetCharacterModelData(QWidget *pEditor, const QModelIndex& rkIndex) const
{
    TCharacterProperty *pProp = static_cast<TCharacterProperty*>(mpModel->PropertyForIndex(rkIndex, true));
    CAnimationParameters Params = pProp->Get();
    EPropertyType Type = DetermineCharacterPropType(Params.Version(), rkIndex);

    if (Type == eFileProperty)
    {
        Params.SetResource( static_cast<WResourceSelector*>(pEditor)->GetResource() );
    }

    else if (Type == eEnumProperty)
    {
        Params.SetNodeIndex( static_cast<QComboBox*>(pEditor)->currentIndex() );
    }

    else if (Type == eLongProperty)
    {
        int UnkIndex = (Params.Version() <= eEchoes ? rkIndex.row() - 2 : rkIndex.row() - 1);
        Params.SetUnknown(UnkIndex, static_cast<WIntegralSpinBox*>(pEditor)->value() );
    }

    pProp->Set(Params);
}

EPropertyType CPropertyDelegate::DetermineCharacterPropType(EGame Game, const QModelIndex& rkIndex) const
{
    if (Game <= eEchoes)
    {
        if      (rkIndex.row() == 0) return eFileProperty;
        else if (rkIndex.row() == 1) return eEnumProperty;
        else if (rkIndex.row() == 2) return eLongProperty;
    }
    else if (Game <= eCorruption)
    {
        if      (rkIndex.row() == 0) return eFileProperty;
        else if (rkIndex.row() == 1) return eLongProperty;
    }
    else
    {
        if      (rkIndex.row() == 0) return eFileProperty;
        else if (rkIndex.row() <= 3) return eLongProperty;
    }
    return eUnknownProperty;
}

// ************ PUBLIC SLOTS ************
void CPropertyDelegate::WidgetEdited(QWidget *pWidget, const QModelIndex& rkIndex)
{
    // This slot is used to update property values as they're being updated so changes can be
    // reflected in realtime in other parts of the application.
    setModelData(pWidget, mpModel, rkIndex);
}
