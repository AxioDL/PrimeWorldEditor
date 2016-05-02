#ifndef CCHARACTEREDITOR_H
#define CCHARACTEREDITOR_H

#include "CCharacterEditorViewport.h"
#include "CSkeletonHierarchyModel.h"
#include <Core/Scene/CScene.h>
#include <Core/Scene/CCharacterNode.h>

#include <QComboBox>
#include <QMainWindow>
#include <QTimer>

namespace Ui {
class CCharacterEditor;
}

class CCharacterEditor : public QMainWindow
{
    Q_OBJECT

    Ui::CCharacterEditor *ui;
    CScene *mpScene;
    CCharacterNode *mpCharNode;
    CBone *mpSelectedBone;

    CSkeletonHierarchyModel mSkeletonModel;
    QComboBox *mpCharComboBox;
    QComboBox *mpAnimComboBox;
    QTimer mRefreshTimer;

    TResPtr<CAnimSet> mpSet;
    u32 mCurrentChar;
    u32 mCurrentAnim;
    bool mBindPose;

    // Playback Controls
    double mLastAnimUpdate;
    float mAnimTime;
    bool mPlayAnim;
    bool mLoopAnim;
    float mPlaybackSpeed;

    // Constants
    static const CVector3f skDefaultOrbitTarget;
    static const float skDefaultOrbitDistance;

public:
    explicit CCharacterEditor(QWidget *parent = 0);
    ~CCharacterEditor();
    void UpdateAnimTime();
    void UpdateCameraOrbit();
    CSkeleton* CurrentSkeleton() const;
    CAnimation* CurrentAnimation() const;
    void SetSelectedBone(CBone *pBone);

public slots:
    void Open();
    void ToggleGrid(bool Enable);
    void ToggleMeshVisible(bool Visible);
    void ToggleSkeletonVisible(bool Visible);
    void ToggleBindPose(bool Enable);
    void ToggleOrbit(bool Enable);
    void RefreshViewport();
    void OnViewportHoverBoneChanged(u32 BoneID);
    void OnViewportClick();
    void OnSkeletonTreeSelectionChanged(const QModelIndex& rkIndex);
    void SetActiveCharacterIndex(int CharIndex);
    void SetActiveAnimation(int AnimIndex);
    void PrevAnim();
    void NextAnim();

    void SetAnimTime(int Time);
    void SetAnimTime(float Time);
    void TogglePlay();
    void ToggleLoop(bool Loop);
    void Rewind();
    void FastForward();
    void AnimSpeedSpinBoxChanged(double NewVal);
};

#endif // CCHARACTEREDITORWINDOW_H
