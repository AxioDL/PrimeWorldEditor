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

    CSkeletonHierarchyModel mSkeletonModel;
    QComboBox *mpCharComboBox;
    QComboBox *mpAnimComboBox;
    QTimer mRefreshTimer;

    TResPtr<CAnimSet> mpSet;
    u32 mCurrentChar;
    u32 mCurrentAnim;
    bool mShowSkeleton;

    // Playback Controls
    double mLastAnimUpdate;
    float mAnimTime;
    bool mPlayAnim;
    bool mLoopAnim;
    float mPlaybackSpeed;

public:
    explicit CCharacterEditor(QWidget *parent = 0);
    ~CCharacterEditor();
    void UpdateAnimTime();
    CAnimation* CurrentAnimation() const;

public slots:
    void Open();
    void ToggleSkeletonVisible(bool Visible);
    void RefreshViewport();
    void HoverBoneChanged(u32 BoneID);
    void SetActiveCharacterIndex(int CharIndex);
    void SetActiveAnimation(int AnimIndex);
    void SetAnimTime(int Time);
    void SetAnimTime(float Time);

    void TogglePlay();
    void ToggleLoop(bool Loop);
    void AnimSpeedSpinBoxChanged(double NewVal);
};

#endif // CCHARACTEREDITORWINDOW_H
