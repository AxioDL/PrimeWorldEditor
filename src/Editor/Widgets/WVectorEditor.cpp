#include "WVectorEditor.h"

WVectorEditor::WVectorEditor(QWidget *pParent) : QWidget(pParent)
{
    SetupUI();
    mValue = CVector3f::skZero;
    mpSpinBoxX->setValue(0.0);
    mpSpinBoxY->setValue(0.0);
    mpSpinBoxZ->setValue(0.0);
}

WVectorEditor::WVectorEditor(const CVector3f& value, QWidget *pParent) : QWidget(pParent)
{
    SetupUI();
    mValue = value;
    mpSpinBoxX->setValue((double) value.x);
    mpSpinBoxY->setValue((double) value.y);
    mpSpinBoxZ->setValue((double) value.z);
}

WVectorEditor::~WVectorEditor()
{
    delete mpLayout;
}

CVector3f WVectorEditor::Value()
{
    return mValue;
}

void WVectorEditor::SetValue(const CVector3f& value)
{
    mValue = value;

    mpSpinBoxX->blockSignals(true);
    mpSpinBoxY->blockSignals(true);
    mpSpinBoxZ->blockSignals(true);
    mpSpinBoxX->setValue((double) value.x);
    mpSpinBoxY->setValue((double) value.y);
    mpSpinBoxZ->setValue((double) value.z);
    mpSpinBoxX->blockSignals(false);
    mpSpinBoxY->blockSignals(false);
    mpSpinBoxZ->blockSignals(false);
}

void WVectorEditor::SetOrientation(Qt::Orientation orientation)
{
    mOrientation = orientation;

    if (mpLayout)
    {
        mpLayout->removeItem(mpXLayout);
        mpLayout->removeItem(mpYLayout);
        mpLayout->removeItem(mpZLayout);
        delete mpLayout;
    }

    mpLayout = (orientation == Qt::Horizontal ? (QLayout*) new QHBoxLayout : (QLayout*) new QVBoxLayout);
    mpLayout->addItem(mpXLayout);
    mpLayout->addItem(mpYLayout);
    mpLayout->addItem(mpZLayout);
    mpLayout->setContentsMargins(5,5,5,5);
    setLayout(mpLayout);
}

void WVectorEditor::SetDefaultValue(double value)
{
    mpSpinBoxX->SetDefaultValue(value);
    mpSpinBoxY->SetDefaultValue(value);
    mpSpinBoxZ->SetDefaultValue(value);
}

void WVectorEditor::SetSingleStep(double step)
{
    mpSpinBoxX->setSingleStep(step);
    mpSpinBoxY->setSingleStep(step);
    mpSpinBoxZ->setSingleStep(step);
}

void WVectorEditor::SetLabelsHidden(bool hidden)
{
    if (hidden)
    {
        mpLabelX->hide();
        mpLabelY->hide();
        mpLabelZ->hide();
    }

    else
    {
        mpLabelX->show();
        mpLabelY->show();
        mpLabelZ->show();
    }
}

bool WVectorEditor::IsBeingDragged()
{
    return (mpSpinBoxX->IsBeingDragged() || mpSpinBoxY->IsBeingDragged() || mpSpinBoxZ->IsBeingDragged());
}

// ************ PUBLIC SLOTS ************
void WVectorEditor::SetX(double x)
{
    mValue.x = (float) x;

    mpSpinBoxX->blockSignals(true);
    mpSpinBoxX->setValue((double) x);
    mpSpinBoxX->blockSignals(false);

    emit ValueChanged(mValue);
}

void WVectorEditor::SetY(double y)
{
    mValue.y = (float) y;

    mpSpinBoxY->blockSignals(true);
    mpSpinBoxY->setValue((double) y);
    mpSpinBoxY->blockSignals(false);

    emit ValueChanged(mValue);
}

void WVectorEditor::SetZ(double z)
{
    mValue.z = (float) z;

    mpSpinBoxZ->blockSignals(true);
    mpSpinBoxZ->setValue((double) z);
    mpSpinBoxZ->blockSignals(false);

    emit ValueChanged(mValue);
}

// ************ PRIVATE ************
void WVectorEditor::SetupUI()
{
    // Create and initialize widgets
    mpLabelX = new QLabel("X", this);
    mpLabelY = new QLabel("Y", this);
    mpLabelZ = new QLabel("Z", this);
    mpSpinBoxX = new WDraggableSpinBox(this);
    mpSpinBoxY = new WDraggableSpinBox(this);
    mpSpinBoxZ = new WDraggableSpinBox(this);
    mpSpinBoxX->setDecimals(4);
    mpSpinBoxY->setDecimals(4);
    mpSpinBoxZ->setDecimals(4);
    mpSpinBoxX->setFocusPolicy(Qt::StrongFocus);
    mpSpinBoxY->setFocusPolicy(Qt::StrongFocus);
    mpSpinBoxZ->setFocusPolicy(Qt::StrongFocus);
    mpSpinBoxX->setContextMenuPolicy(Qt::NoContextMenu);
    mpSpinBoxY->setContextMenuPolicy(Qt::NoContextMenu);
    mpSpinBoxZ->setContextMenuPolicy(Qt::NoContextMenu);
    connect(mpSpinBoxX, SIGNAL(valueChanged(double)), this, SLOT(SetX(double)));
    connect(mpSpinBoxY, SIGNAL(valueChanged(double)), this, SLOT(SetY(double)));
    connect(mpSpinBoxZ, SIGNAL(valueChanged(double)), this, SLOT(SetZ(double)));
    connect(mpSpinBoxX, SIGNAL(editingFinished()), this, SLOT(OnSpinBoxEditingDone()));
    connect(mpSpinBoxY, SIGNAL(editingFinished()), this, SLOT(OnSpinBoxEditingDone()));
    connect(mpSpinBoxZ, SIGNAL(editingFinished()), this, SLOT(OnSpinBoxEditingDone()));

    // Create and initialize spinbox layouts
    mpXLayout = new QHBoxLayout();
    mpYLayout = new QHBoxLayout();
    mpZLayout = new QHBoxLayout();
    mpXLayout->addWidget(mpLabelX, 0);
    mpXLayout->addWidget(mpSpinBoxX, 1);
    mpXLayout->setSpacing(5);
    mpYLayout->addWidget(mpLabelY, 0);
    mpYLayout->addWidget(mpSpinBoxY, 1);
    mpYLayout->setSpacing(5);
    mpZLayout->addWidget(mpLabelZ, 0);
    mpZLayout->addWidget(mpSpinBoxZ, 1);
    mpZLayout->setSpacing(5);

    // Create and initialize widget layout
    mpLayout = nullptr;
    SetOrientation(Qt::Vertical);
}

// ************ PRIVATE SLOTS ************
void WVectorEditor::OnSpinBoxEditingDone()
{
    emit EditingDone(mValue);
}
