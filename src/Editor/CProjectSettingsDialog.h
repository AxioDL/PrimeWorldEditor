#ifndef CPROJECTSETTINGSDIALOG_H
#define CPROJECTSETTINGSDIALOG_H

#include "Editor/WorldEditor/CWorldEditor.h"
#include <Core/GameProject/CGameProject.h>
#include <Core/Resource/CWorld.h>
#include <QDialog>

namespace Ui {
class CProjectSettingsDialog;
}

class CProjectSettingsDialog : public QDialog
{
    Q_OBJECT
    Ui::CProjectSettingsDialog *mpUI;
    CGameProject *mpProject;

    QVector<CResourceEntry*> mWorldEntries;
    QVector<CResourceEntry*> mAreaEntries;
    TResPtr<CWorld> mpWorld;

public:
    explicit CProjectSettingsDialog(QWidget *pParent = 0);
    ~CProjectSettingsDialog();

public slots:
    void ActiveProjectChanged(CGameProject *pProj);
    void GameNameChanged();
    void SetupPackagesList();
    void CookPackage();
    void CookAllDirtyPackages();
    void BuildISO();
};

#endif // CPROJECTSETTINGSDIALOG_H
