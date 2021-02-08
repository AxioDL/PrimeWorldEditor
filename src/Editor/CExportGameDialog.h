#ifndef CEXPORTGAMEDIALOG_H
#define CEXPORTGAMEDIALOG_H

#include <Common/EGame.h>
#include <Core/GameProject/CGameExporter.h>
#include <Core/GameProject/CGameProject.h>
#include <QDialog>
#include <QString>
#include <nod/DiscBase.hpp>
#include <memory>

namespace Ui {
class CExportGameDialog;
}

class CExportGameDialog : public QDialog
{
    Q_OBJECT

    std::unique_ptr<Ui::CExportGameDialog> mpUI;
    std::unique_ptr<nod::DiscBase> mpDisc;
    std::unique_ptr<CGameExporter> mpExporter;

    TString mGameTitle;
    TString mGameID;

    // Build Info
    EDiscType mDiscType{EDiscType::Normal};
    EGame mGame{EGame::Invalid};
    ERegion mRegion{ERegion::Unknown};
    float mBuildVer = 0.0f;
    bool mWiiFrontend = false;

    bool mExportSuccess = false;
    QString mNewProjectPath;

public:
    explicit CExportGameDialog(const QString& rkIsoPath, const QString& rkExportDir, QWidget *pParent = nullptr);
    ~CExportGameDialog() override;

    void InitUI(QString ExportDir);
    bool ValidateGame();
    bool RequestWiiPortGame();
    float FindBuildVersion();

    // Disc Tree
    void RecursiveAddToTree(const nod::Node *pkNode, class QTreeWidgetItem *pParent);

    // Accessors
    bool HasValidDisc() const    { return mpDisc != nullptr; }
    bool ExportSucceeded() const { return mExportSuccess; }
    QString ProjectPath() const  { return mNewProjectPath; }

public slots:
    void BrowseOutputDirectory();
    void BrowseAssetNameMap();
    void BrowseGameEditorInfo();
    void Export();
};

#endif // CEXPORTGAMEDIALOG_H
