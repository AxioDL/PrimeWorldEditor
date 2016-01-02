#include "WPropertyEditor.h"
#include "WDraggableSpinBox.h"
#include "WIntegralSpinBox.h"
#include "WResourceSelector.h"
#include "WColorPicker.h"
#include "WVectorEditor.h"
#include "WAnimParamsEditor.h"
#include "Editor/UICommon.h"
#include <Core/Resource/CAnimSet.h>
#include <Core/Resource/Script/CScriptLayer.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFontMetrics>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>

static const QString gskNullProperty = "[NULL]";
static const QString gskUnsupportedType = "Invalid property type";

WPropertyEditor::WPropertyEditor(QWidget *pParent, IProperty *pProperty)
    : QWidget(pParent)
{
    mUI.PropertyName = new QLabel(gskNullProperty, this);
    mUI.EditorWidget = nullptr;
    mUI.Layout = new QHBoxLayout(this);
    mUI.Layout->addWidget(mUI.PropertyName);
    mUI.Layout->setContentsMargins(0,0,0,0);
    setLayout(mUI.Layout);

    mpProperty = nullptr;
    SetProperty(pProperty);
}

WPropertyEditor::~WPropertyEditor()
{

}

void WPropertyEditor::resizeEvent(QResizeEvent* /*pEvent*/)
{
    CreateLabelText();
}

void WPropertyEditor::SetProperty(IProperty *pProperty)
{
    if (pProperty)
    {
        bool IsNewProperty = ((!mpProperty) || (pProperty->Template() != mpProperty->Template()));
        mpProperty = pProperty;

        if (IsNewProperty)
            CreateEditor();
        else
            UpdateEditor();
    }

    else
    {
        delete mUI.EditorWidget;
        mUI.EditorWidget = nullptr;

        mpProperty = pProperty;
        mUI.PropertyName->setText(gskNullProperty);
    }
}

void WPropertyEditor::CreateEditor()
{
    // Clear existing edit widget (if any)
    delete mUI.EditorWidget;

    // Set name
    mUI.PropertyName->setText(TO_QSTRING(mpProperty->Name()));
    mUI.PropertyName->setToolTip(TO_QSTRING(mpProperty->Name()));

    // Set editor widget
    switch (mpProperty->Type())
    {

    // Bool - QCheckBox
    case eBoolProperty:
    {
        TBoolProperty *pBoolCast = static_cast<TBoolProperty*>(mpProperty);
        QCheckBox *pCheckBox = new QCheckBox(this);

        pCheckBox->setChecked(pBoolCast->Get());

        mUI.EditorWidget = pCheckBox;
        break;
    }

    // Byte - WIntegralSpinBox
    case eByteProperty:
    {
        TByteProperty *pByteCast = static_cast<TByteProperty*>(mpProperty);
        QSpinBox *pSpinBox = new WIntegralSpinBox(this);

        pSpinBox->setRange(-128, 128);
        pSpinBox->setFocusPolicy(Qt::StrongFocus);
        pSpinBox->setContextMenuPolicy(Qt::NoContextMenu);
        pSpinBox->setValue(pByteCast->Get());

        mUI.EditorWidget = pSpinBox;
        break;
    }

    // Short - WIntegralSpinBox
    case eShortProperty:
    {
        TShortProperty *pShortCast = static_cast<TShortProperty*>(mpProperty);
        QSpinBox *pSpinBox = new WIntegralSpinBox(this);

        pSpinBox->setRange(-32768, 32767);
        pSpinBox->setFocusPolicy(Qt::StrongFocus);
        pSpinBox->setContextMenuPolicy(Qt::NoContextMenu);
        pSpinBox->setValue(pShortCast->Get());

        mUI.EditorWidget = pSpinBox;
        break;
    }

    // Long - WIntegralSpinBox
    case eLongProperty:
    {
        TLongProperty *pLongCast = static_cast<TLongProperty*>(mpProperty);
        QSpinBox *pSpinBox = new WIntegralSpinBox(this);

        pSpinBox->setRange(INT32_MIN, INT32_MAX);
        pSpinBox->setFocusPolicy(Qt::StrongFocus);
        pSpinBox->setContextMenuPolicy(Qt::NoContextMenu);
        pSpinBox->setValue(pLongCast->Get());

        mUI.EditorWidget = pSpinBox;
        break;
    }

    // Enum - QComboBox
    case eEnumProperty:
    {
        TEnumProperty *pEnumCast = static_cast<TEnumProperty*>(mpProperty);
        CEnumTemplate *pTemplate = static_cast<CEnumTemplate*>(pEnumCast->Template());
        QComboBox *pComboBox = new QComboBox(this);

        for (u32 iEnum = 0; iEnum < pTemplate->NumEnumerators(); iEnum++)
        {
            TString name = pTemplate->EnumeratorName(iEnum);
            pComboBox->addItem(TO_QSTRING(name));
        }

        u32 index = pEnumCast->Get();
        if (index < pTemplate->NumEnumerators()) pComboBox->setCurrentIndex(index);
        pComboBox->setFocusPolicy(Qt::StrongFocus);
        pComboBox->setContextMenuPolicy(Qt::NoContextMenu);

        mUI.EditorWidget = pComboBox;
        break;
    }

    // Bitfield - QGroupBox containing QCheckBoxes
    case eBitfieldProperty:
    {
        TBitfieldProperty *pBitfieldCast = static_cast<TBitfieldProperty*>(mpProperty);
        CBitfieldTemplate *pTemplate = static_cast<CBitfieldTemplate*>(pBitfieldCast->Template());
        long value = pBitfieldCast->Get();

        QGroupBox *pGroupBox = new QGroupBox(this);
        QVBoxLayout *pBitfieldLayout = new QVBoxLayout(pGroupBox);
        pBitfieldLayout->setContentsMargins(5,5,5,5);
        pGroupBox->setLayout(pBitfieldLayout);
        pGroupBox->setTitle(TO_QSTRING(pBitfieldCast->Name()));
        mUI.PropertyName->hide();

        for (u32 iFlag = 0; iFlag < pTemplate->NumFlags(); iFlag++)
        {
            TString flagName = pTemplate->FlagName(iFlag);
            long mask = pTemplate->FlagMask(iFlag);

            QCheckBox *pCheckBox = new QCheckBox(TO_QSTRING(flagName), pGroupBox);
            pCheckBox->setChecked((value & mask) != 0);
            pBitfieldLayout->addWidget(pCheckBox);
        }

        mUI.EditorWidget = pGroupBox;
        break;
    }

    // Float - WDraggableSpinBox
    case eFloatProperty:
    {
        TFloatProperty *pFloatCast = static_cast<TFloatProperty*>(mpProperty);
        WDraggableSpinBox *pDraggableSpinBox = new WDraggableSpinBox(this);

        pDraggableSpinBox->setDecimals(4);
        pDraggableSpinBox->setFocusPolicy(Qt::StrongFocus);
        pDraggableSpinBox->setContextMenuPolicy(Qt::NoContextMenu);
        pDraggableSpinBox->setValue(pFloatCast->Get());

        mUI.EditorWidget = pDraggableSpinBox;
        break;
    }

    // String - QLineEdit
    case eStringProperty:
    {
        TStringProperty *pStringCast = static_cast<TStringProperty*>(mpProperty);
        QLineEdit *pLineEdit = new QLineEdit(this);

        pLineEdit->setText(TO_QSTRING(pStringCast->Get()));
        pLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        //pLineEdit->setCursorPosition(0);

        mUI.EditorWidget = pLineEdit;
        break;
    }

    // Vector3 - WVectorEditor (inside QGroupBox)
    case eVector3Property:
    {
        TVector3Property *pVector3Cast = static_cast<TVector3Property*>(mpProperty);
        QGroupBox *pGroupBox = new QGroupBox(this);
        WVectorEditor *pVectorEditor = new WVectorEditor(pGroupBox);

        QVBoxLayout *pGroupLayout = new QVBoxLayout(pGroupBox);
        pGroupLayout->addWidget(pVectorEditor);
        pGroupLayout->setContentsMargins(0,0,0,0);
        pGroupBox->setLayout(pGroupLayout);

        pGroupBox->setTitle(TO_QSTRING(mpProperty->Name()));
        pVectorEditor->SetValue(pVector3Cast->Get());
        pVectorEditor->SetOrientation(Qt::Vertical);
        mUI.PropertyName->hide();

        mUI.EditorWidget = pGroupBox;
        break;
    }

    // Color - WColorPicker
    case eColorProperty:
    {
        TColorProperty *pColorCast = static_cast<TColorProperty*>(mpProperty);
        WColorPicker *pColorPicker = new WColorPicker(this);

        CColor color = pColorCast->Get();
        QColor qcolor = QColor(color.r * 255, color.g * 255, color.b * 255, color.a * 255);
        pColorPicker->setColor(qcolor);

        mUI.EditorWidget = pColorPicker;
        break;
    }

    // File - WResourceSelector
    case eFileProperty:
    {
        TFileProperty *pFileCast = static_cast<TFileProperty*>(mpProperty);
        CFileTemplate *pFileTemp = static_cast<CFileTemplate*>(pFileCast->Template());
        WResourceSelector *pResourceSelector = new WResourceSelector(this);

        pResourceSelector->AdjustPreviewToParent(true);
        pResourceSelector->SetAllowedExtensions(pFileTemp->Extensions());
        pResourceSelector->SetResource(pFileCast->Get());

        mUI.EditorWidget = pResourceSelector;
        break;
    }

    // Struct - QGroupBox
    case eStructProperty:
    {
        CPropertyStruct *pStructCast = static_cast<CPropertyStruct*>(mpProperty);

        QGroupBox *pGroupBox = new QGroupBox(this);
        QVBoxLayout *pStructLayout = new QVBoxLayout(pGroupBox);
        pStructLayout->setContentsMargins(5,5,5,5);
        pGroupBox->setLayout(pStructLayout);
        pGroupBox->setTitle(TO_QSTRING(pStructCast->Name()));
        mUI.PropertyName->hide();

        for (u32 p = 0; p < pStructCast->Count(); p++)
        {
            WPropertyEditor *pEditor = new WPropertyEditor(pGroupBox, pStructCast->PropertyByIndex(p));
            pStructLayout->addWidget(pEditor);
        }

        mUI.EditorWidget = pGroupBox;
        break;
    }

    // AnimParams - WAnimParamsEditor
    case eCharacterProperty:
    {
        TAnimParamsProperty *pAnimCast = static_cast<TAnimParamsProperty*>(mpProperty);
        CAnimationParameters params = pAnimCast->Get();

        WAnimParamsEditor *pEditor = new WAnimParamsEditor(params, this);
        pEditor->SetTitle(TO_QSTRING(pAnimCast->Name()));

        mUI.PropertyName->hide();
        mUI.EditorWidget = pEditor;
        break;
    }

    // Invalid
    case eInvalidProperty:
    default:
        mUI.EditorWidget = new QLabel(gskUnsupportedType, this);
        break;
    }

    // For some reason setting a minimum size on group boxes flattens it...
    if ((mpProperty->Type() != eStructProperty) &&
        (mpProperty->Type() != eBitfieldProperty) &&
        (mpProperty->Type() != eVector3Property) &&
        (mpProperty->Type() != eCharacterProperty))
    {
        mUI.EditorWidget->setMinimumHeight(21);
        mUI.EditorWidget->setMaximumHeight(21);
    }

    mUI.Layout->addWidget(mUI.EditorWidget, 0);
    CreateLabelText();
}

void WPropertyEditor::UpdateEditor()
{
    switch (mpProperty->Type())
    {

    case eBoolProperty:
    {
        TBoolProperty *pBoolCast = static_cast<TBoolProperty*>(mpProperty);
        QCheckBox *pCheckBox = static_cast<QCheckBox*>(mUI.EditorWidget);
        pCheckBox->setChecked(pBoolCast->Get());
        break;
    }

    case eByteProperty:
    {
        TByteProperty *pByteCast = static_cast<TByteProperty*>(mpProperty);
        QSpinBox *pSpinBox = static_cast<QSpinBox*>(mUI.EditorWidget);
        pSpinBox->setValue(pByteCast->Get());
        break;
    }

    case eShortProperty:
    {
        TShortProperty *pShortCast = static_cast<TShortProperty*>(mpProperty);
        QSpinBox *pSpinBox = static_cast<QSpinBox*>(mUI.EditorWidget);
        pSpinBox->setValue(pShortCast->Get());
        break;
    }

    case eLongProperty:
    {
        TLongProperty *pLongCast = static_cast<TLongProperty*>(mpProperty);
        QSpinBox *pSpinBox = static_cast<QSpinBox*>(mUI.EditorWidget);
        pSpinBox->setValue(pLongCast->Get());
        break;
    }

    case eEnumProperty:
    {
        TEnumProperty *pEnumCast = static_cast<TEnumProperty*>(mpProperty);
        QComboBox *pComboBox = static_cast<QComboBox*>(mUI.EditorWidget);
        pComboBox->setCurrentIndex(pEnumCast->Get());
        break;
    }

    case eBitfieldProperty:
    {
        TBitfieldProperty *pBitfieldCast = static_cast<TBitfieldProperty*>(mpProperty);
        CBitfieldTemplate *pTemplate = static_cast<CBitfieldTemplate*>(pBitfieldCast->Template());
        QGroupBox *pGroupBox = static_cast<QGroupBox*>(mUI.EditorWidget);

        QObjectList ChildList = pGroupBox->children();
        long value = pBitfieldCast->Get();
        u32 propNum = 0;

        foreach (QObject *pObj, ChildList)
        {
            if (pObj != pGroupBox->layout())
            {
                u32 mask = pTemplate->FlagMask(propNum);
                QCheckBox *pCheckBox = static_cast<QCheckBox*>(pObj);
                pCheckBox->setChecked((value & mask) != 0);
                propNum++;
            }
        }
        break;
    }

    case eFloatProperty:
    {
        TFloatProperty *pFloatCast = static_cast<TFloatProperty*>(mpProperty);
        WDraggableSpinBox *pDraggableSpinBox = static_cast<WDraggableSpinBox*>(mUI.EditorWidget);
        pDraggableSpinBox->setValue(pFloatCast->Get());
        break;
    }

    case eStringProperty:
    {
        TStringProperty *pStringCast = static_cast<TStringProperty*>(mpProperty);
        QLineEdit *pLineEdit = static_cast<QLineEdit*>(mUI.EditorWidget);
        pLineEdit->setText(TO_QSTRING(pStringCast->Get()));
        pLineEdit->setCursorPosition(0);
        break;
    }

    case eVector3Property:
    {
        TVector3Property *pVector3Cast = static_cast<TVector3Property*>(mpProperty);
        QGroupBox *pGroupBox = static_cast<QGroupBox*>(mUI.EditorWidget);

        WVectorEditor *pVectorEditor = static_cast<WVectorEditor*>(pGroupBox->children().first());
        pVectorEditor->SetValue(pVector3Cast->Get());
        break;
    }

    case eColorProperty:
    {
        TColorProperty *pColorCast = static_cast<TColorProperty*>(mpProperty);
        WColorPicker *pColorPicker = static_cast<WColorPicker*>(mUI.EditorWidget);

        CColor color = pColorCast->Get();
        QColor qcolor = QColor(color.r * 255, color.g * 255, color.b * 255, color.a * 255);
        pColorPicker->setColor(qcolor);

        break;
    }

    case eFileProperty:
    {
        TFileProperty *pFileCast = static_cast<TFileProperty*>(mpProperty);
        CFileTemplate *pFileTemp = static_cast<CFileTemplate*>(pFileCast->Template());
        WResourceSelector *pResourceSelector = static_cast<WResourceSelector*>(mUI.EditorWidget);
        pResourceSelector->SetAllowedExtensions(pFileTemp->Extensions());
        pResourceSelector->SetResource(pFileCast->Get());
        break;
    }

    case eStructProperty:
    {
        CPropertyStruct *pStructCast = static_cast<CPropertyStruct*>(mpProperty);
        QGroupBox *pGroupBox = static_cast<QGroupBox*>(mUI.EditorWidget);

        QObjectList ChildList = pGroupBox->children();
        u32 PropNum = 0;

        foreach (QObject *pObj, ChildList)
        {
            if (pObj != pGroupBox->layout())
            {
                IProperty *pProp = pStructCast->PropertyByIndex(PropNum);
                static_cast<WPropertyEditor*>(pObj)->SetProperty(pProp);
                PropNum++;
            }
        }
        break;
    }

    case eCharacterProperty:
    {
        TAnimParamsProperty *pAnimCast = static_cast<TAnimParamsProperty*>(mpProperty);
        WAnimParamsEditor *pEditor = static_cast<WAnimParamsEditor*>(mUI.EditorWidget);
        pEditor->SetParameters(pAnimCast->Get());
        break;
    }

    }

}

void WPropertyEditor::CreateLabelText()
{
    mUI.PropertyName->setText(TO_QSTRING(mpProperty->Name()));
    QFontMetrics metrics(mUI.PropertyName->font());
    QString text = metrics.elidedText(TO_QSTRING(mpProperty->Name()), Qt::ElideRight, mUI.PropertyName->width());
    mUI.PropertyName->setText(text);
}
