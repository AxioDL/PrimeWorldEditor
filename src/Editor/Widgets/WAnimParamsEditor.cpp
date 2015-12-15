#include "WAnimParamsEditor.h"
#include "Editor/UICommon.h"
#include <Core/Resource/CAnimSet.h>
#include <Core/Resource/CResCache.h>

WAnimParamsEditor::WAnimParamsEditor(QWidget *pParent)
    : QWidget(pParent),
      mpGroupBox(new QGroupBox("Animation Parameters", this)),
      mpGroupLayout(nullptr),
      mpSelector(nullptr),
      mpCharComboBox(nullptr)
{
    for (u32 iBox = 0; iBox < 4; iBox++)
        mpSpinBoxes[iBox] = nullptr;

    for (u32 iLabel = 0; iLabel < 5; iLabel++) {
        mpLabels[iLabel] = nullptr;
        mpValueLayouts[iLabel] = nullptr;
    }

    QVBoxLayout *pLayout = new QVBoxLayout(this);
    pLayout->addWidget(mpGroupBox);
    pLayout->setContentsMargins(0,0,0,0);
    setLayout(pLayout);
}

WAnimParamsEditor::WAnimParamsEditor(const CAnimationParameters& params, QWidget *pParent)
    : QWidget(pParent),
      mpGroupBox(new QGroupBox("Animation Parameters", this)),
      mpGroupLayout(nullptr),
      mpSelector(nullptr),
      mpCharComboBox(nullptr)
{
    for (u32 iBox = 0; iBox < 4; iBox++)
        mpSpinBoxes[iBox] = nullptr;

    for (u32 iLabel = 0; iLabel < 5; iLabel++) {
        mpLabels[iLabel] = nullptr;
        mpValueLayouts[iLabel] = nullptr;
    }

    QVBoxLayout *pLayout = new QVBoxLayout(this);
    pLayout->addWidget(mpGroupBox);
    pLayout->setContentsMargins(0,0,0,0);
    setLayout(pLayout);

    mParams = params;
    SetupUI();
}

WAnimParamsEditor::~WAnimParamsEditor()
{
}

void WAnimParamsEditor::SetTitle(const QString& title)
{
    mpGroupBox->setTitle(title);
}

void WAnimParamsEditor::SetParameters(const CAnimationParameters& params)
{
    mParams = params;
    SetupUI();
}

// ************ PRIVATE SLOTS ************
void WAnimParamsEditor::OnResourceChanged(QString path)
{
    CResource *pRes = gResCache.GetResource(path.toStdString());
    if (pRes->Type() != eAnimSet) pRes = nullptr;

    mParams.SetResource(pRes);
    emit ParametersChanged(mParams);
}

void WAnimParamsEditor::OnCharacterChanged(int index)
{
    mParams.SetNodeIndex(index);
    emit ParametersChanged(mParams);
}

void WAnimParamsEditor::OnUnknownChanged()
{
    for (u32 iBox = 0; iBox < 4; iBox++)
        if (mpSpinBoxes[iBox])
            mParams.SetUnknown(iBox, mpSpinBoxes[iBox]->value());
    emit ParametersChanged(mParams);
}

// ************ PRIVATE ************
void WAnimParamsEditor::SetupUI()
{
    // Clean existing layout
    if (!mLayoutWidgets.isEmpty())
    {
        foreach (QObject *pObject, mLayoutWidgets)
            delete pObject;

        mpSelector = nullptr;
        mpCharComboBox = nullptr;

        for (u32 iBox = 0; iBox < 4; iBox++)
            mpSpinBoxes[iBox] = nullptr;

        for (u32 iLabel = 0; iLabel < 5; iLabel++) {
            mpLabels[iLabel] = nullptr;
            mpValueLayouts[iLabel] = nullptr;
        }

        mLayoutWidgets.clear();
    }

    delete mpGroupLayout;
    mpGroupLayout = new QVBoxLayout(mpGroupBox);
    mpGroupLayout->setContentsMargins(5,5,5,5);
    mpGroupBox->setLayout(mpGroupLayout);

    // Create resource selector
    mpLabels[0] = new QLabel(mParams.Version() <= eEchoes ? "AnimSet" : "Character");
    mpSelector = new WResourceSelector(this);
    mpSelector->SetAllowedExtensions(mParams.Version() <= eEchoes ? "ANCS" : "CHAR");
    mpSelector->AdjustPreviewToParent(true);
    mpSelector->SetResource(mParams.Resource());

    mpValueLayouts[0] = new QHBoxLayout(this);
    mpValueLayouts[0]->addWidget(mpLabels[0], 0);
    mpValueLayouts[0]->addWidget(mpSelector, 1);
    mpValueLayouts[0]->setSpacing(5);
    mpGroupLayout->addLayout(mpValueLayouts[0]);

    mLayoutWidgets << mpLabels[0] << mpSelector << mpValueLayouts[0];
    connect(mpSelector, SIGNAL(ResourceChanged(QString)), this, SLOT(OnResourceChanged(QString)));

    // Create MP1/2 widgets
    if (mParams.Version() <= eEchoes)
    {
        // Create character select combo box
        mpCharComboBox = new QComboBox(this);
        CAnimSet *pSet = static_cast<CAnimSet*>(mParams.Resource());

        if (pSet)
            for (u32 iChar = 0; iChar < pSet->getNodeCount(); iChar++)
                mpCharComboBox->addItem(TO_QSTRING(pSet->getNodeName(iChar)));

        mpCharComboBox->setCurrentIndex(mParams.CharacterIndex());
        mpLabels[1] = new QLabel("Character", this);

        // Create unknown spin box
        mpSpinBoxes[0] = new WIntegralSpinBox(this);
        mpSpinBoxes[0]->setRange(INT32_MIN, INT32_MAX);
        mpSpinBoxes[0]->setFocusPolicy(Qt::StrongFocus);
        mpSpinBoxes[0]->setContextMenuPolicy(Qt::NoContextMenu);
        mpSpinBoxes[0]->setValue(mParams.Unknown(0));
        mpLabels[2] = new QLabel("Unknown", this);

        // Create layouts
        mpValueLayouts[1] = new QHBoxLayout(this);
        mpValueLayouts[1]->addWidget(mpLabels[1], 0);
        mpValueLayouts[1]->addWidget(mpCharComboBox, 1);
        mpValueLayouts[1]->setSpacing(5);

        mpValueLayouts[2] = new QHBoxLayout(this);
        mpValueLayouts[2]->addWidget(mpLabels[2], 0);
        mpValueLayouts[2]->addWidget(mpSpinBoxes[0], 1);
        mpValueLayouts[2]->setSpacing(5);

        mpGroupLayout->addLayout(mpValueLayouts[1]);
        mpGroupLayout->addLayout(mpValueLayouts[2]);

        // Finish UI
        mLayoutWidgets << mpLabels[1] << mpCharComboBox << mpLabels[2] << mpSpinBoxes[0] << mpValueLayouts[1] << mpValueLayouts[2];
        connect(mpCharComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCharacterChanged(int)));
        connect(mpSpinBoxes[0], SIGNAL(valueChanged(int)), this, SLOT(OnUnknownChanged()));
    }

    // Create MP3 widgets
    else if (mParams.Version() <= eCorruption)
    {
        // Create unknown spin box
        mpSpinBoxes[0] = new WIntegralSpinBox(this);
        mpSpinBoxes[0]->setRange(INT32_MIN, INT32_MAX);
        mpSpinBoxes[0]->setFocusPolicy(Qt::StrongFocus);
        mpSpinBoxes[0]->setContextMenuPolicy(Qt::NoContextMenu);
        mpSpinBoxes[0]->setValue(mParams.Unknown(0));
        mpLabels[1] = new QLabel("Unknown", this);

        // Create layouts
        mpValueLayouts[1] = new QHBoxLayout(this);
        mpValueLayouts[1]->addWidget(mpLabels[1], 0);
        mpValueLayouts[1]->addWidget(mpSpinBoxes[0], 1);
        mpValueLayouts[1]->setSpacing(5);
        mpGroupLayout->addLayout(mpValueLayouts[1]);

        // Finish UI
        mLayoutWidgets << mpLabels[1] << mpSpinBoxes[0] << mpValueLayouts[1];
        connect(mpSpinBoxes[0], SIGNAL(valueChanged(int)), this, SLOT(OnUnknownChanged()));
    }

    // Create DKCR widgets
    else if (mParams.Version() == eReturns)
    {
        // Create unknown spin box A
        mpSpinBoxes[0] = new WIntegralSpinBox(this);
        mpSpinBoxes[0]->setRange(0, 255);
        mpSpinBoxes[0]->setFocusPolicy(Qt::StrongFocus);
        mpSpinBoxes[0]->setContextMenuPolicy(Qt::NoContextMenu);
        mpSpinBoxes[0]->setValue(mParams.Unknown(0));
        mpLabels[1] = new QLabel("Unknown", this);

        mpValueLayouts[1] = new QHBoxLayout(this);
        mpValueLayouts[1]->addWidget(mpLabels[1], 0);
        mpValueLayouts[1]->addWidget(mpSpinBoxes[0], 1);
        mpValueLayouts[1]->setSpacing(5);
        mpGroupLayout->addLayout(mpValueLayouts[1]);

        mLayoutWidgets << mpLabels[1] << mpSpinBoxes[0] << mpValueLayouts[1];
        connect(mpSpinBoxes[0], SIGNAL(valueChanged(int)), this, SLOT(OnUnknownChanged()));

        // Create unknown spin box B/C/D
        for (u32 iBox = 1; iBox < 4; iBox++)
        {
            mpSpinBoxes[iBox] = new WIntegralSpinBox(this);
            mpSpinBoxes[iBox]->setRange(INT32_MIN, INT32_MAX);
            mpSpinBoxes[iBox]->setFocusPolicy(Qt::StrongFocus);
            mpSpinBoxes[iBox]->setContextMenuPolicy(Qt::NoContextMenu);
            mpSpinBoxes[iBox]->setValue(mParams.Unknown(iBox));
            mpLabels[iBox+1] = new QLabel("Unknown", this);

            mpValueLayouts[iBox+1] = new QHBoxLayout(this);
            mpValueLayouts[iBox+1]->addWidget(mpLabels[iBox+1], 0);
            mpValueLayouts[iBox+1]->addWidget(mpSpinBoxes[iBox], 1);
            mpValueLayouts[iBox+1]->setSpacing(5);
            mpGroupLayout->addLayout(mpValueLayouts[iBox+1]);

            mLayoutWidgets << mpLabels[iBox+1] << mpSpinBoxes[iBox] << mpValueLayouts[iBox+1];
            connect(mpSpinBoxes[iBox], SIGNAL(valueChanged(int)), this, SLOT(OnUnknownChanged()));
        }
    }
}
