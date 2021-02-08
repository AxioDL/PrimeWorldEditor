#ifndef CEDITORAPPLICATION_H
#define CEDITORAPPLICATION_H

#include <Core/GameProject/CGameProject.h>
#include <QApplication>
#include <QTimer>
#include <QVector>
#include <memory>

class CBasicViewport;
class CProjectSettingsDialog;
class CResourceBrowser;
class CResourceEntry;
class CWorldEditor;
class IEditor;

constexpr int gkTickFrequencyMS = 8;

class CEditorApplication : public QApplication
{
    Q_OBJECT

    std::unique_ptr<CGameProject> mpActiveProject;
    CWorldEditor *mpWorldEditor = nullptr;
    CResourceBrowser *mpResourceBrowser = nullptr;
    CProjectSettingsDialog *mpProjectDialog = nullptr;
    QVector<IEditor*> mEditorWindows;
    QMap<CResourceEntry*,IEditor*> mEditingMap;
    bool mInitialized = false;

    QTimer mRefreshTimer;
    double mLastUpdate;

public:
    CEditorApplication(int& rArgc, char **ppArgv);
    ~CEditorApplication() override;

    void InitEditor();
    bool CloseAllEditors();
    bool CloseProject();
    bool OpenProject(const QString& rkProjPath);
    void EditResource(CResourceEntry *pEntry);
    void NotifyAssetsModified();

    bool CookPackage(CPackage *pPackage);
    bool CookAllDirtyPackages();
    bool CookPackageList(QList<CPackage*> PackageList);
    bool HasAnyDirtyPackages();

    bool RebuildResourceDatabase();

    CResourceBrowser* ResourceBrowser() const { return mpResourceBrowser; }

    // Accessors
    CGameProject* ActiveProject() const              { return mpActiveProject.get(); }
    CWorldEditor* WorldEditor() const                { return mpWorldEditor; }
    CProjectSettingsDialog* ProjectDialog() const    { return mpProjectDialog; }
    EGame CurrentGame() const                        { return mpActiveProject ? mpActiveProject->Game() : EGame::Invalid; }

    void SetEditorTicksEnabled(bool Enabled)         { Enabled ? mRefreshTimer.start(gkTickFrequencyMS) : mRefreshTimer.stop(); }
    bool AreEditorTicksEnabled() const               { return mRefreshTimer.isActive(); }

public slots:
    void AddEditor(IEditor *pEditor);
    void TickEditors();
    void OnEditorClose();

signals:
    void ActiveProjectChanged(CGameProject *pNewProj);
    void AssetsModified();
    void PackagesCooked();
};

#define gpEdApp static_cast<CEditorApplication*>(qApp)

#endif // CEDITORAPPLICATION_H
