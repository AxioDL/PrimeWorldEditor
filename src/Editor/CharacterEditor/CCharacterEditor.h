#ifndef CCHARACTEREDITOR_H
#define CCHARACTEREDITOR_H

#include "CCharacterEditorViewport.h"
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

    QComboBox *mpCharComboBox;
    QComboBox *mpAnimComboBox;
    QTimer mRefreshTimer;

    TResPtr<CAnimSet> mpSet;
    u32 mCurrentChar;
    u32 mCurrentAnim;

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
    void RefreshViewport();
    void HoverBoneChanged(u32 BoneID);
    void SetActiveCharacterIndex(int CharIndex);
    void SetActiveAnimation(int AnimIndex);
    void SetAnimTime(int Time);
    void SetAnimTime(float Time);

    void PlayPauseButtonPressed();
    void LoopButtonToggled(bool Checked);
    void AnimSpeedSpinBoxChanged(double NewVal);
};

#endif // CCHARACTEREDITORWINDOW_H
