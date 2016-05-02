#include "CCharacterEditor.h"
#include "ui_CCharacterEditor.h"
#include "Editor/UICommon.h"
#include <Common/Assert.h>
#include <Math/MathUtil.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QTreeView>

const CVector3f CCharacterEditor::skDefaultOrbitTarget = CVector3f(0,0,1);
const float CCharacterEditor::skDefaultOrbitDistance = 4.f;

CCharacterEditor::CCharacterEditor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CCharacterEditor)
    , mpScene(new CScene())
    , mpCharNode(new CCharacterNode(mpScene, -1))
    , mpSelectedBone(nullptr)
    , mBindPose(false)
    , mAnimTime(0.f)
    , mPlayAnim(true)
    , mLoopAnim(true)
    , mPlaybackSpeed(1.f)
{
    ui->setupUi(this);

    ui->Viewport->SetNode(mpCharNode);

    CCamera& rCamera = ui->Viewport->Camera();
    rCamera.SetMoveSpeed(0.5f);
    rCamera.SetPitch(-0.3f);
    rCamera.SetMoveMode(eOrbitCamera);

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

    connect(ui->Viewport, SIGNAL(HoverBoneChanged(u32)), this, SLOT(OnViewportHoverBoneChanged(u32)));
    connect(ui->Viewport, SIGNAL(ViewportClick(QMouseEvent*)), this, SLOT(OnViewportClick()));
    connect(ui->ActionOpen, SIGNAL(triggered()), this, SLOT(Open()));
    connect(ui->ActionShowGrid, SIGNAL(toggled(bool)), this, SLOT(ToggleGrid(bool)));
    connect(ui->ActionShowMesh, SIGNAL(toggled(bool)), this, SLOT(ToggleMeshVisible(bool)));
    connect(ui->ActionShowSkeleton, SIGNAL(toggled(bool)), this, SLOT(ToggleSkeletonVisible(bool)));
    connect(ui->ActionBindPose, SIGNAL(toggled(bool)), this, SLOT(ToggleBindPose(bool)));
    connect(ui->ActionOrbit, SIGNAL(toggled(bool)), this, SLOT(ToggleOrbit(bool)));
    connect(ui->ActionPlay, SIGNAL(triggered()), this, SLOT(TogglePlay()));
    connect(ui->ActionRewind, SIGNAL(triggered()), this, SLOT(Rewind()));
    connect(ui->ActionFastForward, SIGNAL(triggered()), this, SLOT(FastForward()));
    connect(ui->ActionPrevAnim, SIGNAL(triggered()), this, SLOT(PrevAnim()));
    connect(ui->ActionNextAnim, SIGNAL(triggered()), this, SLOT(NextAnim()));
    connect(mpCharComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SetActiveCharacterIndex(int)));
    connect(mpAnimComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SetActiveAnimation(int)));

    connect(ui->AnimSlider, SIGNAL(valueChanged(int)), this, SLOT(SetAnimTime(int)));
    connect(ui->PlayPauseButton, SIGNAL(pressed()), this, SLOT(TogglePlay()));
    connect(ui->LoopButton, SIGNAL(toggled(bool)), this, SLOT(ToggleLoop(bool)));
    connect(ui->RewindButton, SIGNAL(pressed()), this, SLOT(Rewind()));
    connect(ui->FastForwardButton, SIGNAL(pressed()), this, SLOT(FastForward()));
    connect(ui->AnimSpeedSpinBox, SIGNAL(valueChanged(double)), this, SLOT(AnimSpeedSpinBoxChanged(double)));

    // Init skeleton tree view
    ui->SkeletonHierarchyTreeView->setModel(&mSkeletonModel);
    QList<int> SplitterSizes;
    SplitterSizes << width() * 0.2 << width() * 0.8;
    ui->splitter->setSizes(SplitterSizes);

    connect(ui->SkeletonHierarchyTreeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(OnSkeletonTreeSelectionChanged(QModelIndex)));
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

    if (pAnim && mPlayAnim && !mBindPose && !ui->AnimSlider->isSliderDown())
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

void CCharacterEditor::UpdateCameraOrbit()
{
    CSkeleton *pSkel = CurrentSkeleton();

    if (!pSkel)
    {
        // Center around character if we have one, otherwise fall back to default orbit.
        if (mpSet)
            ui->Viewport->Camera().SetOrbitTarget(mpCharNode->CenterPoint());
        else
            ui->Viewport->Camera().SetOrbit(skDefaultOrbitTarget, skDefaultOrbitDistance);
    }

    else
    {
        // If we have a selected bone, orbit around that.
        if (mpSelectedBone)
            ui->Viewport->Camera().SetOrbitTarget(mpCharNode->BonePosition(mpSelectedBone->ID()));

        // Otherwise, try to find Skeleton_Root. Barring that, we can orbit the root bone.
        else
        {
            CBone *pRoot = pSkel->RootBone();
            CBone *pSkelRoot = (pRoot ? pRoot->ChildByIndex(0) : pRoot);
            CVector3f OrbitTarget = (pSkelRoot ? mpCharNode->BonePosition(pSkelRoot->ID()) : mpCharNode->CenterPoint());
            ui->Viewport->Camera().SetOrbitTarget(OrbitTarget);
        }
    }
}

CSkeleton* CCharacterEditor::CurrentSkeleton() const
{
    if (mpSet)
        return mpSet->NodeSkeleton(mCurrentChar);
    else
        return nullptr;
}

CAnimation* CCharacterEditor::CurrentAnimation() const
{
    if (mpSet)
        return mpSet->Animation(mCurrentAnim);
    else
        return nullptr;
}

void CCharacterEditor::SetSelectedBone(CBone *pBone)
{
    if (pBone != mpSelectedBone)
    {
        if (mpSelectedBone) mpSelectedBone->SetSelected(false);
        mpSelectedBone = pBone;
        if (mpSelectedBone) mpSelectedBone->SetSelected(true);
    }
}

// ************ PUBLIC SLOTS ************
void CCharacterEditor::Open()
{
    QString CharFilename = QFileDialog::getOpenFileName(this, "Open Character", "", "Animation Character Set (*.ANCS)");
    if (CharFilename.isEmpty()) return;

    CAnimSet *pSet = (CAnimSet*) gResCache.GetResource(CharFilename.toStdString());

    if (pSet)
    {
        mpSet = pSet;
        mpCharNode->SetCharSet(mpSet);
        setWindowTitle("Prime World Editor - Character Editor: " + TO_QSTRING(mpSet->Source()));

        // Clear selected bone
        ui->SkeletonHierarchyTreeView->selectionModel()->clear();
        SetSelectedBone(nullptr);

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

        // Select first child bone of root (which should be Skeleton_Root) to line up the camera for orbiting.
        QModelIndex RootIndex = mSkeletonModel.index(0, 0, QModelIndex());
        ui->SkeletonHierarchyTreeView->selectionModel()->setCurrentIndex( mSkeletonModel.index(0, 0, RootIndex), QItemSelectionModel::ClearAndSelect );

        // Run CCamera::SetOrbit to reset orbit distance.
        ui->Viewport->Camera().SetOrbit(mpCharNode->AABox(), 4.f);
    }

    else
    {
        QMessageBox::warning(this, "Error", "Couldn't load file: " + CharFilename);
    }

    gResCache.Clean();
}

void CCharacterEditor::ToggleGrid(bool Enable)
{
    ui->Viewport->SetGridEnabled(Enable);
}

void CCharacterEditor::ToggleMeshVisible(bool Visible)
{
    // eShowObjectGeometry isn't the best fit, but close enough...?
    ui->Viewport->SetShowFlag(eShowObjectGeometry, Visible);
}

void CCharacterEditor::ToggleSkeletonVisible(bool Visible)
{
    ui->Viewport->SetShowFlag(eShowSkeletons, Visible);
}

void CCharacterEditor::ToggleBindPose(bool Enable)
{
    mpCharNode->SetAnimated(!Enable);

    if (sender() != ui->ActionBindPose)
    {
        ui->ActionBindPose->blockSignals(true);
        ui->ActionBindPose->setChecked(Enable);
        ui->ActionBindPose->blockSignals(false);
    }

    if (Enable && mPlayAnim)
    {
        SetAnimTime(0.f);
    }

    ui->AnimSlider->setEnabled(!Enable);
    mBindPose = Enable;
}

void CCharacterEditor::ToggleOrbit(bool Enable)
{
    ui->Viewport->Camera().SetMoveMode(Enable ? eOrbitCamera : eFreeCamera);
}

void CCharacterEditor::RefreshViewport()
{
    UpdateAnimTime();
    UpdateCameraOrbit();
    ui->Viewport->ProcessInput();
    ui->Viewport->Render();
}

void CCharacterEditor::OnViewportHoverBoneChanged(u32 BoneID)
{
    if (BoneID == 0xFFFFFFFF)
        ui->StatusBar->clearMessage();
    else
        ui->StatusBar->showMessage(QString("Bone %1: %2").arg(BoneID).arg( TO_QSTRING(mpSet->NodeSkeleton(mCurrentChar)->BoneByID(BoneID)->Name()) ));
}

void CCharacterEditor::OnViewportClick()
{
    u32 HoverBoneID = ui->Viewport->HoverBoneID();
    CSkeleton *pSkel = (mpSet ? mpSet->NodeSkeleton(mCurrentChar) : nullptr);
    CBone *pBone = (pSkel ? pSkel->BoneByID(HoverBoneID) : nullptr);

    if (!pBone || !pBone->IsSelected())
    {
        if (pBone)
        {
            QModelIndex NewBoneIndex = mSkeletonModel.IndexForBone(pBone);
            ui->SkeletonHierarchyTreeView->selectionModel()->setCurrentIndex(NewBoneIndex, QItemSelectionModel::ClearAndSelect);
        }
        else
            ui->SkeletonHierarchyTreeView->selectionModel()->clear();

        SetSelectedBone(pBone);
    }
}

void CCharacterEditor::OnSkeletonTreeSelectionChanged(const QModelIndex& rkIndex)
{
    CBone *pBone = mSkeletonModel.BoneForIndex(rkIndex);
    SetSelectedBone(pBone);
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

    mpAnimComboBox->blockSignals(true);
    mpAnimComboBox->setCurrentIndex(AnimIndex);
    mpAnimComboBox->blockSignals(false);

    SetAnimTime(0.f);
}

void CCharacterEditor::PrevAnim()
{
    if (mCurrentAnim > 0) SetActiveAnimation(mCurrentAnim - 1);
}

void CCharacterEditor::NextAnim()
{
    u32 MaxAnim = (mpSet ? mpSet->NumAnims() - 1 : 0);
    if (mCurrentAnim < MaxAnim) SetActiveAnimation(mCurrentAnim + 1);
}

void CCharacterEditor::SetAnimTime(int Time)
{
    float FloatTime = Time / 1000.f;
    SetAnimTime(FloatTime);
}

void CCharacterEditor::SetAnimTime(float Time)
{
    if (mBindPose) Time = 0.f;
    mAnimTime = Time;

    if (ui->AnimSlider != sender() || mBindPose)
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
    if (mBindPose) ToggleBindPose(false);

    mPlayAnim = !mPlayAnim;
    QString NewText = (mPlayAnim ? "Pause" : "Play");
    ui->PlayPauseButton->setText(NewText);

    if (ui->ActionPlay != sender())
    {
        ui->ActionPlay->blockSignals(true);
        ui->ActionPlay->setChecked(mPlayAnim);
        ui->ActionPlay->blockSignals(false);
    }

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

void CCharacterEditor::Rewind()
{
    SetAnimTime(0.f);
}

void CCharacterEditor::FastForward()
{
    CAnimation *pAnim = CurrentAnimation();
    if (pAnim && !mBindPose) SetAnimTime(pAnim->Duration());
}

void CCharacterEditor::AnimSpeedSpinBoxChanged(double NewVal)
{
    mPlaybackSpeed = (float) NewVal;
}
