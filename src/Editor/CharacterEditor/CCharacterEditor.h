#ifndef CCHARACTEREDITOR_H
#define CCHARACTEREDITOR_H

#include "IEditor.h"
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

class CCharacterEditor : public IEditor
{
    Q_OBJECT

    Ui::CCharacterEditor *ui;
    CScene *mpScene;
    CCharacterNode *mpCharNode;
    CBone *mpSelectedBone;

    CSkeletonHierarchyModel mSkeletonModel;
    QComboBox *mpCharComboBox;
    QComboBox *mpAnimComboBox;

    TResPtr<CAnimSet> mpSet;
    uint32 mCurrentChar;
    uint32 mCurrentAnim;
    bool mBindPose;

    // Playback Controls
    bool mPlayAnim;
    bool mLoopAnim;
    float mAnimTime;
    float mPlaybackSpeed;

    // Constants
    static const CVector3f skDefaultOrbitTarget;
    static const float skDefaultOrbitDistance;

public:
    explicit CCharacterEditor(CAnimSet *pSet, QWidget *parent = 0);
    ~CCharacterEditor();
    void EditorTick(float DeltaTime);
    void UpdateAnimTime(float DeltaTime);
    void UpdateCameraOrbit();
    CSkeleton* CurrentSkeleton() const;
    CAnimation* CurrentAnimation() const;
    void SetActiveAnimSet(CAnimSet *pSet);
    void SetSelectedBone(CBone *pBone);
    CCharacterEditorViewport* Viewport() const;

public slots:
    void ToggleGrid(bool Enable);
    void ToggleMeshVisible(bool Visible);
    void ToggleSkeletonVisible(bool Visible);
    void ToggleBindPose(bool Enable);
    void ToggleOrbit(bool Enable);
    void RefreshViewport();
    void OnViewportHoverBoneChanged(uint32 BoneID);
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
