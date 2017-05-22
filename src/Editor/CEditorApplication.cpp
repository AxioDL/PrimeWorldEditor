#include "CEditorApplication.h"
#include "IEditor.h"
#include "CBasicViewport.h"
#include "CProgressDialog.h"
#include "CProjectSettingsDialog.h"
#include "Editor/CharacterEditor/CCharacterEditor.h"
#include "Editor/ModelEditor/CModelEditorWindow.h"
#include "Editor/ResourceBrowser/CResourceBrowser.h"
#include "Editor/WorldEditor/CWorldEditor.h"
#include <Common/AssertMacro.h>
#include <Common/CTimer.h>
#include <Core/GameProject/CGameProject.h>

#include <QFuture>
#include <QtConcurrent/QtConcurrentRun>

CEditorApplication::CEditorApplication(int& rArgc, char **ppArgv)
    : QApplication(rArgc, ppArgv)
    , mpActiveProject(nullptr)
    , mpWorldEditor(nullptr)
    , mpResourceBrowser(nullptr)
    , mpProjectDialog(nullptr)
{
    mLastUpdate = CTimer::GlobalTime();

    connect(&mRefreshTimer, SIGNAL(timeout()), this, SLOT(TickEditors()));
    mRefreshTimer.start(8);
}

CEditorApplication::~CEditorApplication()
{
    delete mpWorldEditor;
    delete mpProjectDialog;
}

void CEditorApplication::InitEditor()
{
    mpWorldEditor = new CWorldEditor();
    mpResourceBrowser = new CResourceBrowser(mpWorldEditor);
    mpProjectDialog = new CProjectSettingsDialog(mpWorldEditor);
    mpWorldEditor->showMaximized();
}

bool CEditorApplication::CloseProject()
{
    if (mpActiveProject)
    {
        // Close active editor windows. todo: check for unsaved changes
        foreach (IEditor *pEditor, mEditorWindows)
        {
            if (pEditor != mpWorldEditor && !pEditor->close())
                return false;
        }

        // Close world
        if (!mpWorldEditor->CloseWorld())
            return false;

        mpResourceBrowser->close();
        mpProjectDialog->close();

        // Emit before actually deleting the project to allow editor references to clean up
        CGameProject *pOldProj = mpActiveProject;
        mpActiveProject = nullptr;
        emit ActiveProjectChanged(nullptr);
        delete pOldProj;
    }

    return true;
}

bool CEditorApplication::OpenProject(const QString& rkProjPath)
{
    // Close existing project
    if (!CloseProject())
        return false;

    // Load new project
    TString Path = TO_TSTRING(rkProjPath);
    mpActiveProject = CGameProject::LoadProject(Path);

    if (mpActiveProject)
    {
        gpResourceStore = mpActiveProject->ResourceStore();
        emit ActiveProjectChanged(mpActiveProject);
        return true;
    }
    else
    {
        UICommon::ErrorMsg(mpWorldEditor, "Failed to open project! Is it already open in another Prime World Editor instance?");
        return false;
    }
}

void CEditorApplication::EditResource(CResourceEntry *pEntry)
{
    ASSERT(pEntry != nullptr);

    // Check if we're already editing this resource
    if (mEditingMap.contains(pEntry))
    {
        IEditor *pEd = mEditingMap[pEntry];
        pEd->show();
        pEd->raise();
    }

    else
    {
        // Attempt to load asset
        CResource *pRes = pEntry->Load();

        if (!pRes)
        {
            UICommon::ErrorMsg(mpWorldEditor, "Failed to load resource!");
            return;
        }

        // Launch editor window
        IEditor *pEd = nullptr;

        switch (pEntry->ResourceType())
        {
        case eModel:
            pEd = new CModelEditorWindow((CModel*) pRes, mpWorldEditor);
            break;

        case eAnimSet:
            pEd = new CCharacterEditor((CAnimSet*) pRes, mpWorldEditor);
            break;
        }

        if (pEd)
        {
            pEd->show();
            mEditingMap[pEntry] = pEd;
        }
        else
            UICommon::InfoMsg(mpWorldEditor, "Unsupported Resource", "This resource type is currently unsupported for editing.");
    }
}

void CEditorApplication::NotifyAssetsModified()
{
    emit AssetsModified();
}

bool CEditorApplication::CookPackage(CPackage *pPkg)
{
    return CookPackageList(QList<CPackage*>() << pPkg);
}

bool CEditorApplication::CookAllDirtyPackages()
{
    ASSERT(mpActiveProject != nullptr);
    QList<CPackage*> PackageList;

    for (u32 iPkg = 0; iPkg < mpActiveProject->NumPackages(); iPkg++)
    {
        CPackage *pPackage = mpActiveProject->PackageByIndex(iPkg);

        if (pPackage->NeedsRecook())
            PackageList << pPackage;
    }

    return CookPackageList(PackageList);
}

bool CEditorApplication::CookPackageList(QList<CPackage*> PackageList)
{
    if (!PackageList.isEmpty())
    {
        CProgressDialog Dialog("Cooking package" + QString(PackageList.size() > 1  ? "s" : ""), true, mpWorldEditor);

        QFuture<void> Future = QtConcurrent::run([&]()
        {
            Dialog.SetNumTasks(PackageList.size());

            for (int PkgIdx = 0; PkgIdx < PackageList.size() && !Dialog.ShouldCancel(); PkgIdx++)
            {
                CPackage *pPkg = PackageList[PkgIdx];
                Dialog.SetTask(PkgIdx, "Cooking " + pPkg->Name() + ".pak...");
                pPkg->Cook(&Dialog);
            }
        });

        Dialog.WaitForResults(Future);

        emit PackagesCooked();
        return !Dialog.ShouldCancel();
    }
    else return true;
}

// ************ SLOTS ************
void CEditorApplication::AddEditor(IEditor *pEditor)
{
    mEditorWindows << pEditor;
    connect(pEditor, SIGNAL(Closed()), this, SLOT(OnEditorClose()));
}

void CEditorApplication::TickEditors()
{
    double LastUpdate = mLastUpdate;
    mLastUpdate = CTimer::GlobalTime();
    double DeltaTime = mLastUpdate - LastUpdate;

    // The resource store should NOT be dirty at the beginning of a tick - this indicates we forgot to save it after updating somewhere
    if (gpResourceStore && gpResourceStore->IsDirty())
    {
        Log::Error("Resource store is dirty at the beginning of a tick! Call ConditionalSaveStore() after making any significant changes to assets!");
        DEBUG_BREAK;
        gpResourceStore->ConditionalSaveStore();
    }

    // Tick each editor window and redraw their viewports
    foreach(IEditor *pEditor, mEditorWindows)
    {
        if (pEditor->isVisible())
        {
            CBasicViewport *pViewport = pEditor->Viewport();
            bool ViewportVisible = (pViewport && pViewport->isVisible() && !pEditor->isMinimized());

            if (ViewportVisible) pViewport->ProcessInput();
            pEditor->EditorTick((float) DeltaTime);
            if (ViewportVisible) pViewport->Render();
        }
    }
}

void CEditorApplication::OnEditorClose()
{
    IEditor *pEditor = qobject_cast<IEditor*>(sender());
    ASSERT(pEditor);

    if (pEditor == mpWorldEditor)
    {
        mpWorldEditor = nullptr;
        quit();
    }
    else
    {
        for (auto Iter = mEditingMap.begin(); Iter != mEditingMap.end(); Iter++)
        {
            if (Iter.value() == pEditor)
            {
                mEditingMap.erase(Iter);
                break;
            }
        }

        mEditorWindows.removeOne(pEditor);
        delete pEditor;

        mpActiveProject->ResourceStore()->DestroyUnreferencedResources();
    }
}
