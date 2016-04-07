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

public:
    explicit CCharacterEditor(QWidget *parent = 0);
    ~CCharacterEditor();

public slots:
    void Open();
    void RefreshViewport();
    void SetActiveCharacterIndex(int CharIndex);
    void SetActiveAnimation(int AnimIndex);
};

#endif // CCHARACTEREDITORWINDOW_H
