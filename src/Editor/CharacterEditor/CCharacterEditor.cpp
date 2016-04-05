#include "CCharacterEditor.h"
#include "ui_CCharacterEditor.h"
#include "Editor/UICommon.h"
#include <QFileDialog>

CCharacterEditor::CCharacterEditor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CCharacterEditor)
    , mpScene(new CScene())
    , mpCharNode(new CCharacterNode(mpScene, -1))
{
    ui->setupUi(this);

    ui->Viewport->SetNode(mpCharNode);

    CCamera& rCamera = ui->Viewport->Camera();
    rCamera.Snap(CVector3f(0, 3, 1));
    rCamera.SetOrbit(CVector3f(0, 0, 1), 3.f);
    rCamera.SetMoveSpeed(0.5f);

    // Init UI
    mpCharComboBox = new QComboBox(this);
    mpCharComboBox->setMinimumWidth(175);
    ui->ToolBar->addSeparator();
    ui->ToolBar->addWidget(mpCharComboBox);

    connect(&mRefreshTimer, SIGNAL(timeout()), this, SLOT(RefreshViewport()));
    mRefreshTimer.start(0);

    connect(mpCharComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SetActiveCharacterIndex(int)));
    connect(ui->ActionOpen, SIGNAL(triggered()), this, SLOT(Open()));
}

CCharacterEditor::~CCharacterEditor()
{
    delete ui;
}

// ************ PUBLIC SLOTS ************
void CCharacterEditor::Open()
{
    QString CharFilename = QFileDialog::getOpenFileName(this, "Open Character", "", "Animation Character Set (*.ANCS)");
    if (CharFilename.isEmpty()) return;

    TResPtr<CAnimSet> pSet = gResCache.GetResource(CharFilename.toStdString());

    if (pSet)
    {
        mpCharNode->SetCharacter(pSet);
        setWindowTitle("Prime World Editor - Character Editor: " + TO_QSTRING(pSet->Source()));

        // Set up character combo box
        mpCharComboBox->blockSignals(true);
        mpCharComboBox->clear();

        for (u32 iChar = 0; iChar < pSet->NumNodes(); iChar++)
            mpCharComboBox->addItem( TO_QSTRING(pSet->NodeName(iChar)) );

        SetActiveCharacterIndex(0);
        mpCharComboBox->blockSignals(false);
    }

    gResCache.Clean();
}

void CCharacterEditor::RefreshViewport()
{
    ui->Viewport->ProcessInput();
    ui->Viewport->Render();
}

void CCharacterEditor::SetActiveCharacterIndex(int CharIndex)
{
    mpCharNode->SetActiveCharSet((u32) CharIndex);
}
