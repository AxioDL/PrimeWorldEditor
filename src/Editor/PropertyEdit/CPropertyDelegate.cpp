#include "Editor/PropertyEdit/CPropertyDelegate.h"

#include "Editor/UICommon.h"
#include "Editor/PropertyEdit/CPropertyModel.h"
#include "Editor/PropertyEdit/CPropertyRelay.h"
#include "Editor/Undo/CEditScriptPropertyCommand.h"
#include "Editor/Undo/CEditIntrinsicPropertyCommand.h"
#include "Editor/Undo/CResizeScriptArrayCommand.h"
#include "Editor/Widgets/CResourceSelector.h"
#include "Editor/Widgets/WColorPicker.h"
#include "Editor/Widgets/WDraggableSpinBox.h"
#include "Editor/Widgets/WIntegralSpinBox.h"
#include "Editor/WorldEditor/CWorldEditor.h"

#include <Core/Resource/Animation/CAnimSet.h>
#include <Core/Resource/Script/Property/Properties.h>

#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QLineEdit>

#include <concepts>
#include <memory>
#include <type_traits>

// This function should be used on every widget where changes should be reflected in realtime and not just when the edit is finished.
template <typename Widget, typename MemPtr>
requires(std::derived_from<Widget, QWidget> && std::is_member_function_pointer_v<MemPtr>)
static void ConnectRelay(const CPropertyDelegate* delegate, Widget* widget, const QModelIndex& idx, MemPtr signal)
{
    auto* relay = new CPropertyRelay(widget, idx);
    delegate->connect(widget, signal, relay, &CPropertyRelay::OnWidgetEdited);
    delegate->connect(relay, &CPropertyRelay::WidgetEdited, delegate, &CPropertyDelegate::WidgetEdited);
}

CPropertyDelegate::CPropertyDelegate(QObject* pParent)
    : QStyledItemDelegate(pParent)
{
    mpEditor = UICommon::FindAncestor<IEditor>(pParent);
}

CPropertyDelegate::~CPropertyDelegate() = default;

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
    if (!mpModel)
        return nullptr;

    IProperty *pProp = mpModel->PropertyForIndex(rkIndex, false);
    QWidget *pOut = nullptr;

    if (pProp)
    {
        EPropertyType Type = mpModel->GetEffectiveFieldType(pProp);

        switch (Type)
        {
        case EPropertyType::Bool:
        {
            auto* pCheckBox = new QCheckBox(pParent);
            ConnectRelay(this, pCheckBox, rkIndex, &QCheckBox::toggled);
            pOut = pCheckBox;
            break;
        }

        case EPropertyType::Short:
        {
            auto* pSpinBox = new WIntegralSpinBox(pParent);
            pSpinBox->setMinimum(INT16_MIN);
            pSpinBox->setMaximum(INT16_MAX);
            pSpinBox->setSuffix(TO_QSTRING(pProp->Suffix()));
            ConnectRelay(this, pSpinBox, rkIndex, &WIntegralSpinBox::valueChanged);
            pOut = pSpinBox;
            break;
        }

        case EPropertyType::Int:
        {
            auto* pSpinBox = new WIntegralSpinBox(pParent);
            pSpinBox->setMinimum(INT32_MIN);
            pSpinBox->setMaximum(INT32_MAX);
            pSpinBox->setSuffix(TO_QSTRING(pProp->Suffix()));
            ConnectRelay(this, pSpinBox, rkIndex, &WIntegralSpinBox::valueChanged);
            pOut = pSpinBox;
            break;
        }

        case EPropertyType::Float:
        {
            auto* pSpinBox = new WDraggableSpinBox(pParent);
            pSpinBox->setSingleStep(0.1);
            pSpinBox->setSuffix(TO_QSTRING(pProp->Suffix()));
            ConnectRelay(this, pSpinBox, rkIndex, &WDraggableSpinBox::valueChanged);
            pOut = pSpinBox;
            break;
        }

        case EPropertyType::Color:
        {
            auto* pColorPicker = new WColorPicker(pParent);
            ConnectRelay(this, pColorPicker, rkIndex, &WColorPicker::ColorChanged);
            pOut = pColorPicker;
            break;
        }

        case EPropertyType::Sound:
        {
            auto* pSpinBox = new WIntegralSpinBox(pParent);
            pSpinBox->setMinimum(-1);
            pSpinBox->setMaximum(0xFFFF);
            ConnectRelay(this, pSpinBox, rkIndex, &WIntegralSpinBox::valueChanged);
            pOut = pSpinBox;
            break;
        }

        case EPropertyType::String:
        {
            auto* pLineEdit = new QLineEdit(pParent);
            ConnectRelay(this, pLineEdit, rkIndex, &QLineEdit::textEdited);
            pOut = pLineEdit;
            break;
        }

        case EPropertyType::Enum:
        case EPropertyType::Choice:
        {
            auto* pComboBox = new QComboBox(pParent);
            auto* pEnum = TPropCast<CEnumProperty>(pProp);

            for (size_t ValueIdx = 0; ValueIdx < pEnum->NumPossibleValues(); ValueIdx++)
                pComboBox->addItem(TO_QSTRING(pEnum->HasReadableName(ValueIdx) ? pEnum->ValueReadableName(ValueIdx) : pEnum->ValueName(ValueIdx)));

            ConnectRelay(this, pComboBox, rkIndex, &QComboBox::currentIndexChanged);
            pOut = pComboBox;
            break;
        }

        case EPropertyType::Asset:
        {
            auto* pSelector = new CResourceSelector(pParent);
            pSelector->SetFrameVisible(false);

            auto* pAsset = TPropCast<CAssetProperty>(pProp);
            pSelector->SetTypeFilter(pAsset->GetTypeFilter());

            ConnectRelay(this, pSelector, rkIndex, &CResourceSelector::ResourceChanged);
            pOut = pSelector;
            break;
        }

        case EPropertyType::Array:
        {
            // No relay here, would prefer user to be sure of their change before it's reflected on the UI
            auto* pSpinBox = new WIntegralSpinBox(pParent);
            pSpinBox->setMinimum(0);
            pSpinBox->setMaximum(999);
            pOut = pSpinBox;
            break;
        }

        default:
            break;
        }
    }

    // Check for sub-property of flags/animation set
    else if (rkIndex.internalId() & 0x80000000)
    {
        pProp = mpModel->PropertyForIndex(rkIndex, true);
        const EPropertyType Type = mpModel->GetEffectiveFieldType(pProp);

        // Handle character
        if (Type == EPropertyType::AnimationSet)
        {
            pOut = CreateCharacterEditor(pParent, rkIndex);
        }
        else if (Type == EPropertyType::Flags)
        {
            // Handle flags
            auto* pCheckBox = new QCheckBox(pParent);
            ConnectRelay(this, pCheckBox, rkIndex, &QCheckBox::toggled);
            pOut = pCheckBox;
        }
    }

    if (pOut)
    {
        pOut->setFocusPolicy(Qt::StrongFocus);
        const QSize Size = mpModel->data(rkIndex, Qt::SizeHintRole).toSize();
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
                const auto Type = mpModel->GetEffectiveFieldType(pProp);

                switch (Type)
                {
                case EPropertyType::Bool:
                {
                    auto* pCheckBox = static_cast<QCheckBox*>(pEditor);
                    const auto* pBool = TPropCast<CBoolProperty>(pProp);
                    pCheckBox->setChecked(pBool->Value(pData));
                    break;
                }

                case EPropertyType::Short:
                {
                    auto* pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);

                    if (!pSpinBox->hasFocus())
                    {
                        const auto* pShort = TPropCast<CShortProperty>(pProp);
                        pSpinBox->setValue(pShort->Value(pData));
                    }

                    break;
                }

                case EPropertyType::Int:
                {
                    auto* pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);

                    if (!pSpinBox->hasFocus())
                    {
                        // Ints use static_cast since sometimes we treat other property types as ints
                        const auto* pInt = static_cast<CIntProperty*>(pProp);
                        pSpinBox->setValue(pInt->Value(pData));
                    }

                    break;
                }

                case EPropertyType::Sound:
                {
                    auto* pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);

                    if (!pSpinBox->hasFocus())
                    {
                        const auto* pSound = TPropCast<CSoundProperty>(pProp);
                        pSpinBox->setValue(pSound->Value(pData));
                    }

                    break;
                }

                case EPropertyType::Float:
                {
                    auto* pSpinBox = static_cast<WDraggableSpinBox*>(pEditor);

                    if (!pSpinBox->hasFocus())
                    {
                        const auto* pFloat = TPropCast<CFloatProperty>(pProp);
                        pSpinBox->setValue(pFloat->Value(pData));
                    }

                    break;
                }

                case EPropertyType::Color:
                {
                    auto* pColorPicker = static_cast<WColorPicker*>(pEditor);
                    const auto* pColor = TPropCast<CColorProperty>(pProp);

                    const auto& Color = pColor->ValueRef(pData);
                    pColorPicker->SetColor(TO_QCOLOR(Color));
                    break;
                }

                case EPropertyType::String:
                {
                    auto* pLineEdit = static_cast<QLineEdit*>(pEditor);

                    if (!pLineEdit->hasFocus())
                    {
                        const auto* pString = TPropCast<CStringProperty>(pProp);
                        pLineEdit->setText(TO_QSTRING(pString->ValueRef(pData)));
                    }

                    break;
                }

                case EPropertyType::Enum:
                case EPropertyType::Choice:
                {
                    auto* pComboBox = static_cast<QComboBox*>(pEditor);
                    const auto* pEnum = TPropCast<CEnumProperty>(pProp);
                    pComboBox->setCurrentIndex(pEnum->ValueIndex(pEnum->Value(pData)));
                    break;
                }

                case EPropertyType::Asset:
                {
                    auto* pSelector = static_cast<CResourceSelector*>(pEditor);
                    const auto* pAsset = TPropCast<CAssetProperty>(pProp);
                    pSelector->SetResource(pAsset->ValueRef(pData));
                    break;
                }

                case EPropertyType::Array:
                {
                    auto* pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);

                    if (!pSpinBox->hasFocus())
                    {
                        const auto* pArray = static_cast<CArrayProperty*>(pProp);
                        pSpinBox->setValue(pArray->ArrayCount(pData));
                    }

                    break;
                }
                default:
                    break;
                }
            }
        }
        // Set editor data for animation set/flags sub-property
        else if (rkIndex.internalId() & 0x80000000)
        {
            pProp = mpModel->PropertyForIndex(rkIndex, true);
            const EPropertyType Type = mpModel->GetEffectiveFieldType(pProp);

            if (Type == EPropertyType::AnimationSet)
            {
                SetCharacterEditorData(pEditor, rkIndex);
            }
            else if (Type == EPropertyType::Flags)
            {
                auto* pCheckBox = static_cast<QCheckBox*>(pEditor);
                const auto* pFlags = TPropCast<CFlagsProperty>(pProp);
                const auto Mask = pFlags->FlagMask(rkIndex.row());
                const bool Set = (pFlags->Value(pData) & Mask) != 0;
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

    std::unique_ptr<IEditPropertyCommand> pCommand;
    IProperty* pProp = mpModel->PropertyForIndex(rkIndex, true);
    void* pData = mpModel->DataPointerForIndex(rkIndex);

    if (pProp)
    {
        EPropertyType Type = mpModel->GetEffectiveFieldType(pProp);
        CScriptObject* pObject = mpModel->GetScriptObject();

        if (!pObject)
        {
            const QList<void*> DataPointers{pData};
            pCommand = std::make_unique<CEditIntrinsicPropertyCommand>(pProp, DataPointers, mpModel, rkIndex);
        }
        else
        {
            QList<CScriptObject*> Objects{pObject};
            pCommand = (Type != EPropertyType::Array) ?
                        std::make_unique<CEditScriptPropertyCommand>(pProp, Objects, mpModel, rkIndex) :
                        std::make_unique<CResizeScriptArrayCommand>(pProp, Objects, mpModel, rkIndex);
        }

        pCommand->SaveOldData();

        if (Type != EPropertyType::Array)
        {
            // Handle sub-properties of flags and animation sets
            if (rkIndex.internalId() & 0x80000000)
            {
                if (Type == EPropertyType::AnimationSet)
                {
                    SetCharacterModelData(pEditor, rkIndex);
                }
                else if (Type == EPropertyType::Flags)
                {
                    auto* pCheckBox = static_cast<const QCheckBox*>(pEditor);
                    auto* pFlags = static_cast<CFlagsProperty*>(pProp);
                    const auto Mask = pFlags->FlagMask(rkIndex.row());

                    auto Flags = pFlags->Value(pData);
                    if (pCheckBox->isChecked())
                        Flags |= Mask;
                    else
                        Flags &= ~Mask;
                    pFlags->ValueRef(pData) = Flags;
                }
            }
            else
            {
                switch (Type)
                {
                case EPropertyType::Bool:
                {
                    auto* pCheckBox = static_cast<const QCheckBox*>(pEditor);
                    auto* pBool = static_cast<CBoolProperty*>(pProp);
                    pBool->ValueRef(pData) = pCheckBox->isChecked();
                    break;
                }

                case EPropertyType::Short:
                {
                    auto* pSpinBox = static_cast<const WIntegralSpinBox*>(pEditor);
                    auto* pShort = static_cast<CShortProperty*>(pProp);
                    pShort->ValueRef(pData) = pSpinBox->value();
                    break;
                }

                case EPropertyType::Int:
                {
                    // Ints use static_cast since sometimes we treat other property types as ints
                    auto* pSpinBox = static_cast<const WIntegralSpinBox*>(pEditor);
                    auto* pInt = static_cast<CIntProperty*>(pProp);
                    pInt->ValueRef(pData) = pSpinBox->value();
                    break;
                }

                case EPropertyType::Sound:
                {
                    auto* pSpinBox = static_cast<const WIntegralSpinBox*>(pEditor);
                    auto* pSound = static_cast<CSoundProperty*>(pProp);
                    pSound->ValueRef(pData) = pSpinBox->value();
                    break;
                }

                case EPropertyType::Float:
                {
                    auto* pSpinBox = static_cast<const WDraggableSpinBox*>(pEditor);
                    auto* pFloat = static_cast<CFloatProperty*>(pProp);
                    pFloat->ValueRef(pData) = (float) pSpinBox->value();
                    break;
                }

                case EPropertyType::Color:
                {
                    auto* pColorPicker = static_cast<const WColorPicker*>(pEditor);
                    auto* pColor = static_cast<CColorProperty*>(pProp);

                    const QColor& Color = pColorPicker->Color();
                    pColor->ValueRef(pData) = TO_CCOLOR(Color);
                    break;
                }

                case EPropertyType::String:
                {
                    auto* pLineEdit = static_cast<const QLineEdit*>(pEditor);
                    auto* pString = static_cast<CStringProperty*>(pProp);
                    pString->ValueRef(pData) = TO_TSTRING(pLineEdit->text());
                    break;
                }

                case EPropertyType::Enum:
                case EPropertyType::Choice:
                {
                    auto* pComboBox = static_cast<const QComboBox*>(pEditor);
                    auto* pEnum = static_cast<CEnumProperty*>(pProp);
                    pEnum->ValueRef(pData) = pEnum->ValueID(pComboBox->currentIndex());
                    break;
                }

                case EPropertyType::Asset:
                {
                    const auto* pSelector = static_cast<CResourceSelector*>(pEditor);
                    const auto* pEntry = pSelector->Entry();

                    auto* pAsset = static_cast<CAssetProperty*>(pProp);
                    pAsset->ValueRef(pData) = (pEntry ? pEntry->ID() : CAssetID::InvalidID(pAsset->Game()));
                    break;
                }

                default:
                    break;
                }
            }

            pCommand->SaveNewData();
        }
        else // Array
        {
            auto* pSpinBox = static_cast<WIntegralSpinBox*>(pEditor);
            auto* pArray = static_cast<CArrayProperty*>(pProp);
            const int OldCount = pArray->ArrayCount(pData);
            const int NewCount = pSpinBox->value();

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
        const bool DataChanged = pCommand->IsNewDataDifferent();

        if (DataChanged && mInRelayWidgetEdit && (pEditor->hasFocus() || pProp->Type() == EPropertyType::Color))
            mEditInProgress = true;

        const bool EditWasInProgress = mEditInProgress;

        // Check for edit finished
        if (!mInRelayWidgetEdit || (!pEditor->hasFocus() && pProp->Type() != EPropertyType::Color))
            mEditInProgress = false;

        // Push undo command
        if (DataChanged || EditWasInProgress)
        {
            // Always consider the edit done for bool properties
            pCommand->SetEditComplete(!mEditInProgress || pProp->Type() == EPropertyType::Bool);

            // Transfer ownership of the command to the undo stack
            mpEditor->UndoStack().push(pCommand.release());
        }
    }
}

bool CPropertyDelegate::eventFilter(QObject *pObject, QEvent *pEvent)
{
    if (pEvent->type() == QEvent::Wheel)
    {
        auto* pWidget = static_cast<QWidget*>(pObject);

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
    const auto* pAnimSetProp = TPropCast<CAnimationSetProperty>(mpModel->PropertyForIndex(rkIndex, true));
    const auto& Params = pAnimSetProp->ValueRef(mpModel->DataPointerForIndex(rkIndex));

    // Determine property type
    const auto Type = DetermineCharacterPropType(Params.Version(), rkIndex);

    // Create widget
    if (Type == EPropertyType::Asset)
    {
        auto* pSelector = new CResourceSelector(pParent);
        pSelector->SetFrameVisible(false);

        if (Params.Version() <= EGame::Echoes)
            pSelector->SetTypeFilter(gpEdApp->CurrentGame(), "ANCS");
        else
            pSelector->SetTypeFilter(gpEdApp->CurrentGame(), "CHAR");

        ConnectRelay(this, pSelector, rkIndex, &CResourceSelector::ResourceChanged);
        return pSelector;
    }

    if (Type == EPropertyType::Enum || Type == EPropertyType::Choice)
    {
        auto* pComboBox = new QComboBox(pParent);
        const auto* pAnimSet = Params.AnimSet();

        if (pAnimSet)
        {
            for (const auto& character : pAnimSet->Characters())
                pComboBox->addItem(TO_QSTRING(character.Name));
        }

        ConnectRelay(this, pComboBox, rkIndex, &QComboBox::currentIndexChanged);
        return pComboBox;
    }

    if (Type == EPropertyType::Int)
    {
        auto* pSpinBox = new WIntegralSpinBox(pParent);
        ConnectRelay(this, pSpinBox, rkIndex, &WIntegralSpinBox::valueChanged);
        return pSpinBox;
    }

    return nullptr;
}

void CPropertyDelegate::SetCharacterEditorData(QWidget *pEditor, const QModelIndex& rkIndex) const
{
    const auto* pAnimSetProp = TPropCast<CAnimationSetProperty>(mpModel->PropertyForIndex(rkIndex, true));
    const auto& Params = pAnimSetProp->ValueRef(mpModel->DataPointerForIndex(rkIndex));
    const auto Type = DetermineCharacterPropType(Params.Version(), rkIndex);

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
        const int UnkIndex = (Params.Version() <= EGame::Echoes ? rkIndex.row() - 2 : rkIndex.row() - 1);
        const uint32_t Value = Params.Unknown(UnkIndex);
        static_cast<WIntegralSpinBox*>(pEditor)->setValue(Value);
    }
}

void CPropertyDelegate::SetCharacterModelData(QWidget *pEditor, const QModelIndex& rkIndex) const
{
    auto* pAnimSetProp = TPropCast<CAnimationSetProperty>(mpModel->PropertyForIndex(rkIndex, true));
    auto Params = pAnimSetProp->Value(mpModel->DataPointerForIndex(rkIndex));
    const auto Type = DetermineCharacterPropType(Params.Version(), rkIndex);

    if (Type == EPropertyType::Asset)
    {
        const auto* pEntry = static_cast<CResourceSelector*>(pEditor)->Entry();
        Params.SetResource(pEntry);
    }
    else if (Type == EPropertyType::Enum || Type == EPropertyType::Choice)
    {
        Params.SetCharIndex(static_cast<QComboBox*>(pEditor)->currentIndex());
    }
    else if (Type == EPropertyType::Int)
    {
        const int UnkIndex = (Params.Version() <= EGame::Echoes ? rkIndex.row() - 2 : rkIndex.row() - 1);
        Params.SetUnknown(UnkIndex, static_cast<WIntegralSpinBox*>(pEditor)->value());
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

EPropertyType CPropertyDelegate::DetermineCharacterPropType(EGame Game, const QModelIndex& rkIndex)
{
    if (Game <= EGame::Echoes)
    {
        if (rkIndex.row() == 0) return EPropertyType::Asset;
        if (rkIndex.row() == 1) return EPropertyType::Choice;
        if (rkIndex.row() == 2) return EPropertyType::Int;
    }
    else if (Game <= EGame::Corruption)
    {
        if (rkIndex.row() == 0) return EPropertyType::Asset;
        if (rkIndex.row() == 1) return EPropertyType::Int;
    }
    else
    {
        if (rkIndex.row() == 0) return EPropertyType::Asset;
        if (rkIndex.row() <= 2) return EPropertyType::Int;
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
