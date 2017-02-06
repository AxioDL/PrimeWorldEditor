#ifndef CEXPORTGAMEDIALOG_H
#define CEXPORTGAMEDIALOG_H

#include <Common/EGame.h>
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

    TString mGameTitle;
    TString mGameID;
    EGame mGame;
    ERegion mRegion;
    float mBuildVer;
    bool mTrilogy;

    bool mExportSuccess;
    QString mNewProjectPath;

public:
    explicit CExportGameDialog(const QString& rkIsoPath, const QString& rkExportDir, QWidget *pParent = 0);
    ~CExportGameDialog();

    void InitUI(QString ExportDir);
    bool ValidateGame();
    bool RequestTrilogyGame();
    float FindBuildVersion();

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
