#include "CPropertyDelegate.h"
#include "CPropertyRelay.h"

#include "Editor/UICommon.h"
#include "Editor/Undo/CEditScriptPropertyCommand.h"
#include "Editor/Undo/CResizeScriptArrayCommand.h"
#include "Editor/Widgets/CResourceSelector.h"
#include "Editor/Widgets/WColorPicker.h"
#include "Editor/Widgets/WDraggableSpinBox.h"
#include "Editor/Widgets/WIntegralSpinBox.h"

#include <Core/Resource/Animation/CAnimSet.h>
#include <Core/Resource/Script/IProperty.h>
#include <Core/Resource/Script/IPropertyTemplate.h>

#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QLineEdit>

// This macro should be used on every widget where changes should be reflected in realtime and not just when the edit is finished.
#define CONNECT_RELAY(Widget, Index, Signal) \
    { \
    CPropertyRelay *pRelay = new CPropertyRelay(Widget, Index); \
    connect(Widget, SIGNAL(Signal), pRelay, SLOT(OnWidgetEdited())); \
    connect(pRelay, SIGNAL(WidgetEdited(QWidget*, const QModelIndex&)), this, SLOT(WidgetEdited(QWidget*, const QModelIndex&))); \
    }

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
    IPropertyNew *pProp = mpModel->PropertyForIndex(rkIndex, false);
    QWidget *pOut = nullptr;

    if (pProp)
    {
        switch (pProp->Type())
        {

        case EPropertyTypeNew::Bool:
        {
            QCheckBox *pCheckBox = new QCheckBox(pParent);
            CONNECT_RELAY(pCheckBox, rkIndex, toggled(bool))
            pOut = pCheckBox;
            break;
        }

        case EPropertyTypeNew::Short:
        {
            WIntegralSpinBox *pSpinBox = new WIntegralSpinBox(pParent);
            pSpinBox->setMinimum(INT16_MIN);
            pSpinBox->setMaximum(INT16_MAX);
            pSpinBox->setSuffix(TO_QSTRING(pProp->Suffix()));
            CONNECT_RELAY(pSpinBox, rkIndex, valueChanged(int))
            pOut = pSpinBox;
            break;
        }

        case EPropertyTypeNew::Int:
        {
            WIntegralSpinBox *pSpinBox = new WIntegralSpinBox(pParent);
            pSpinBox->setMinimum(INT32_MIN);
            pSpinBox->setMaximum(INT32_MAX);
            pSpinBox->setSuffix(TO_QSTRING(pProp->Suffix()));
            CONNECT_RELAY(pSpinBox, rkIndex, valueChanged(int))
            pOut = pSpinBox;
            break;
        }

        case EPropertyTypeNew::Float:
        {
            WDraggableSpinBox *pSpinBox = new WDraggableSpinBox(pParent);
            pSpinBox->setSingleStep(0.1);
            pSpinBox->setSuffix(TO_QSTRING(pProp->Suffix()));
            CONNECT_RELAY(pSpinBox, rkIndex, valueChanged(double))
            pOut = pSpinBox;
            break;
        }

        case EPropertyTypeNew::Color:
        {
            WColorPicker *pColorPicker = new WColorPicker(pParent);
            CONNECT_RELAY(pColorPicker, rkIndex, ColorChanged(QColor))
            pOut = pColorPicker;
            break;
        }

        case EPropertyTypeNew::Sound:
        {
            WIntegralSpinBox *pSpinBox = new WIntegralSpinBox(pParent);
            pSpinBox->setMinimum(-1);
            pSpinBox->setMaximum(0xFFFF);
            CONNECT_RELAY(pSpinBox, rkIndex, valueChanged(int))
            pOut = pSpinBox;
            break;
        }

        case EPropertyTypeNew::String:
        {
            QLineEdit *pLineEdit = new QLineEdit(pParent);
            CONNECT_RELAY(pLineEdit, rkIndex, textEdited(QString))
            pOut = pLineEdit;
            break;
        }

        case EPropertyTypeNew::Enum:
        case EPropertyTypeNew::Choice:
        {
            QComboBox *pComboBox = new QComboBox(pParent);
            CEnumProperty* pEnum = TPropCast<CEnumProperty>(pProp);

            for (u32 ValueIdx = 0; ValueIdx < pEnum->NumPossibleValues(); ValueIdx++)
                pComboBox->addItem(TO_QSTRING(pEnum->ValueName(ValueIdx)));

            CONNECT_RELAY(pComboBox, rkIndex, currentIndexChanged(int))
            pOut = pComboBox;
            break;
        }

        case EPropertyTypeNew::Asset:
        {
            CResourceSelector *pSelector = new CResourceSelector(pParent);
            pSelector->SetFrameVisible(false);

            CAssetProperty *pAsset = TPropCast<CAssetProperty>(pProp);
            pSelector->SetTypeFilter(pAsset->GetTypeFilter());

            CONNECT_RELAY(pSelector, rkIndex, ResourceChanged(CResourceEntry*))
            pOut = pSelector;
            break;
        }

        case EPropertyTypeNew::Array:
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

    // Check for sub-property of flgs/animation set
    else if (rkIndex.internalId() & 0x80000000)
    {
        pProp = mpModel->PropertyForIndex(rkIndex, true);
        EPropertyTypeNew Type = pProp->Type();

        // Handle character
        if (Type == EPropertyTypeNew::AnimationSet)
            pOut = CreateCharacterEditor(pParent, rkIndex);

        // Handle flags
        else if (Type == EPropertyTypeNew::Flags)
        {
            QCheckBox *pCheckBox = new QCheckBox(pParent);
            CONNECT_RELAY(pCheckBox, rkIndex, toggled(bool))
            pOut = pCheckBox;
        }
    }

    if (pOut)
    {
        pOut->setFocusPolicy(Qt::StrongFocus);
        QSize Size = mpModel->data(rkIndex, Qt::SizeHintRole).toSize();
        pOut->setFixedHeight(Size.height());
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
        IPropertyNew *pProp = mpModel->PropertyForIndex(rkIndex, false);
        void* pData = mpModel->DataPointerForIndex(rkIndex);

        if (pProp)
        {
            if (!mEditInProgress)
            {
                switch (pProp->Type())
                {

                case EPropertyTypeNew::Bool:
                {
                    QCheckBox *pCheckBox = static_cast<QCheckBox*>(pEditor);
                    CBoolProperty *pBool = TPropCast<CBoolProperty>(pProp);
                    pCheckBox->setChecked( pBool->Value(pData) );
                    break;
                }

                case EPropertyTypeNew::Short:
                {
                    WIntegralSpinBox *pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);

                    if (!pSpinBox->hasFocus())
                    {
                        CShortProperty *pShort = TPropCast<CShortProperty>(pProp);
                        pSpinBox->setValue( pShort->Value(pData) );
                    }

                    break;
                }

                case EPropertyTypeNew::Int:
                {
                    WIntegralSpinBox *pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);

                    if (!pSpinBox->hasFocus())
                    {
                        CIntProperty *pInt = TPropCast<CIntProperty>(pProp);
                        pSpinBox->setValue( pInt->Value(pData) );
                    }

                    break;
                }

                case EPropertyTypeNew::Sound:
                {
                    WIntegralSpinBox *pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);

                    if (!pSpinBox->hasFocus())
                    {
                        CSoundProperty *pSound = TPropCast<CSoundProperty>(pProp);
                        pSpinBox->setValue( pSound->Value(pData) );
                    }

                    break;
                }

                case EPropertyTypeNew::Float:
                {
                    WDraggableSpinBox *pSpinBox = static_cast<WDraggableSpinBox*>(pEditor);

                    if (!pSpinBox->hasFocus())
                    {
                        CFloatProperty *pFloat = TPropCast<CFloatProperty>(pProp);
                        pSpinBox->setValue( pFloat->Value(pData) );
                    }

                    break;
                }

                case EPropertyTypeNew::Color:
                {
                    WColorPicker *pColorPicker = static_cast<WColorPicker*>(pEditor);
                    CColorProperty *pColor = TPropCast<CColorProperty>(pProp);

                    CColor Color = pColor->Value(pData);
                    pColorPicker->SetColor(TO_QCOLOR(Color));
                    break;
                }

                case EPropertyTypeNew::String:
                {
                    QLineEdit *pLineEdit = static_cast<QLineEdit*>(pEditor);

                    if (!pLineEdit->hasFocus())
                    {
                        CStringProperty *pString = TPropCast<CStringProperty>(pProp);
                        pLineEdit->setText( TO_QSTRING(pString->Value(pData)) );
                    }

                    break;
                }

                case EPropertyTypeNew::Enum:
                case EPropertyTypeNew::Choice:
                {
                    QComboBox *pComboBox = static_cast<QComboBox*>(pEditor);
                    CEnumProperty* pEnum = TPropCast<CEnumProperty>(pProp);
                    pComboBox->setCurrentIndex( pEnum->ValueIndex( pEnum->Value(pData) ) );
                    break;
                }

                case EPropertyTypeNew::Asset:
                {
                    CResourceSelector *pSelector = static_cast<CResourceSelector*>(pEditor);
                    CAssetProperty *pAsset = TPropCast<CAssetProperty>(pProp);
                    pSelector->SetResource(pAsset->Value(pData));
                    break;
                }

                case EPropertyTypeNew::Array:
                {
                    WIntegralSpinBox *pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);

                    if (!pSpinBox->hasFocus())
                    {
                        CArrayProperty *pArray = static_cast<CArrayProperty*>(pProp);
                        pSpinBox->setValue( pArray->ArrayCount(pData) );
                    }

                    break;
                }

                }
            }
        }

        // Set editor data for animation set/flags sub-property
        else if (rkIndex.internalId() & 0x80000000)
        {
            pProp = mpModel->PropertyForIndex(rkIndex, true);

            if (pProp->Type() == EPropertyTypeNew::AnimationSet)
                SetCharacterEditorData(pEditor, rkIndex);

            else if (pProp->Type() == EPropertyTypeNew::Flags)
            {
                QCheckBox *pCheckBox = static_cast<QCheckBox*>(pEditor);
                CFlagsProperty* pFlags = TPropCast<CFlagsProperty>(pProp);
                u32 Mask = pFlags->FlagMask(rkIndex.row());
                bool Set = (pFlags->Value(pData) & Mask) != 0;
                pCheckBox->setChecked(Set);
            }
        }
    }

    BlockRelays(false);
}

void CPropertyDelegate::setModelData(QWidget *pEditor, QAbstractItemModel* /*pModel*/, const QModelIndex &rkIndex) const
{
    if (!mpModel) return;
    if (!pEditor) return;

    IEditPropertyCommand* pCommand = nullptr;
    IPropertyNew *pProp = mpModel->PropertyForIndex(rkIndex, true);
    void* pData = mpModel->DataPointerForIndex(rkIndex);

    if (pProp)
    {
        EPropertyTypeNew Type = pProp->Type();

        if (Type != EPropertyTypeNew::Array)
        {
            // TODO: support this for non script object properties
            pCommand = new CEditScriptPropertyCommand(mpEditor, rkIndex, mpModel);
            pCommand->SaveOldData();

            // Handle sub-properties of flags and animation sets
            if (rkIndex.internalId() & 0x80000000)
            {
                if (pProp->Type() == EPropertyTypeNew::AnimationSet)
                    SetCharacterModelData(pEditor, rkIndex);

                else if (pProp->Type() == EPropertyTypeNew::Flags)
                {
                    QCheckBox* pCheckBox = static_cast<QCheckBox*>(pEditor);
                    CFlagsProperty* pFlags = static_cast<CFlagsProperty*>(pProp);
                    u32 Mask = pFlags->FlagMask(rkIndex.row());

                    int Flags = pFlags->Value(pData);
                    if (pCheckBox->isChecked()) Flags |= Mask;
                    else Flags &= ~Mask;
                    pFlags->ValueRef(pData) = Flags;
                }
            }

            else
            {
                switch (pProp->Type())
                {

                case EPropertyTypeNew::Bool:
                {
                    QCheckBox *pCheckBox = static_cast<QCheckBox*>(pEditor);
                    CBoolProperty* pBool = static_cast<CBoolProperty*>(pProp);
                    pBool->ValueRef(pData) = pCheckBox->isChecked();
                    break;
                }

                case EPropertyTypeNew::Short:
                {
                    WIntegralSpinBox* pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);
                    CShortProperty* pShort = static_cast<CShortProperty*>(pProp);
                    pShort->ValueRef(pData) = pSpinBox->value();
                    break;
                }

                case EPropertyTypeNew::Int:
                {
                    WIntegralSpinBox* pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);
                    CIntProperty* pInt = static_cast<CIntProperty*>(pProp);
                    pInt->ValueRef(pData) = pSpinBox->value();
                    break;
                }

                case EPropertyTypeNew::Sound:
                {
                    WIntegralSpinBox* pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);
                    CSoundProperty* pSound = static_cast<CSoundProperty*>(pProp);
                    pSound->ValueRef(pData) = pSpinBox->value();
                    break;
                }

                case EPropertyTypeNew::Float:
                {
                    WDraggableSpinBox* pSpinBox = static_cast<WDraggableSpinBox*>(pEditor);
                    CFloatProperty* pFloat = static_cast<CFloatProperty*>(pProp);
                    pFloat->ValueRef(pData) = (float) pSpinBox->value();
                    break;
                }

                case EPropertyTypeNew::Color:
                {
                    WColorPicker* pColorPicker = static_cast<WColorPicker*>(pEditor);
                    CColorProperty* pColor = static_cast<CColorProperty*>(pProp);

                    QColor Color = pColorPicker->Color();
                    pColor->ValueRef(pData) = TO_CCOLOR(Color);
                    break;
                }

                case EPropertyTypeNew::String:
                {
                    QLineEdit* pLineEdit = static_cast<QLineEdit*>(pEditor);
                    CStringProperty* pString = static_cast<CStringProperty*>(pProp);
                    pString->ValueRef(pData) = TO_TSTRING(pLineEdit->text());
                    break;
                }

                case EPropertyTypeNew::Enum:
                case EPropertyTypeNew::Choice:
                {
                    QComboBox* pComboBox = static_cast<QComboBox*>(pEditor);
                    CEnumProperty* pEnum = static_cast<CEnumProperty*>(pProp);
                    pEnum->ValueRef(pData) = pEnum->ValueID(pComboBox->currentIndex());
                    break;
                }

                case EPropertyTypeNew::Asset:
                {
                    CResourceSelector* pSelector = static_cast<CResourceSelector*>(pEditor);
                    CResourceEntry* pEntry = pSelector->Entry();

                    CAssetProperty* pAsset = static_cast<CAssetProperty*>(pProp);
                    pAsset->ValueRef(pData) = (pEntry ? pEntry->ID() : CAssetID::InvalidID(pAsset->Game()));
                    break;
                }

                }
            }

            pCommand->SaveNewData();
        }

        // Array
        else
        {
            pCommand = new CResizeScriptArrayCommand(mpEditor, rkIndex, mpModel);
            pCommand->SaveOldData();

            WIntegralSpinBox* pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);
            CArrayProperty* pArray = static_cast<CArrayProperty*>(pProp);
            int OldCount = pArray->ArrayCount(pData);
            int NewCount = pSpinBox->value();

            if (OldCount != NewCount)
            {
                mpModel->ArrayAboutToBeResized(rkIndex, NewCount);
                pArray->Resize(pData, NewCount);
                mpModel->ArrayResized(rkIndex, OldCount);
            }

            pCommand->SaveNewData();
        }
    }

    if (pCommand)
    {
        // Check for edit in progress
        bool DataChanged = pCommand->IsNewDataDifferent();

        if (DataChanged && mInRelayWidgetEdit && (pEditor->hasFocus() || pProp->Type() == EPropertyTypeNew::Color))
            mEditInProgress = true;

        bool EditWasInProgress = mEditInProgress;

        // Check for edit finished
        if (!mInRelayWidgetEdit || (!pEditor->hasFocus() && pProp->Type() != EPropertyTypeNew::Color))
            mEditInProgress = false;

        // Push undo command
        if (DataChanged || EditWasInProgress)
        {
            // Always consider the edit done for bool properties
            pCommand->SetEditComplete(!mEditInProgress || pProp->Type() == EPropertyTypeNew::Bool);
            mpEditor->UndoStack()->push(pCommand);
        }

        else
            delete pCommand;
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
    CAnimationSetProperty* pAnimSetProp = TPropCast<CAnimationSetProperty>(mpModel->PropertyForIndex(rkIndex, true));
    CAnimationParameters Params = pAnimSetProp->Value(mpModel->DataPointerForIndex(rkIndex));

    // Determine property type
    EPropertyTypeNew Type = DetermineCharacterPropType(Params.Version(), rkIndex);

    // Create widget
    if (Type == EPropertyTypeNew::Asset)
    {
        CResourceSelector* pSelector = new CResourceSelector(pParent);
        pSelector->SetFrameVisible(false);

        if (Params.Version() <= eEchoes)
            pSelector->SetTypeFilter(mpEditor->CurrentGame(), "ANCS");
        else
            pSelector->SetTypeFilter(mpEditor->CurrentGame(), "CHAR");

        CONNECT_RELAY(pSelector, rkIndex, ResourceChanged(CResourceEntry*));
        return pSelector;
    }

    else if (Type == EPropertyTypeNew::Enum || Type == EPropertyTypeNew::Choice)
    {
        QComboBox* pComboBox = new QComboBox(pParent);
        CAnimSet* pAnimSet = Params.AnimSet();

        if (pAnimSet)
        {
            for (u32 CharIdx = 0; CharIdx < pAnimSet->NumCharacters(); CharIdx++)
                pComboBox->addItem(TO_QSTRING(pAnimSet->Character(CharIdx)->Name));
        }

        CONNECT_RELAY(pComboBox, rkIndex, currentIndexChanged(int));
        return pComboBox;
    }

    else if (Type == EPropertyTypeNew::Int)
    {
        WIntegralSpinBox *pSpinBox = new WIntegralSpinBox(pParent);
        CONNECT_RELAY(pSpinBox, rkIndex, valueChanged(int));
        return pSpinBox;
    }

    return nullptr;
}

void CPropertyDelegate::SetCharacterEditorData(QWidget *pEditor, const QModelIndex& rkIndex) const
{
    CAnimationSetProperty* pAnimSetProp = TPropCast<CAnimationSetProperty>(mpModel->PropertyForIndex(rkIndex, true));
    CAnimationParameters Params = pAnimSetProp->Value(mpModel->DataPointerForIndex(rkIndex));
    EPropertyTypeNew Type = DetermineCharacterPropType(Params.Version(), rkIndex);

    if (Type == EPropertyTypeNew::Asset)
    {
        static_cast<CResourceSelector*>(pEditor)->SetResource(Params.AnimSet());
    }

    else if (Type == EPropertyTypeNew::Enum || Type == EPropertyTypeNew::Choice)
    {
        static_cast<QComboBox*>(pEditor)->setCurrentIndex(Params.CharacterIndex());
    }

    else if (Type == EPropertyTypeNew::Int && !pEditor->hasFocus())
    {
        int UnkIndex = (Params.Version() <= eEchoes ? rkIndex.row() - 2 : rkIndex.row() - 1);
        u32 Value = Params.Unknown(UnkIndex);
        static_cast<WIntegralSpinBox*>(pEditor)->setValue(Value);
    }
}

void CPropertyDelegate::SetCharacterModelData(QWidget *pEditor, const QModelIndex& rkIndex) const
{
    CAnimationSetProperty* pAnimSetProp = TPropCast<CAnimationSetProperty>(mpModel->PropertyForIndex(rkIndex, true));
    CAnimationParameters Params = pAnimSetProp->Value(mpModel->DataPointerForIndex(rkIndex));
    EPropertyTypeNew Type = DetermineCharacterPropType(Params.Version(), rkIndex);

    if (Type == EPropertyTypeNew::Asset)
    {
        CResourceEntry *pEntry = static_cast<CResourceSelector*>(pEditor)->Entry();
        Params.SetResource( pEntry ? pEntry->ID() : CAssetID::InvalidID(mpEditor->CurrentGame()) );
    }

    else if (Type == EPropertyTypeNew::Enum || Type == EPropertyTypeNew::Choice)
    {
        Params.SetCharIndex( static_cast<QComboBox*>(pEditor)->currentIndex() );
    }

    else if (Type == EPropertyTypeNew::Int)
    {
        int UnkIndex = (Params.Version() <= eEchoes ? rkIndex.row() - 2 : rkIndex.row() - 1);
        Params.SetUnknown(UnkIndex, static_cast<WIntegralSpinBox*>(pEditor)->value() );
    }

    pAnimSetProp->ValueRef(mpModel->DataPointerForIndex(rkIndex)) = Params;

    // If we just updated the resource, make sure all the sub-properties of the character are flagged as changed.
    // We want to do this -after- updating the anim params on the property, which is why we have a second type check.
    if (Type == EPropertyTypeNew::Asset)
    {
        QModelIndex ParentIndex = rkIndex.parent();
        mpModel->dataChanged(mpModel->index(1, 1, ParentIndex), mpModel->index(mpModel->rowCount(ParentIndex) - 1, 1, ParentIndex));
    }
}

EPropertyTypeNew CPropertyDelegate::DetermineCharacterPropType(EGame Game, const QModelIndex& rkIndex) const
{
    if (Game <= eEchoes)
    {
        if      (rkIndex.row() == 0) return EPropertyTypeNew::Asset;
        else if (rkIndex.row() == 1) return EPropertyTypeNew::Choice;
        else if (rkIndex.row() == 2) return EPropertyTypeNew::Int;
    }
    else if (Game <= eCorruption)
    {
        if      (rkIndex.row() == 0) return EPropertyTypeNew::Asset;
        else if (rkIndex.row() == 1) return EPropertyTypeNew::Int;
    }
    else
    {
        if      (rkIndex.row() == 0) return EPropertyTypeNew::Asset;
        else if (rkIndex.row() <= 2) return EPropertyTypeNew::Int;
    }
    return EPropertyTypeNew::Invalid;
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
