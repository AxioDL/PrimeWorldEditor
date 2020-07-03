#include "WVectorEditor.h"

WVectorEditor::WVectorEditor(QWidget *pParent)
    : QWidget(pParent)
    , mValue(CVector3f::Zero())
    , mEditing(false)
{
    SetupUI();
    mpSpinBoxX->setValue(0.0);
    mpSpinBoxY->setValue(0.0);
    mpSpinBoxZ->setValue(0.0);
}

WVectorEditor::WVectorEditor(const CVector3f& rkValue, QWidget *pParent)
    : QWidget(pParent)
    , mValue(rkValue)
    , mEditing(false)
{
    SetupUI();
    mValue = rkValue;
    mpSpinBoxX->setValue((double) rkValue.X);
    mpSpinBoxY->setValue((double) rkValue.Y);
    mpSpinBoxZ->setValue((double) rkValue.Z);
    mEditing = false;
}

WVectorEditor::~WVectorEditor()
{
    delete mpLayout;
}

CVector3f WVectorEditor::Value() const
{
    return mValue;
}

void WVectorEditor::SetValue(const CVector3f& rkValue)
{
    mValue = rkValue;

    mpSpinBoxX->blockSignals(true);
    mpSpinBoxY->blockSignals(true);
    mpSpinBoxZ->blockSignals(true);
    mpSpinBoxX->setValue((double) rkValue.X);
    mpSpinBoxY->setValue((double) rkValue.Y);
    mpSpinBoxZ->setValue((double) rkValue.Z);
    mpSpinBoxX->blockSignals(false);
    mpSpinBoxY->blockSignals(false);
    mpSpinBoxZ->blockSignals(false);
}

void WVectorEditor::SetOrientation(Qt::Orientation Orientation)
{
    mOrientation = Orientation;

    if (mpLayout)
    {
        mpLayout->removeItem(mpXLayout);
        mpLayout->removeItem(mpYLayout);
        mpLayout->removeItem(mpZLayout);
        delete mpLayout;
    }

    mpLayout = (Orientation == Qt::Horizontal ? (QLayout*) new QHBoxLayout : (QLayout*) new QVBoxLayout);
    mpLayout->addItem(mpXLayout);
    mpLayout->addItem(mpYLayout);
    mpLayout->addItem(mpZLayout);
    mpLayout->setContentsMargins(5,5,5,5);
    setLayout(mpLayout);
}

void WVectorEditor::SetDefaultValue(double Value)
{
    mpSpinBoxX->SetDefaultValue(Value);
    mpSpinBoxY->SetDefaultValue(Value);
    mpSpinBoxZ->SetDefaultValue(Value);
}

void WVectorEditor::SetSingleStep(double Step)
{
    mpSpinBoxX->setSingleStep(Step);
    mpSpinBoxY->setSingleStep(Step);
    mpSpinBoxZ->setSingleStep(Step);
}

void WVectorEditor::SetLabelsHidden(bool Hidden)
{
    if (Hidden)
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

bool WVectorEditor::IsBeingDragged() const
{
    return (mpSpinBoxX->IsBeingDragged() || mpSpinBoxY->IsBeingDragged() || mpSpinBoxZ->IsBeingDragged());
}

bool WVectorEditor::IsBeingEdited() const
{
    return IsBeingDragged() || mEditing;
}

// ************ PUBLIC SLOTS ************
void WVectorEditor::SetX(double X)
{
    mValue.X = (float) X;

    if (sender() != mpSpinBoxX)
    {
        mpSpinBoxX->blockSignals(true);
        mpSpinBoxX->setValue((double) X);
        mpSpinBoxX->blockSignals(false);
    }

    mEditing = true;
    emit ValueChanged(mValue);
}

void WVectorEditor::SetY(double Y)
{
    mValue.Y = (float) Y;

    if (sender() != mpSpinBoxY)
    {
        mpSpinBoxY->blockSignals(true);
        mpSpinBoxY->setValue((double) Y);
        mpSpinBoxY->blockSignals(false);
    }

    mEditing = true;
    emit ValueChanged(mValue);
}

void WVectorEditor::SetZ(double Z)
{
    mValue.Z = (float) Z;

    if (sender() != mpSpinBoxZ)
    {
        mpSpinBoxZ->blockSignals(true);
        mpSpinBoxZ->setValue((double) Z);
        mpSpinBoxZ->blockSignals(false);
    }

    mEditing = true;
    emit ValueChanged(mValue);
}

// ************ PRIVATE ************
void WVectorEditor::SetupUI()
{
    // Create and initialize widgets
    mpLabelX = new QLabel(tr("X"), this);
    mpLabelY = new QLabel(tr("Y"), this);
    mpLabelZ = new QLabel(tr("Z"), this);
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
    connect(mpSpinBoxX, qOverload<double>(&WDraggableSpinBox::valueChanged), this, &WVectorEditor::SetX);
    connect(mpSpinBoxY, qOverload<double>(&WDraggableSpinBox::valueChanged), this, &WVectorEditor::SetY);
    connect(mpSpinBoxZ, qOverload<double>(&WDraggableSpinBox::valueChanged), this, &WVectorEditor::SetZ);
    connect(mpSpinBoxX, &WDraggableSpinBox::editingFinished, this, &WVectorEditor::OnSpinBoxEditingDone);
    connect(mpSpinBoxY, &WDraggableSpinBox::editingFinished, this, &WVectorEditor::OnSpinBoxEditingDone);
    connect(mpSpinBoxZ, &WDraggableSpinBox::editingFinished, this, &WVectorEditor::OnSpinBoxEditingDone);

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

    setTabOrder(mpSpinBoxX, mpSpinBoxY);
    setTabOrder(mpSpinBoxY, mpSpinBoxZ);

    // Create and initialize widget layout
    mpLayout = nullptr;
    SetOrientation(Qt::Vertical);
}

// ************ PRIVATE SLOTS ************
void WVectorEditor::OnSpinBoxEditingDone()
{
    if (mEditing) emit EditingDone(mValue);
    mEditing = false;
}
