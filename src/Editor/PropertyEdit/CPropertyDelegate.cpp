#include "CPropertyDelegate.h"
#include "CPropertyRelay.h"

#include "Editor/UICommon.h"
#include "Editor/Undo/CEditScriptPropertyCommand.h"
#include "Editor/Undo/CEditIntrinsicPropertyCommand.h"
#include "Editor/Undo/CResizeScriptArrayCommand.h"
#include "Editor/Widgets/CResourceSelector.h"
#include "Editor/Widgets/WColorPicker.h"
#include "Editor/Widgets/WDraggableSpinBox.h"
#include "Editor/Widgets/WIntegralSpinBox.h"

#include <Core/Resource/Animation/CAnimSet.h>
#include <Core/Resource/Script/Property/Properties.h>

#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QLineEdit>

// This macro should be used on every widget where changes should be reflected in realtime and not just when the edit is finished.
#define CONNECT_RELAY(Widget, Index, Signal) \
    do { \
        CPropertyRelay *pRelay = new CPropertyRelay(Widget, Index); \
        connect(Widget, Signal, pRelay, &CPropertyRelay::OnWidgetEdited); \
        connect(pRelay, &CPropertyRelay::WidgetEdited, this, &CPropertyDelegate::WidgetEdited); \
    } while (false)

CPropertyDelegate::CPropertyDelegate(QObject* pParent)
    : QStyledItemDelegate(pParent)
{
    mpEditor = UICommon::FindAncestor<IEditor>(pParent);
}

void CPropertyDelegate::SetEditor(IEditor* pEditor)
{
    mpEditor = pEditor;
}

void CPropertyDelegate::SetPropertyModel(CPropertyModel* pModel)
{
    mpModel = pModel;
}

QWidget* CPropertyDelegate::createEditor(QWidget* pParent, const QStyleOptionViewItem& /*rkOption*/, const QModelIndex& rkIndex) const
{
    if (!mpModel) return nullptr;
    IProperty *pProp = mpModel->PropertyForIndex(rkIndex, false);
    QWidget *pOut = nullptr;

    if (pProp)
    {
        EPropertyType Type = mpModel->GetEffectiveFieldType(pProp);

        switch (Type)
        {

        case EPropertyType::Bool:
        {
            QCheckBox *pCheckBox = new QCheckBox(pParent);
            CONNECT_RELAY(pCheckBox, rkIndex, &QCheckBox::toggled);
            pOut = pCheckBox;
            break;
        }

        case EPropertyType::Short:
        {
            WIntegralSpinBox *pSpinBox = new WIntegralSpinBox(pParent);
            pSpinBox->setMinimum(INT16_MIN);
            pSpinBox->setMaximum(INT16_MAX);
            pSpinBox->setSuffix(TO_QSTRING(pProp->Suffix()));
            CONNECT_RELAY(pSpinBox, rkIndex, qOverload<int>(&WIntegralSpinBox::valueChanged));
            pOut = pSpinBox;
            break;
        }

        case EPropertyType::Int:
        {
            WIntegralSpinBox *pSpinBox = new WIntegralSpinBox(pParent);
            pSpinBox->setMinimum(INT32_MIN);
            pSpinBox->setMaximum(INT32_MAX);
            pSpinBox->setSuffix(TO_QSTRING(pProp->Suffix()));
            CONNECT_RELAY(pSpinBox, rkIndex, qOverload<int>(&WIntegralSpinBox::valueChanged));
            pOut = pSpinBox;
            break;
        }

        case EPropertyType::Float:
        {
            WDraggableSpinBox *pSpinBox = new WDraggableSpinBox(pParent);
            pSpinBox->setSingleStep(0.1);
            pSpinBox->setSuffix(TO_QSTRING(pProp->Suffix()));
            CONNECT_RELAY(pSpinBox, rkIndex, qOverload<double>(&WDraggableSpinBox::valueChanged));
            pOut = pSpinBox;
            break;
        }

        case EPropertyType::Color:
        {
            WColorPicker *pColorPicker = new WColorPicker(pParent);
            CONNECT_RELAY(pColorPicker, rkIndex, &WColorPicker::ColorChanged);
            pOut = pColorPicker;
            break;
        }

        case EPropertyType::Sound:
        {
            WIntegralSpinBox *pSpinBox = new WIntegralSpinBox(pParent);
            pSpinBox->setMinimum(-1);
            pSpinBox->setMaximum(0xFFFF);
            CONNECT_RELAY(pSpinBox, rkIndex, qOverload<int>(&WIntegralSpinBox::valueChanged));
            pOut = pSpinBox;
            break;
        }

        case EPropertyType::String:
        {
            QLineEdit *pLineEdit = new QLineEdit(pParent);
            CONNECT_RELAY(pLineEdit, rkIndex, &QLineEdit::textEdited);
            pOut = pLineEdit;
            break;
        }

        case EPropertyType::Enum:
        case EPropertyType::Choice:
        {
            QComboBox *pComboBox = new QComboBox(pParent);
            CEnumProperty* pEnum = TPropCast<CEnumProperty>(pProp);

            for (size_t ValueIdx = 0; ValueIdx < pEnum->NumPossibleValues(); ValueIdx++)
                pComboBox->addItem(TO_QSTRING(pEnum->ValueName(ValueIdx)));

            CONNECT_RELAY(pComboBox, rkIndex, qOverload<int>(&QComboBox::currentIndexChanged));
            pOut = pComboBox;
            break;
        }

        case EPropertyType::Asset:
        {
            CResourceSelector *pSelector = new CResourceSelector(pParent);
            pSelector->SetFrameVisible(false);

            CAssetProperty *pAsset = TPropCast<CAssetProperty>(pProp);
            pSelector->SetTypeFilter(pAsset->GetTypeFilter());

            CONNECT_RELAY(pSelector, rkIndex, &CResourceSelector::ResourceChanged);
            pOut = pSelector;
            break;
        }

        case EPropertyType::Array:
        {
            // No relay here, would prefer user to be sure of their change before it's reflected on the UI
            WIntegralSpinBox *pSpinBox = new WIntegralSpinBox(pParent);
            pSpinBox->setMinimum(0);
            pSpinBox->setMaximum(999);
            pOut = pSpinBox;
            break;
        }

        default: break;
        }
    }

    // Check for sub-property of flags/animation set
    else if (rkIndex.internalId() & 0x80000000)
    {
        pProp = mpModel->PropertyForIndex(rkIndex, true);
        EPropertyType Type = mpModel->GetEffectiveFieldType(pProp);

        // Handle character
        if (Type == EPropertyType::AnimationSet)
            pOut = CreateCharacterEditor(pParent, rkIndex);

        // Handle flags
        else if (Type == EPropertyType::Flags)
        {
            QCheckBox *pCheckBox = new QCheckBox(pParent);
            CONNECT_RELAY(pCheckBox, rkIndex, &QCheckBox::toggled);
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
        IProperty *pProp = mpModel->PropertyForIndex(rkIndex, false);
        void* pData = mpModel->DataPointerForIndex(rkIndex);

        if (pProp)
        {
            if (!mEditInProgress)
            {
                EPropertyType Type = mpModel->GetEffectiveFieldType(pProp);

                switch (Type)
                {

                case EPropertyType::Bool:
                {
                    QCheckBox *pCheckBox = static_cast<QCheckBox*>(pEditor);
                    CBoolProperty *pBool = TPropCast<CBoolProperty>(pProp);
                    pCheckBox->setChecked( pBool->Value(pData) );
                    break;
                }

                case EPropertyType::Short:
                {
                    WIntegralSpinBox *pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);

                    if (!pSpinBox->hasFocus())
                    {
                        CShortProperty *pShort = TPropCast<CShortProperty>(pProp);
                        pSpinBox->setValue( pShort->Value(pData) );
                    }

                    break;
                }

                case EPropertyType::Int:
                {
                    WIntegralSpinBox *pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);

                    if (!pSpinBox->hasFocus())
                    {
                        // Ints use static_cast since sometimes we treat other property types as ints
                        CIntProperty *pInt = static_cast<CIntProperty*>(pProp);
                        pSpinBox->setValue( pInt->Value(pData) );
                    }

                    break;
                }

                case EPropertyType::Sound:
                {
                    WIntegralSpinBox *pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);

                    if (!pSpinBox->hasFocus())
                    {
                        CSoundProperty *pSound = TPropCast<CSoundProperty>(pProp);
                        pSpinBox->setValue( pSound->Value(pData) );
                    }

                    break;
                }

                case EPropertyType::Float:
                {
                    WDraggableSpinBox *pSpinBox = static_cast<WDraggableSpinBox*>(pEditor);

                    if (!pSpinBox->hasFocus())
                    {
                        CFloatProperty *pFloat = TPropCast<CFloatProperty>(pProp);
                        pSpinBox->setValue( pFloat->Value(pData) );
                    }

                    break;
                }

                case EPropertyType::Color:
                {
                    WColorPicker *pColorPicker = static_cast<WColorPicker*>(pEditor);
                    CColorProperty *pColor = TPropCast<CColorProperty>(pProp);

                    CColor Color = pColor->Value(pData);
                    pColorPicker->SetColor(TO_QCOLOR(Color));
                    break;
                }

                case EPropertyType::String:
                {
                    QLineEdit *pLineEdit = static_cast<QLineEdit*>(pEditor);

                    if (!pLineEdit->hasFocus())
                    {
                        CStringProperty *pString = TPropCast<CStringProperty>(pProp);
                        pLineEdit->setText( TO_QSTRING(pString->Value(pData)) );
                    }

                    break;
                }

                case EPropertyType::Enum:
                case EPropertyType::Choice:
                {
                    QComboBox *pComboBox = static_cast<QComboBox*>(pEditor);
                    CEnumProperty* pEnum = TPropCast<CEnumProperty>(pProp);
                    pComboBox->setCurrentIndex( pEnum->ValueIndex( pEnum->Value(pData) ) );
                    break;
                }

                case EPropertyType::Asset:
                {
                    CResourceSelector *pSelector = static_cast<CResourceSelector*>(pEditor);
                    CAssetProperty *pAsset = TPropCast<CAssetProperty>(pProp);
                    pSelector->SetResource(pAsset->Value(pData));
                    break;
                }

                case EPropertyType::Array:
                {
                    WIntegralSpinBox *pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);

                    if (!pSpinBox->hasFocus())
                    {
                        CArrayProperty *pArray = static_cast<CArrayProperty*>(pProp);
                        pSpinBox->setValue( pArray->ArrayCount(pData) );
                    }

                    break;
                }
                default: break;
                }
            }
        }

        // Set editor data for animation set/flags sub-property
        else if (rkIndex.internalId() & 0x80000000)
        {
            pProp = mpModel->PropertyForIndex(rkIndex, true);
            EPropertyType Type = mpModel->GetEffectiveFieldType(pProp);

            if (Type == EPropertyType::AnimationSet)
                SetCharacterEditorData(pEditor, rkIndex);

            else if (Type == EPropertyType::Flags)
            {
                QCheckBox *pCheckBox = static_cast<QCheckBox*>(pEditor);
                CFlagsProperty* pFlags = TPropCast<CFlagsProperty>(pProp);
                uint32 Mask = pFlags->FlagMask(rkIndex.row());
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
    IProperty *pProp = mpModel->PropertyForIndex(rkIndex, true);
    void* pData = mpModel->DataPointerForIndex(rkIndex);

    if (pProp)
    {
        EPropertyType Type = mpModel->GetEffectiveFieldType(pProp);
        CScriptObject* pObject = mpModel->GetScriptObject();

        if (!pObject)
        {
            const QVector<void*> DataPointers{pData};
            pCommand = new CEditIntrinsicPropertyCommand(pProp, DataPointers, mpModel, rkIndex);
        }
        else
        {
            const QVector<CScriptObject*> Objects{pObject};
            pCommand = (Type != EPropertyType::Array) ?
                        new CEditScriptPropertyCommand(pProp, Objects, mpModel, rkIndex) :
                        new CResizeScriptArrayCommand (pProp, Objects, mpModel, rkIndex);
        }

        pCommand->SaveOldData();

        if (Type != EPropertyType::Array)
        {
            // Handle sub-properties of flags and animation sets
            if (rkIndex.internalId() & 0x80000000)
            {
                if (Type == EPropertyType::AnimationSet)
                    SetCharacterModelData(pEditor, rkIndex);

                else if (Type == EPropertyType::Flags)
                {
                    QCheckBox* pCheckBox = static_cast<QCheckBox*>(pEditor);
                    CFlagsProperty* pFlags = static_cast<CFlagsProperty*>(pProp);
                    uint32 Mask = pFlags->FlagMask(rkIndex.row());

                    int Flags = pFlags->Value(pData);
                    if (pCheckBox->isChecked()) Flags |= Mask;
                    else Flags &= ~Mask;
                    pFlags->ValueRef(pData) = Flags;
                }
            }

            else
            {
                switch (Type)
                {

                case EPropertyType::Bool:
                {
                    QCheckBox *pCheckBox = static_cast<QCheckBox*>(pEditor);
                    CBoolProperty* pBool = static_cast<CBoolProperty*>(pProp);
                    pBool->ValueRef(pData) = pCheckBox->isChecked();
                    break;
                }

                case EPropertyType::Short:
                {
                    WIntegralSpinBox* pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);
                    CShortProperty* pShort = static_cast<CShortProperty*>(pProp);
                    pShort->ValueRef(pData) = pSpinBox->value();
                    break;
                }

                case EPropertyType::Int:
                {
                    // Ints use static_cast since sometimes we treat other property types as ints
                    WIntegralSpinBox* pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);
                    CIntProperty* pInt = static_cast<CIntProperty*>(pProp);
                    pInt->ValueRef(pData) = pSpinBox->value();
                    break;
                }

                case EPropertyType::Sound:
                {
                    WIntegralSpinBox* pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);
                    CSoundProperty* pSound = static_cast<CSoundProperty*>(pProp);
                    pSound->ValueRef(pData) = pSpinBox->value();
                    break;
                }

                case EPropertyType::Float:
                {
                    WDraggableSpinBox* pSpinBox = static_cast<WDraggableSpinBox*>(pEditor);
                    CFloatProperty* pFloat = static_cast<CFloatProperty*>(pProp);
                    pFloat->ValueRef(pData) = (float) pSpinBox->value();
                    break;
                }

                case EPropertyType::Color:
                {
                    WColorPicker* pColorPicker = static_cast<WColorPicker*>(pEditor);
                    CColorProperty* pColor = static_cast<CColorProperty*>(pProp);

                    QColor Color = pColorPicker->Color();
                    pColor->ValueRef(pData) = TO_CCOLOR(Color);
                    break;
                }

                case EPropertyType::String:
                {
                    QLineEdit* pLineEdit = static_cast<QLineEdit*>(pEditor);
                    CStringProperty* pString = static_cast<CStringProperty*>(pProp);
                    pString->ValueRef(pData) = TO_TSTRING(pLineEdit->text());
                    break;
                }

                case EPropertyType::Enum:
                case EPropertyType::Choice:
                {
                    QComboBox* pComboBox = static_cast<QComboBox*>(pEditor);
                    CEnumProperty* pEnum = static_cast<CEnumProperty*>(pProp);
                    pEnum->ValueRef(pData) = pEnum->ValueID(pComboBox->currentIndex());
                    break;
                }

                case EPropertyType::Asset:
                {
                    CResourceSelector* pSelector = static_cast<CResourceSelector*>(pEditor);
                    CResourceEntry* pEntry = pSelector->Entry();

                    CAssetProperty* pAsset = static_cast<CAssetProperty*>(pProp);
                    pAsset->ValueRef(pData) = (pEntry ? pEntry->ID() : CAssetID::InvalidID(pAsset->Game()));
                    break;
                }
                default: break;
                }
            }

            pCommand->SaveNewData();
        }

        // Array
        else
        {
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

        if (DataChanged && mInRelayWidgetEdit && (pEditor->hasFocus() || pProp->Type() == EPropertyType::Color))
            mEditInProgress = true;

        bool EditWasInProgress = mEditInProgress;

        // Check for edit finished
        if (!mInRelayWidgetEdit || (!pEditor->hasFocus() && pProp->Type() != EPropertyType::Color))
            mEditInProgress = false;

        // Push undo command
        if (DataChanged || EditWasInProgress)
        {
            // Always consider the edit done for bool properties
            pCommand->SetEditComplete(!mEditInProgress || pProp->Type() == EPropertyType::Bool);
            mpEditor->UndoStack().push(pCommand);
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
    EPropertyType Type = DetermineCharacterPropType(Params.Version(), rkIndex);

    // Create widget
    if (Type == EPropertyType::Asset)
    {
        CResourceSelector* pSelector = new CResourceSelector(pParent);
        pSelector->SetFrameVisible(false);

        if (Params.Version() <= EGame::Echoes)
            pSelector->SetTypeFilter(gpEdApp->CurrentGame(), "ANCS");
        else
            pSelector->SetTypeFilter(gpEdApp->CurrentGame(), "CHAR");

        CONNECT_RELAY(pSelector, rkIndex, &CResourceSelector::ResourceChanged);
        return pSelector;
    }

    if (Type == EPropertyType::Enum || Type == EPropertyType::Choice)
    {
        QComboBox* pComboBox = new QComboBox(pParent);
        CAnimSet* pAnimSet = Params.AnimSet();

        if (pAnimSet)
        {
            for (size_t CharIdx = 0; CharIdx < pAnimSet->NumCharacters(); CharIdx++)
                pComboBox->addItem(TO_QSTRING(pAnimSet->Character(CharIdx)->Name));
        }

        CONNECT_RELAY(pComboBox, rkIndex, qOverload<int>(&QComboBox::currentIndexChanged));
        return pComboBox;
    }

    if (Type == EPropertyType::Int)
    {
        WIntegralSpinBox *pSpinBox = new WIntegralSpinBox(pParent);
        CONNECT_RELAY(pSpinBox, rkIndex, qOverload<int>(&WIntegralSpinBox::valueChanged));
        return pSpinBox;
    }

    return nullptr;
}

void CPropertyDelegate::SetCharacterEditorData(QWidget *pEditor, const QModelIndex& rkIndex) const
{
    CAnimationSetProperty* pAnimSetProp = TPropCast<CAnimationSetProperty>(mpModel->PropertyForIndex(rkIndex, true));
    CAnimationParameters Params = pAnimSetProp->Value(mpModel->DataPointerForIndex(rkIndex));
    EPropertyType Type = DetermineCharacterPropType(Params.Version(), rkIndex);

    if (Type == EPropertyType::Asset)
    {
        static_cast<CResourceSelector*>(pEditor)->SetResource(Params.AnimSet());
    }

    else if (Type == EPropertyType::Enum || Type == EPropertyType::Choice)
    {
        static_cast<QComboBox*>(pEditor)->setCurrentIndex(Params.CharacterIndex());
    }

    else if (Type == EPropertyType::Int && !pEditor->hasFocus())
    {
        int UnkIndex = (Params.Version() <= EGame::Echoes ? rkIndex.row() - 2 : rkIndex.row() - 1);
        uint32 Value = Params.Unknown(UnkIndex);
        static_cast<WIntegralSpinBox*>(pEditor)->setValue(Value);
    }
}

void CPropertyDelegate::SetCharacterModelData(QWidget *pEditor, const QModelIndex& rkIndex) const
{
    CAnimationSetProperty* pAnimSetProp = TPropCast<CAnimationSetProperty>(mpModel->PropertyForIndex(rkIndex, true));
    CAnimationParameters Params = pAnimSetProp->Value(mpModel->DataPointerForIndex(rkIndex));
    EPropertyType Type = DetermineCharacterPropType(Params.Version(), rkIndex);

    if (Type == EPropertyType::Asset)
    {
        CResourceEntry *pEntry = static_cast<CResourceSelector*>(pEditor)->Entry();
        Params.SetResource( pEntry ? pEntry->ID() : CAssetID::InvalidID(gpEdApp->CurrentGame()) );
    }

    else if (Type == EPropertyType::Enum || Type == EPropertyType::Choice)
    {
        Params.SetCharIndex( static_cast<QComboBox*>(pEditor)->currentIndex() );
    }

    else if (Type == EPropertyType::Int)
    {
        int UnkIndex = (Params.Version() <= EGame::Echoes ? rkIndex.row() - 2 : rkIndex.row() - 1);
        Params.SetUnknown(UnkIndex, static_cast<WIntegralSpinBox*>(pEditor)->value() );
    }

    pAnimSetProp->ValueRef(mpModel->DataPointerForIndex(rkIndex)) = Params;

    // If we just updated the resource, make sure all the sub-properties of the character are flagged as changed.
    // We want to do this -after- updating the anim params on the property, which is why we have a second type check.
    if (Type == EPropertyType::Asset)
    {
        QModelIndex ParentIndex = rkIndex.parent();
        mpModel->dataChanged(mpModel->index(1, 1, ParentIndex), mpModel->index(mpModel->rowCount(ParentIndex) - 1, 1, ParentIndex));
    }
}

EPropertyType CPropertyDelegate::DetermineCharacterPropType(EGame Game, const QModelIndex& rkIndex) const
{
    if (Game <= EGame::Echoes)
    {
        if      (rkIndex.row() == 0) return EPropertyType::Asset;
        else if (rkIndex.row() == 1) return EPropertyType::Choice;
        else if (rkIndex.row() == 2) return EPropertyType::Int;
    }
    else if (Game <= EGame::Corruption)
    {
        if      (rkIndex.row() == 0) return EPropertyType::Asset;
        else if (rkIndex.row() == 1) return EPropertyType::Int;
    }
    else
    {
        if      (rkIndex.row() == 0) return EPropertyType::Asset;
        else if (rkIndex.row() <= 2) return EPropertyType::Int;
    }
    return EPropertyType::Invalid;
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
