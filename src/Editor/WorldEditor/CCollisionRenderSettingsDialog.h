#ifndef CCOLLISIONRENDERSETTINGSDIALOG_H
#define CCOLLISIONRENDERSETTINGSDIALOG_H

#include <QDialog>

class CWorldEditor;

namespace Ui {
class CCollisionRenderSettingsDialog;
}

class CCollisionRenderSettingsDialog : public QDialog
{
    Q_OBJECT
    Ui::CCollisionRenderSettingsDialog *mpUi;

    CWorldEditor *mpEditor;

public:
    explicit CCollisionRenderSettingsDialog(CWorldEditor *pEditor, QWidget *pParent = 0);
    ~CCollisionRenderSettingsDialog();

public slots:
    void OnHideMaskChanged(QString NewMask);
    void OnHighlightMaskChanged(QString NewMask);
    void OnWireframeToggled(bool Enable);
};

#endif // CCOLLISIONRENDERSETTINGSDIALOG_H
