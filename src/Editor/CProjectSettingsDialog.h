#ifndef CPROJECTSETTINGSDIALOG_H
#define CPROJECTSETTINGSDIALOG_H

#include "Editor/WorldEditor/CWorldEditor.h"
#include <Core/GameProject/CGameProject.h>
#include <Core/Resource/CWorld.h>
#include <QDialog>

#include <memory>

namespace Ui {
class CProjectSettingsDialog;
}

class CProjectSettingsDialog : public QDialog
{
    Q_OBJECT
    std::unique_ptr<Ui::CProjectSettingsDialog> mpUI;
    CGameProject *mpProject = nullptr;

    QVector<CResourceEntry*> mWorldEntries;
    QVector<CResourceEntry*> mAreaEntries;
    TResPtr<CWorld> mpWorld;

public:
    explicit CProjectSettingsDialog(QWidget *pParent = nullptr);
    ~CProjectSettingsDialog() override;

public slots:
    void ActiveProjectChanged(CGameProject *pProj);
    void GameNameChanged();
    void SetupPackagesList();
    void CookPackage();
    void CookAllDirtyPackages();
    void BuildISO();
};

#endif // CPROJECTSETTINGSDIALOG_H
