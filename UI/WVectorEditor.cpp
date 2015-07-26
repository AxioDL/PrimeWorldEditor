#include "WVectorEditor.h"

WVectorEditor::WVectorEditor(QWidget *pParent) : QWidget(pParent)
{
    mValue = CVector3f::skZero;

    mpSpinBoxX = new WDraggableSpinBox(this);
    mpSpinBoxY = new WDraggableSpinBox(this);
    mpSpinBoxZ = new WDraggableSpinBox(this);
    connect(mpSpinBoxX, SIGNAL(valueChanged(double)), this, SLOT(SetX(double)));
    connect(mpSpinBoxY, SIGNAL(valueChanged(double)), this, SLOT(SetY(double)));
    connect(mpSpinBoxZ, SIGNAL(valueChanged(double)), this, SLOT(SetZ(double)));

    /*mpLayout = new QHBoxLayout(this);
    mpLayout->setContentsMargins(0,0,0,0);
    mpLayout->addWidget(mpSpinBoxX);
    mpLayout->addWidget(mpSpinBoxY);
    mpLayout->addWidget(mpSpinBoxZ);
    setLayout(mpLayout);*/

    mpGroupBox = new QGroupBox(this);
    mpFormLayout = new QFormLayout(mpGroupBox);
    mpFormLayout->addRow(new QLabel("X", mpGroupBox), mpSpinBoxX);
    mpFormLayout->addRow(new QLabel("Y", mpGroupBox), mpSpinBoxY);
    mpFormLayout->addRow(new QLabel("Z", mpGroupBox), mpSpinBoxZ);
    mpGroupBox->setLayout(mpFormLayout);

    mpLayout = new QHBoxLayout(this);
    mpLayout->addWidget(mpGroupBox);
    setLayout(mpLayout);
}

WVectorEditor::WVectorEditor(const CVector3f& Value, QWidget *pParent) : QWidget(pParent)
{
    mValue = Value;

    mpSpinBoxX = new WDraggableSpinBox(this);
    mpSpinBoxY = new WDraggableSpinBox(this);
    mpSpinBoxZ = new WDraggableSpinBox(this);
    mpSpinBoxX->setValue((double) Value.x);
    mpSpinBoxY->setValue((double) Value.y);
    mpSpinBoxZ->setValue((double) Value.z);
    mpSpinBoxX->setMinimumHeight(21);
    mpSpinBoxY->setMinimumHeight(21);
    mpSpinBoxZ->setMinimumHeight(21);
    mpSpinBoxX->setMaximumHeight(21);
    mpSpinBoxY->setMaximumHeight(21);
    mpSpinBoxZ->setMaximumHeight(21);
    connect(mpSpinBoxX, SIGNAL(valueChanged(double)), this, SLOT(SetX(double)));
    connect(mpSpinBoxY, SIGNAL(valueChanged(double)), this, SLOT(SetY(double)));
    connect(mpSpinBoxZ, SIGNAL(valueChanged(double)), this, SLOT(SetZ(double)));

    mpLayout = new QHBoxLayout(this);
    mpLayout->setContentsMargins(0,0,0,0);
    mpLayout->addWidget(mpSpinBoxX);
    mpLayout->addWidget(mpSpinBoxY);
    mpLayout->addWidget(mpSpinBoxZ);
    setLayout(mpLayout);
}

WVectorEditor::~WVectorEditor()
{
}


CVector3f WVectorEditor::Value()
{
    return mValue;
}

void WVectorEditor::SetValue(const CVector3f& Value)
{
    mValue = Value;

    mpSpinBoxX->blockSignals(true);
    mpSpinBoxY->blockSignals(true);
    mpSpinBoxZ->blockSignals(true);
    mpSpinBoxX->setValue((double) Value.x);
    mpSpinBoxY->setValue((double) Value.y);
    mpSpinBoxZ->setValue((double) Value.z);
    mpSpinBoxX->blockSignals(false);
    mpSpinBoxY->blockSignals(false);
    mpSpinBoxZ->blockSignals(false);
}

void WVectorEditor::SetText(const QString &Text)
{
    mpGroupBox->setTitle(Text);
}

// ************ SLOTS ************
void WVectorEditor::SetX(double x)
{
    mValue.x = (float) x;

    mpSpinBoxX->blockSignals(true);
    mpSpinBoxX->setValue((double) x);
    mpSpinBoxX->blockSignals(false);
}

void WVectorEditor::SetY(double y)
{
    mValue.y = (float) y;

    mpSpinBoxY->blockSignals(true);
    mpSpinBoxY->setValue((double) y);
    mpSpinBoxY->blockSignals(false);
}

void WVectorEditor::SetZ(double z)
{
    mValue.z = (float) z;

    mpSpinBoxZ->blockSignals(true);
    mpSpinBoxZ->setValue((double) z);
    mpSpinBoxZ->blockSignals(false);
}
