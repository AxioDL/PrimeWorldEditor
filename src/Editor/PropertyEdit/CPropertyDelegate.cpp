#include "CPropertyDelegate.h"
#include "CPropertyRelay.h"

#include "Editor/UICommon.h"
#include "Editor/Undo/CEditScriptPropertyCommand.h"
#include "Editor/Undo/CResizeScriptArrayCommand.h"
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
    connect(pRelay, SIGNAL(WidgetEdited(QWidget*, const QModelIndex&)), this, SLOT(WidgetEdited(QWidget*, const QModelIndex&)))

CPropertyDelegate::CPropertyDelegate(QObject *pParent /*= 0*/)
    : QStyledItemDelegate(pParent)
    , mpEditor(nullptr)
    , mpModel(nullptr)
    , mInRelayWidgetEdit(false)
    , mEditInProgress(false)
    , mRelaysBlocked(false)
{
}

void CPropertyDelegate::SetPropertyModel(CPropertyModel *pModel)
{
    mpModel = pModel;
}

void CPropertyDelegate::SetEditor(CWorldEditor *pEditor)
{
    mpEditor = pEditor;
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
        {
            QCheckBox *pCheckBox = new QCheckBox(pParent);
            CONNECT_RELAY(pCheckBox, rkIndex, toggled(bool));
            pOut = pCheckBox;
            break;
        }

        case eShortProperty:
        {
            WIntegralSpinBox *pSpinBox = new WIntegralSpinBox(pParent);
            pSpinBox->setMinimum(INT16_MIN);
            pSpinBox->setMaximum(INT16_MAX);
            CONNECT_RELAY(pSpinBox, rkIndex, valueChanged(int));
            pOut = pSpinBox;
            break;
        }

        case eLongProperty:
        {
            WIntegralSpinBox *pSpinBox = new WIntegralSpinBox(pParent);
            pSpinBox->setMinimum(INT32_MIN);
            pSpinBox->setMaximum(INT32_MAX);
            CONNECT_RELAY(pSpinBox, rkIndex, valueChanged(int));
            pOut = pSpinBox;
            break;
        }

        case eFloatProperty:
        {
            WDraggableSpinBox *pSpinBox = new WDraggableSpinBox(pParent);
            pSpinBox->setSingleStep(0.1);
            CONNECT_RELAY(pSpinBox, rkIndex, valueChanged(double));
            pOut = pSpinBox;
            break;
        }

        case eColorProperty:
        {
            WColorPicker *pColorPicker = new WColorPicker(pParent);
            CONNECT_RELAY(pColorPicker, rkIndex, colorChanged(QColor));
            pOut = pColorPicker;
            break;
        }

        case eStringProperty:
        {
            QLineEdit *pLineEdit = new QLineEdit(pParent);
            CONNECT_RELAY(pLineEdit, rkIndex, textEdited(QString));
            pOut = pLineEdit;
            break;
        }

        case eEnumProperty:
        {
            QComboBox *pComboBox = new QComboBox(pParent);

            CEnumTemplate *pTemp = static_cast<CEnumTemplate*>(pProp->Template());

            for (u32 iEnum = 0; iEnum < pTemp->NumEnumerators(); iEnum++)
                pComboBox->addItem(TO_QSTRING(pTemp->EnumeratorName(iEnum)));

            CONNECT_RELAY(pComboBox, rkIndex, currentIndexChanged(int));
            pOut = pComboBox;
            break;
        }

        case eFileProperty:
        {
            WResourceSelector *pSelector = new WResourceSelector(pParent);
            CFileTemplate *pTemp = static_cast<CFileTemplate*>(pProp->Template());
            pSelector->SetAllowedExtensions(pTemp->Extensions());
            pSelector->setFont(qobject_cast<QWidget*>(parent())->font()); // bit of a hack to stop the resource selector font from changing

            CONNECT_RELAY(pSelector, rkIndex, ResourceChanged(QString));
            pOut = pSelector;
            break;
        }

        case eArrayProperty:
        {
            // No relay here, would prefer user to be sure of their change before it's reflected on the UI
            WIntegralSpinBox *pSpinBox = new WIntegralSpinBox(pParent);
            pSpinBox->setMinimum(0);
            pSpinBox->setMaximum(999);
            pOut = pSpinBox;
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
        {
            QCheckBox *pCheckBox = new QCheckBox(pParent);
            CONNECT_RELAY(pCheckBox, rkIndex, toggled(bool));
            pOut = pCheckBox;
        }

        // Handle vector/color
        else
        {
            WDraggableSpinBox *pSpinBox = new WDraggableSpinBox(pParent);
            pSpinBox->setSingleStep(0.1);

            // Limit to range of 0-1 on colors
            pProp = mpModel->PropertyForIndex(rkIndex, true);

            if (pProp->Type() == eColorProperty)
            {
                pSpinBox->setMinimum(0.0);
                pSpinBox->setMaximum(1.0);
            }

            CONNECT_RELAY(pSpinBox, rkIndex, valueChanged(double));
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
    BlockRelays(true);
    mEditInProgress = false; // fixes case where user does undo mid-edit

    if (pEditor)
    {
        // Set editor data for regular property
        IProperty *pProp = mpModel->PropertyForIndex(rkIndex, false);

        if (pProp)
        {
            if (!mEditInProgress)
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

                case eArrayProperty:
                {
                    WIntegralSpinBox *pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);
                    CArrayProperty *pArray = static_cast<CArrayProperty*>(pProp);
                    pSpinBox->setValue(pArray->Count());
                    break;
                }

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

    BlockRelays(false);
}

void CPropertyDelegate::setModelData(QWidget *pEditor, QAbstractItemModel* /*pModel*/, const QModelIndex &rkIndex) const
{
    if (!mpModel) return;
    if (!pEditor) return;

    IProperty *pProp = mpModel->PropertyForIndex(rkIndex, false);
    IPropertyValue *pOldValue = nullptr;

    if (pProp)
    {
        IPropertyValue *pRawValue = pProp->RawValue();
        pOldValue = pRawValue ? pRawValue->Clone() : nullptr;

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

        case eFileProperty:
        {
            WResourceSelector *pSelector = static_cast<WResourceSelector*>(pEditor);
            TFileProperty *pFile = static_cast<TFileProperty*>(pProp);
            pFile->Set(pSelector->GetResourceInfo());
            break;
        }

        case eArrayProperty:
        {
            WIntegralSpinBox *pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);
            CArrayProperty *pArray = static_cast<CArrayProperty*>(pProp);
            int NewCount = pSpinBox->value();

            if (pArray->Count() != NewCount)
            {
                CResizeScriptArrayCommand *pCmd = new CResizeScriptArrayCommand(pProp, mpEditor, mpModel, NewCount);
                mpEditor->UndoStack()->push(pCmd);
            }
            break;
        }

        }
    }

    // Check for character/bitfield/vector/color sub-properties
    else if (rkIndex.internalId() & 0x1)
    {
        pProp = mpModel->PropertyForIndex(rkIndex, true);

        IPropertyValue *pRawValue = pProp->RawValue();
        pOldValue = pRawValue ? pRawValue->Clone() : nullptr;

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
        }
    }

    if (pProp && pOldValue)
    {
        // Check for edit in progress
        bool Matches = pOldValue->Matches(pProp->RawValue());

        if (!Matches && mInRelayWidgetEdit && pEditor->hasFocus())
            mEditInProgress = true;

        bool EditInProgress = mEditInProgress;

        // Check for edit finished
        if (!mInRelayWidgetEdit || !pEditor->hasFocus())
            mEditInProgress = false;

        // Create undo command
        if (!Matches || EditInProgress)
        {
            // Always consider the edit done for bool properties
            CEditScriptPropertyCommand *pCommand = new CEditScriptPropertyCommand(pProp, mpEditor, pOldValue, (!mEditInProgress || pProp->Type() == eBoolProperty));
            mpEditor->UndoStack()->push(pCommand);
        }

        else
            delete pOldValue;
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
        pSelector->setFont(qobject_cast<QWidget*>(parent())->font()); // hack to keep the selector font from changing

        if (Params.Version() <= eEchoes)
            pSelector->SetAllowedExtensions("ANCS");
        else
            pSelector->SetAllowedExtensions("CHAR");

        CONNECT_RELAY(pSelector, rkIndex, ResourceChanged(QString));
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

        CONNECT_RELAY(pComboBox, rkIndex, currentIndexChanged(int));
        return pComboBox;
    }

    if (Type == eLongProperty)
    {
        WIntegralSpinBox *pSpinBox = new WIntegralSpinBox(pParent);
        CONNECT_RELAY(pSpinBox, rkIndex, valueChanged(int));
        return pSpinBox;
    }

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
        // Reset all other parameters to 0
        Params.SetNodeIndex(0);
        for (u32 iUnk = 0; iUnk < 4; iUnk++)
            Params.SetUnknown(iUnk, 0);
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

    // If we just updated the resource, make sure all the sub-properties of the character are flagged as changed.
    // We want to do this -after- updating the anim params on the property, which is why we have a second type check.
    if (Type == eFileProperty)
    {
        QModelIndex ParentIndex = rkIndex.parent();
        mpModel->dataChanged(mpModel->index(1, 1, ParentIndex), mpModel->index(mpModel->rowCount(ParentIndex) - 1, 1, ParentIndex));
    }
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
    mInRelayWidgetEdit = true;

    if (!mRelaysBlocked)
        setModelData(pWidget, mpModel, rkIndex);

    mInRelayWidgetEdit = false;
}
