#include "CCharacterEditor.h"
#include "ui_CCharacterEditor.h"
#include "Editor/UICommon.h"
#include <Common/Macros.h>
#include <Common/Math/MathUtil.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QTreeView>

const CVector3f CCharacterEditor::skDefaultOrbitTarget = CVector3f(0,0,1);
const float CCharacterEditor::skDefaultOrbitDistance = 4.f;

CCharacterEditor::CCharacterEditor(CAnimSet *pSet, QWidget *parent)
    : IEditor(parent)
    , ui(new Ui::CCharacterEditor)
    , mpScene(new CScene())
    , mpSelectedBone(nullptr)
    , mBindPose(false)
    , mPlayAnim(true)
    , mLoopAnim(true)
    , mAnimTime(0.f)
    , mPlaybackSpeed(1.f)
{
    ui->setupUi(this);
    REPLACE_WINDOWTITLE_APPVARS;

    mpCharNode = new CCharacterNode(mpScene, -1);
    ui->Viewport->SetNode(mpCharNode);

    CCamera& rCamera = ui->Viewport->Camera();
    rCamera.SetMoveSpeed(0.5f);
    rCamera.SetPitch(-0.3f);
    rCamera.SetMoveMode(ECameraMoveMode::Orbit);

    // Init UI
    ui->ToolBar->addSeparator();

    mpCharComboBox = new QComboBox(this);
    mpCharComboBox->setMinimumWidth(175);
    ui->ToolBar->addWidget(mpCharComboBox);

    mpAnimComboBox = new QComboBox(this);
    mpAnimComboBox->setMinimumWidth(175);
    ui->ToolBar->addWidget(mpAnimComboBox);

    connect(ui->Viewport, SIGNAL(HoverBoneChanged(uint32)), this, SLOT(OnViewportHoverBoneChanged(uint32)));
    connect(ui->Viewport, SIGNAL(ViewportClick(QMouseEvent*)), this, SLOT(OnViewportClick()));
    connect(ui->ActionShowGrid, SIGNAL(toggled(bool)), this, SLOT(ToggleGrid(bool)));
    connect(ui->ActionShowMesh, SIGNAL(toggled(bool)), this, SLOT(ToggleMeshVisible(bool)));
    connect(ui->ActionShowSkeleton, SIGNAL(toggled(bool)), this, SLOT(ToggleSkeletonVisible(bool)));
    connect(ui->ActionBindPose, SIGNAL(toggled(bool)), this, SLOT(ToggleBindPose(bool)));
    connect(ui->ActionOrbit, SIGNAL(toggled(bool)), this, SLOT(ToggleOrbit(bool)));
    connect(ui->ActionPlay, SIGNAL(triggered()), this, SLOT(TogglePlay()));
    connect(ui->ActionLoop, SIGNAL(toggled(bool)), this, SLOT(ToggleLoop(bool)));
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

    SetActiveAnimSet(pSet);
}

CCharacterEditor::~CCharacterEditor()
{
    delete ui;
    delete mpScene;
    delete mpCharNode;
}

void CCharacterEditor::EditorTick(float DeltaTime)
{
    UpdateAnimTime(DeltaTime);
    UpdateCameraOrbit();
}

void CCharacterEditor::UpdateAnimTime(float DeltaTime)
{
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
    return mpSet ? mpSet->Character(mCurrentChar)->pSkeleton : nullptr;
}

CAnimation* CCharacterEditor::CurrentAnimation() const
{
    return mpSet ? mpSet->FindAnimationAsset(mCurrentAnim) : nullptr;
}

void CCharacterEditor::SetActiveAnimSet(CAnimSet *pSet)
{
    mpSet = pSet;
    mpCharNode->SetCharSet(mpSet);
    SET_WINDOWTITLE_APPVARS("%APP_FULL_NAME% - Character Editor: " + TO_QSTRING(mpSet->Source()));

    // Clear selected bone
    ui->SkeletonHierarchyTreeView->selectionModel()->clear();
    SetSelectedBone(nullptr);

    // Set up character combo box
    mpCharComboBox->blockSignals(true);
    mpCharComboBox->clear();

    for (uint32 iChar = 0; iChar < mpSet->NumCharacters(); iChar++)
        mpCharComboBox->addItem( TO_QSTRING(mpSet->Character(iChar)->Name) );

    SetActiveCharacterIndex(0);
    mpCharComboBox->blockSignals(false);

    // Set up anim combo box
    mpAnimComboBox->blockSignals(true);
    mpAnimComboBox->clear();

    for (uint32 iAnim = 0; iAnim < mpSet->NumAnimations(); iAnim++)
        mpAnimComboBox->addItem( TO_QSTRING(mpSet->Animation(iAnim)->Name) );

    SetActiveAnimation(0);
    mpAnimComboBox->blockSignals(false);

    // Set up skeleton tree view
    CSkeleton *pSkel = mpSet->Character(mCurrentChar)->pSkeleton;
    mSkeletonModel.SetSkeleton(pSkel);
    ui->SkeletonHierarchyTreeView->expandAll();
    ui->SkeletonHierarchyTreeView->resizeColumnToContents(0);

    // Select first child bone of root (which should be Skeleton_Root) to line up the camera for orbiting.
    QModelIndex RootIndex = mSkeletonModel.index(0, 0, QModelIndex());
    ui->SkeletonHierarchyTreeView->selectionModel()->setCurrentIndex( mSkeletonModel.index(0, 0, RootIndex), QItemSelectionModel::ClearAndSelect );

    // Run CCamera::SetOrbit to reset orbit distance.
    ui->Viewport->Camera().SetOrbit(mpCharNode->AABox());
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

CCharacterEditorViewport* CCharacterEditor::Viewport() const
{
    return ui->Viewport;
}

// ************ PUBLIC SLOTS ************
void CCharacterEditor::ToggleGrid(bool Enable)
{
    ui->Viewport->SetGridEnabled(Enable);
}

void CCharacterEditor::ToggleMeshVisible(bool Visible)
{
    // eShowObjectGeometry isn't the best fit, but close enough...?
    ui->Viewport->SetShowFlag(EShowFlag::ObjectGeometry, Visible);
}

void CCharacterEditor::ToggleSkeletonVisible(bool Visible)
{
    ui->Viewport->SetShowFlag(EShowFlag::Skeletons, Visible);
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
        TogglePlay();

    ui->AnimSlider->setEnabled(!Enable);
    mBindPose = Enable;
}

void CCharacterEditor::ToggleOrbit(bool Enable)
{
    ui->Viewport->Camera().SetMoveMode(Enable ? ECameraMoveMode::Orbit : ECameraMoveMode::Free);
}

void CCharacterEditor::RefreshViewport()
{
    ui->Viewport->ProcessInput();
    ui->Viewport->Render();
}

void CCharacterEditor::OnViewportHoverBoneChanged(uint32 BoneID)
{
    if (BoneID == 0xFFFFFFFF)
        ui->StatusBar->clearMessage();
    else
        ui->StatusBar->showMessage(QString("Bone %1: %2").arg(BoneID).arg( TO_QSTRING(mpSet->Character(mCurrentChar)->pSkeleton->BoneByID(BoneID)->Name()) ));
}

void CCharacterEditor::OnViewportClick()
{
    uint32 HoverBoneID = ui->Viewport->HoverBoneID();
    CSkeleton *pSkel = (mpSet ? mpSet->Character(mCurrentChar)->pSkeleton : nullptr);
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
    mpCharNode->SetActiveChar((uint32) CharIndex);
}

void CCharacterEditor::SetActiveAnimation(int AnimIndex)
{
    mCurrentAnim = AnimIndex;
    mpCharNode->SetActiveAnim((uint32) AnimIndex);

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
    uint32 MaxAnim = (mpSet ? mpSet->NumAnimations() - 1 : 0);
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

    CAnimation *pAnim = CurrentAnimation();
    uint32 NumKeys = 1, CurKey = 0;

    if (pAnim)
    {
        NumKeys = pAnim->NumKeys();
        CurKey = Math::Min<uint32>((uint32) (Time / pAnim->TickInterval()) + 1, NumKeys - 1);
    }

    ui->FrameLabel->setText(QString("Frame %1 / %2 (%3s/%4s)").arg(CurKey).arg(NumKeys - 1).arg(mAnimTime, 0, 'f', 3).arg(pAnim ? pAnim->Duration() : 0.f, 0, 'f', 3));
}

void CCharacterEditor::TogglePlay()
{
    if (mBindPose) ToggleBindPose(false);

    mPlayAnim = !mPlayAnim;
    QString NewText = (mPlayAnim ? "Pause" : "Play");
    ui->PlayPauseButton->setToolTip(NewText);
    ui->ActionPlay->setText(NewText);

    QIcon PlayPauseIcon = QIcon(mPlayAnim ? ":/icons/Pause_24px.png" : ":/icons/Play_24px.png");
    ui->PlayPauseButton->setIcon(PlayPauseIcon);

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

    QString NewText = (Loop ? "Disable Loop" : "Loop");
    ui->LoopButton->setToolTip(NewText);
    ui->ActionLoop->setText(NewText);

    QIcon ActionIcon = QIcon(Loop ? ":/icons/DontLoop_24px" : ":/icons/Loop_24px.png");
    ui->ActionLoop->setIcon(ActionIcon);

    if (sender() != ui->LoopButton)
    {
        ui->LoopButton->blockSignals(true);
        ui->LoopButton->setChecked(Loop);
        ui->LoopButton->blockSignals(false);
    }

    if (sender() != ui->ActionLoop)
    {
        ui->LoopButton->blockSignals(true);
        ui->ActionLoop->setChecked(Loop);
        ui->LoopButton->blockSignals(false);
    }
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
