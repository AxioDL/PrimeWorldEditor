#ifndef CEXPORTGAMEDIALOG_H
#define CEXPORTGAMEDIALOG_H

#include <Common/EGame.h>
#include <Core/GameProject/CGameExporter.h>
#include <Core/GameProject/CGameProject.h>
#include <QDialog>
#include <QString>
#include <nod/DiscBase.hpp>

namespace Ui {
class CExportGameDialog;
}

class CExportGameDialog : public QDialog
{
    Q_OBJECT

    Ui::CExportGameDialog *mpUI;
    nod::DiscBase *mpDisc;
    CGameExporter *mpExporter;

    TString mGameTitle;
    TString mGameID;

    // Build Info
    EDiscType mDiscType;
    EGame mGame;
    ERegion mRegion;
    float mBuildVer;
    bool mWiiFrontend;

    bool mExportSuccess;
    QString mNewProjectPath;

public:
    explicit CExportGameDialog(const QString& rkIsoPath, const QString& rkExportDir, QWidget *pParent = 0);
    ~CExportGameDialog();

    void InitUI(QString ExportDir);
    bool ValidateGame();
    bool RequestWiiPortGame();
    float FindBuildVersion();

    // Disc Tree
    void RecursiveAddToTree(const nod::Node *pkNode, class QTreeWidgetItem *pParent);

    // Accessors
    inline bool HasValidDisc() const    { return mpDisc != nullptr; }
    inline bool ExportSucceeded() const { return mExportSuccess; }
    inline QString ProjectPath() const  { return mNewProjectPath; }

public slots:
    void BrowseOutputDirectory();
    void BrowseAssetNameMap();
    void BrowseGameEditorInfo();
    void Export();
};

#endif // CEXPORTGAMEDIALOG_H
