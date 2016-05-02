#include "CCharacterEditor.h"
#include "ui_CCharacterEditor.h"
#include "Editor/UICommon.h"
#include <Math/MathUtil.h>
#include <QFileDialog>
#include <QTreeView>

CCharacterEditor::CCharacterEditor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CCharacterEditor)
    , mpScene(new CScene())
    , mpCharNode(new CCharacterNode(mpScene, -1))
    , mAnimTime(0.f)
    , mPlayAnim(true)
    , mLoopAnim(true)
    , mPlaybackSpeed(1.f)
{
    ui->setupUi(this);

    ui->Viewport->SetNode(mpCharNode);

    CCamera& rCamera = ui->Viewport->Camera();
    rCamera.Snap(CVector3f(0, 3, 1));
    rCamera.SetOrbit(CVector3f(0, 0, 1), 3.f);
    rCamera.SetMoveSpeed(0.5f);

    // Init UI
    ui->ToolBar->addSeparator();

    mpCharComboBox = new QComboBox(this);
    mpCharComboBox->setMinimumWidth(175);
    ui->ToolBar->addWidget(mpCharComboBox);

    mpAnimComboBox = new QComboBox(this);
    mpAnimComboBox->setMinimumWidth(175);
    ui->ToolBar->addWidget(mpAnimComboBox);

    connect(&mRefreshTimer, SIGNAL(timeout()), this, SLOT(RefreshViewport()));
    mRefreshTimer.start(0);

    connect(ui->Viewport, SIGNAL(HoverBoneChanged(u32)), this, SLOT(HoverBoneChanged(u32)));
    connect(ui->ActionOpen, SIGNAL(triggered()), this, SLOT(Open()));
    connect(ui->ActionShowSkeleton, SIGNAL(toggled(bool)), this, SLOT(ToggleSkeletonVisible(bool)));
    connect(mpCharComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SetActiveCharacterIndex(int)));
    connect(mpAnimComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SetActiveAnimation(int)));

    connect(ui->AnimSlider, SIGNAL(valueChanged(int)), this, SLOT(SetAnimTime(int)));
    connect(ui->PlayPauseButton, SIGNAL(pressed()), this, SLOT(TogglePlay()));
    connect(ui->LoopButton, SIGNAL(toggled(bool)), this, SLOT(ToggleLoop(bool)));
    connect(ui->AnimSpeedSpinBox, SIGNAL(valueChanged(double)), this, SLOT(AnimSpeedSpinBoxChanged(double)));

    // Init skeleton tree view
    ui->SkeletonHierarchyTreeView->setModel(&mSkeletonModel);
    QList<int> SplitterSizes;
    SplitterSizes << width() * 0.2 << width() * 0.8;
    ui->splitter->setSizes(SplitterSizes);
}

CCharacterEditor::~CCharacterEditor()
{
    delete ui;
}

void CCharacterEditor::UpdateAnimTime()
{
    double Time = CTimer::GlobalTime();
    double DeltaTime = Time - mLastAnimUpdate;
    mLastAnimUpdate = Time;

    CAnimation *pAnim = CurrentAnimation();

    if (pAnim && mPlayAnim && !ui->AnimSlider->isSliderDown())
    {
        mAnimTime += DeltaTime * mPlaybackSpeed;

        CAnimation *pAnim = CurrentAnimation();
        float AnimLength = (pAnim ? pAnim->Duration() : 0.f);

        if (mAnimTime > AnimLength)
        {
            if (mLoopAnim)
            {
                mAnimTime = fmodf(mAnimTime, AnimLength);
            }
            else
            {
                mAnimTime = AnimLength;
                TogglePlay();
            }
        }

        if (mAnimTime < 0.f)
        {
            if (mLoopAnim)
            {
                mAnimTime = AnimLength + fmodf(mAnimTime, AnimLength);
            }
            else
            {
                mAnimTime = 0.f;
                TogglePlay();
            }
        }

        SetAnimTime(mAnimTime);
    }
}

CAnimation* CCharacterEditor::CurrentAnimation() const
{
    if (mpSet)
        return mpSet->Animation(mCurrentAnim);
    else
        return nullptr;
}

// ************ PUBLIC SLOTS ************
void CCharacterEditor::Open()
{
    QString CharFilename = QFileDialog::getOpenFileName(this, "Open Character", "", "Animation Character Set (*.ANCS)");
    if (CharFilename.isEmpty()) return;

    mpSet = gResCache.GetResource(CharFilename.toStdString());

    if (mpSet)
    {
        mpCharNode->SetCharSet(mpSet);
        setWindowTitle("Prime World Editor - Character Editor: " + TO_QSTRING(mpSet->Source()));

        // Set up character combo box
        mpCharComboBox->blockSignals(true);
        mpCharComboBox->clear();

        for (u32 iChar = 0; iChar < mpSet->NumNodes(); iChar++)
            mpCharComboBox->addItem( TO_QSTRING(mpSet->NodeName(iChar)) );

        SetActiveCharacterIndex(0);
        mpCharComboBox->blockSignals(false);

        // Set up anim combo box
        mpAnimComboBox->blockSignals(true);
        mpAnimComboBox->clear();

        for (u32 iAnim = 0; iAnim < mpSet->NumAnims(); iAnim++)
            mpAnimComboBox->addItem( TO_QSTRING(mpSet->AnimName(iAnim)) );

        SetActiveAnimation(0);
        mpAnimComboBox->blockSignals(false);

        // Set up skeleton tree view
        CSkeleton *pSkel = mpSet->NodeSkeleton(mCurrentChar);
        mSkeletonModel.SetSkeleton(pSkel);
        ui->SkeletonHierarchyTreeView->expandAll();
        ui->SkeletonHierarchyTreeView->resizeColumnToContents(0);
    }

    gResCache.Clean();
}

void CCharacterEditor::ToggleSkeletonVisible(bool Visible)
{
    ui->Viewport->SetShowFlag(eShowSkeletons, Visible);
}

void CCharacterEditor::RefreshViewport()
{
    UpdateAnimTime();
    ui->Viewport->ProcessInput();
    ui->Viewport->Render();
}

void CCharacterEditor::HoverBoneChanged(u32 BoneID)
{
    if (BoneID == 0xFFFFFFFF)
        ui->StatusBar->clearMessage();
    else
        ui->StatusBar->showMessage(QString("Bone %1: %2").arg(BoneID).arg( TO_QSTRING(mpSet->NodeSkeleton(mCurrentChar)->BoneByID(BoneID)->Name()) ));
}

void CCharacterEditor::SetActiveCharacterIndex(int CharIndex)
{
    mCurrentChar = CharIndex;
    mpCharNode->SetActiveChar((u32) CharIndex);
}

void CCharacterEditor::SetActiveAnimation(int AnimIndex)
{
    mCurrentAnim = AnimIndex;
    mpCharNode->SetActiveAnim((u32) AnimIndex);
    mLastAnimUpdate = CTimer::GlobalTime();

    ui->AnimSlider->blockSignals(true);
    ui->AnimSlider->setMaximum((int) (CurrentAnimation() ? CurrentAnimation()->Duration() * 1000 : 0));
    ui->AnimSlider->blockSignals(false);

    SetAnimTime(0.f);
}

void CCharacterEditor::SetAnimTime(int Time)
{
    float FloatTime = Time / 1000.f;
    SetAnimTime(FloatTime);
}

void CCharacterEditor::SetAnimTime(float Time)
{
    mAnimTime = Time;

    if (ui->AnimSlider != sender())
    {
        int IntTime = (int) (Time * 1000);
        ui->AnimSlider->setValue(IntTime);
    }

    mpCharNode->SetAnimTime(Time);

    CAnimation *pAnim = (mpSet ? mpSet->Animation(mCurrentAnim) : nullptr);
    u32 NumKeys = 1, CurKey = 0;

    if (pAnim)
    {
        NumKeys = pAnim->NumKeys();
        CurKey = Math::Min<u32>((u32) (Time / pAnim->TickInterval()) + 1, NumKeys - 1);
    }

    ui->FrameLabel->setText(QString("Frame %1 / %2 (%3s/%4s)").arg(CurKey).arg(NumKeys - 1).arg(mAnimTime, 0, 'f', 3).arg(pAnim ? pAnim->Duration() : 0.f, 0, 'f', 3));
}

void CCharacterEditor::TogglePlay()
{
    mPlayAnim = !mPlayAnim;
    QString NewText = (mPlayAnim ? "Pause" : "Play");
    ui->PlayPauseButton->setText(NewText);

    CAnimation *pAnim = CurrentAnimation();

    if (pAnim && mPlayAnim)
    {
        if (mPlaybackSpeed < 0.f && mAnimTime == 0.f)
            SetAnimTime(pAnim->Duration());
        if (mPlaybackSpeed >= 0.f && mAnimTime == pAnim->Duration())
            SetAnimTime(0.f);
    }
}

void CCharacterEditor::ToggleLoop(bool Loop)
{
    mLoopAnim = Loop;

    if (sender() != ui->LoopButton)
        ui->LoopButton->setChecked(Loop);
}

void CCharacterEditor::AnimSpeedSpinBoxChanged(double NewVal)
{
    mPlaybackSpeed = (float) NewVal;
}
