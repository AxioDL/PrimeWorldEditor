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

public slots:
    void ActiveProjectChanged(CGameProject *pProj);
    void SetupPackagesList();
    void CookPackage();
    void CookAllDirtyPackages();
};

#endif // CPROJECTOVERVIEWDIALOG_H
