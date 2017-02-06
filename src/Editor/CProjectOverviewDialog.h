#ifndef CPROJECTOVERVIEWDIALOG_H
#define CPROJECTOVERVIEWDIALOG_H

#include "Editor/WorldEditor/CWorldEditor.h"
#include <Core/GameProject/CGameProject.h>
#include <Core/Resource/CWorld.h>
#include <QDialog>

namespace Ui {
class CProjectOverviewDialog;
}

class CProjectOverviewDialog : public QDialog
{
    Q_OBJECT
    Ui::CProjectOverviewDialog *mpUI;
    CGameProject *mpProject;

    QVector<CResourceEntry*> mWorldEntries;
    QVector<CResourceEntry*> mAreaEntries;
    TResPtr<CWorld> mpWorld;

public:
    explicit CProjectOverviewDialog(QWidget *pParent = 0);
    ~CProjectOverviewDialog();

protected:
    void InternalLoadProject(const QString& rkPath);

public slots:
    void OpenProject();
    void ExportGame();
    void LoadWorld();
    void LaunchEditor();
    void LaunchResourceBrowser();
    void CookPackage();

    void SetupWorldsList();
    void SetupPackagesList();

signals:
    void ActiveProjectChanged(CGameProject *pNewProj);
};

#endif // CPROJECTOVERVIEWDIALOG_H
