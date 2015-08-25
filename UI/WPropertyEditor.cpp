#include "WPropertyEditor.h"
#include "WDraggableSpinBox.h"
#include "WIntegralSpinBox.h"
#include "WResourceSelector.h"
#include "WColorPicker.h"
#include "WVectorEditor.h"
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QFontMetrics>

static const QString gskNullProperty = "[NULL]";
static const QString gskUnsupportedType = "Invalid property type";

WPropertyEditor::WPropertyEditor(QWidget *pParent, CPropertyBase *pProperty)
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

void WPropertyEditor::resizeEvent(QResizeEvent *pEvent)
{
    CreateLabelText();
}

void WPropertyEditor::SetProperty(CPropertyBase *pProperty)
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
    mUI.PropertyName->setText(QString::fromStdString(mpProperty->Name()));
    mUI.PropertyName->setToolTip(QString::fromStdString(mpProperty->Name()));

    // Set editor widget
    switch (mpProperty->Type())
    {

    // Bool - QCheckBox
    case eBoolProperty:
    {
        CBoolProperty *pBoolCast = static_cast<CBoolProperty*>(mpProperty);
        QCheckBox *pCheckBox = new QCheckBox(this);

        pCheckBox->setChecked(pBoolCast->Get());

        mUI.EditorWidget = pCheckBox;
        break;
    }

    // Byte - WIntegralSpinBox
    case eByteProperty:
    {
        CByteProperty *pByteCast = static_cast<CByteProperty*>(mpProperty);
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
        CShortProperty *pShortCast = static_cast<CShortProperty*>(mpProperty);
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
        CLongProperty *pLongCast = static_cast<CLongProperty*>(mpProperty);
        QSpinBox *pSpinBox = new WIntegralSpinBox(this);

        pSpinBox->setRange(-2147483648, 2147483647);
        pSpinBox->setFocusPolicy(Qt::StrongFocus);
        pSpinBox->setContextMenuPolicy(Qt::NoContextMenu);
        pSpinBox->setValue(pLongCast->Get());

        mUI.EditorWidget = pSpinBox;
        break;
    }

    // Float - WDraggableSpinBox
    case eFloatProperty:
    {
        CFloatProperty *pFloatCast = static_cast<CFloatProperty*>(mpProperty);
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
        CStringProperty *pStringCast = static_cast<CStringProperty*>(mpProperty);
        QLineEdit *pLineEdit = new QLineEdit(this);

        pLineEdit->setText(QString::fromStdString(pStringCast->Get()));
        pLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        pLineEdit->setCursorPosition(0);

        mUI.EditorWidget = pLineEdit;
        break;
    }

    // Vector3 - WVectorEditor (inside QGroupBox)
    case eVector3Property:
    {
        CVector3Property *pVector3Cast = static_cast<CVector3Property*>(mpProperty);
        QGroupBox *pGroupBox = new QGroupBox(this);
        WVectorEditor *pVectorEditor = new WVectorEditor(pGroupBox);

        QVBoxLayout *pGroupLayout = new QVBoxLayout(pGroupBox);
        pGroupLayout->addWidget(pVectorEditor);
        pGroupLayout->setContentsMargins(0,0,0,0);
        pGroupBox->setLayout(pGroupLayout);

        pGroupBox->setTitle(QString::fromStdString(mpProperty->Name()));
        pVectorEditor->SetValue(pVector3Cast->Get());
        pVectorEditor->SetOrientation(Qt::Vertical);
        mUI.PropertyName->hide();

        mUI.EditorWidget = pGroupBox;
        break;
    }

    // Color - WColorPicker
    case eColorProperty:
    {
        CColorProperty *pColorCast = static_cast<CColorProperty*>(mpProperty);
        WColorPicker *pColorPicker = new WColorPicker(this);

        CColor color = pColorCast->Get();
        QColor qcolor = QColor(color.r, color.g, color.b, color.a);
        pColorPicker->setColor(qcolor);

        mUI.EditorWidget = pColorPicker;
        break;
    }

    // Enum - todo (will be QComboBox)
    case eEnumProperty:
        mUI.EditorWidget = new QLabel("[placeholder]", this);
        break;

    // File - WResourceSelector
    case eFileProperty:
    {
        CFileProperty *pFileCast = static_cast<CFileProperty*>(mpProperty);
        WResourceSelector *pResourceSelector = new WResourceSelector(this);

        pResourceSelector->AdjustPreviewToParent(true);
        pResourceSelector->SetAllowedExtensions(pFileCast->AllowedExtensions());
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
        pGroupBox->setLayout(pStructLayout);
        pGroupBox->setTitle(QString::fromStdString(pStructCast->Name()));
        mUI.PropertyName->hide();

        for (u32 p = 0; p < pStructCast->Count(); p++)
        {
            WPropertyEditor *pEditor = new WPropertyEditor(pGroupBox, pStructCast->PropertyByIndex(p));
            pStructLayout->addWidget(pEditor);
        }

        mUI.EditorWidget = pGroupBox;
        break;
    }

    // Invalid
    case eInvalidProperty:
    default:
        mUI.EditorWidget = new QLabel(gskUnsupportedType, this);
        break;
    }

    // For some reason setting a minimum size on group boxes flattens it...
    if ((mpProperty->Type() != eStructProperty) && (mpProperty->Type() != eVector3Property))
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
        CBoolProperty *pBoolCast = static_cast<CBoolProperty*>(mpProperty);
        QCheckBox *pCheckBox = static_cast<QCheckBox*>(mUI.EditorWidget);
        pCheckBox->setChecked(pBoolCast->Get());
        break;
    }

    case eByteProperty:
    {
        CByteProperty *pByteCast = static_cast<CByteProperty*>(mpProperty);
        QSpinBox *pSpinBox = static_cast<QSpinBox*>(mUI.EditorWidget);
        pSpinBox->setValue(pByteCast->Get());
        break;
    }

    case eShortProperty:
    {
        CShortProperty *pShortCast = static_cast<CShortProperty*>(mpProperty);
        QSpinBox *pSpinBox = static_cast<QSpinBox*>(mUI.EditorWidget);
        pSpinBox->setValue(pShortCast->Get());
        break;
    }

    case eLongProperty:
    {
        CLongProperty *pLongCast = static_cast<CLongProperty*>(mpProperty);
        QSpinBox *pSpinBox = static_cast<QSpinBox*>(mUI.EditorWidget);
        pSpinBox->setValue(pLongCast->Get());
        break;
    }

    case eFloatProperty:
    {
        CFloatProperty *pFloatCast = static_cast<CFloatProperty*>(mpProperty);
        WDraggableSpinBox *pDraggableSpinBox = static_cast<WDraggableSpinBox*>(mUI.EditorWidget);
        pDraggableSpinBox->setValue(pFloatCast->Get());
        break;
    }

    case eStringProperty:
    {
        CStringProperty *pStringCast = static_cast<CStringProperty*>(mpProperty);
        QLineEdit *pLineEdit = static_cast<QLineEdit*>(mUI.EditorWidget);
        pLineEdit->setText(QString::fromStdString(pStringCast->Get()));
        pLineEdit->setCursorPosition(0);
        break;
    }

    case eVector3Property:
    {
        CVector3Property *pVector3Cast = static_cast<CVector3Property*>(mpProperty);
        QGroupBox *pGroupBox = static_cast<QGroupBox*>(mUI.EditorWidget);

        WVectorEditor *pVectorEditor = static_cast<WVectorEditor*>(pGroupBox->children().first());
        pVectorEditor->SetValue(pVector3Cast->Get());
        break;
    }

    case eColorProperty:
    {
        CColorProperty *pColorCast = static_cast<CColorProperty*>(mpProperty);
        WColorPicker *pColorPicker = static_cast<WColorPicker*>(mUI.EditorWidget);

        CColor color = pColorCast->Get();
        QColor qcolor = QColor(color.r, color.g, color.b, color.a);
        pColorPicker->setColor(qcolor);

        break;
    }

    case eEnumProperty:
        break;

    case eFileProperty:
    {
        CFileProperty *pFileCast = static_cast<CFileProperty*>(mpProperty);
        WResourceSelector *pResourceSelector = static_cast<WResourceSelector*>(mUI.EditorWidget);
        pResourceSelector->SetAllowedExtensions(pFileCast->AllowedExtensions());
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
                CPropertyBase *pProp = pStructCast->PropertyByIndex(PropNum);
                static_cast<WPropertyEditor*>(pObj)->SetProperty(pProp);
                PropNum++;
            }
        }
        break;
    }

    }

}

void WPropertyEditor::CreateLabelText()
{
    mUI.PropertyName->setText(QString::fromStdString(mpProperty->Name()));
    QFontMetrics metrics(mUI.PropertyName->font());
    QString text = metrics.elidedText(QString::fromStdString(mpProperty->Name()), Qt::ElideRight, mUI.PropertyName->width());
    mUI.PropertyName->setText(text);
}
